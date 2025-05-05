#include "../../include/protocol/sender_manager.hpp"

#include "../../include/utils/file_utils.hpp"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <vector>

SenderManager::SenderManager(Socket& sock) : socket_(sock), running_(false) {}

SenderManager::~SenderManager() {
    this->stop();
}

void SenderManager::start() {
    this->running_     = true;
    this->retryThread_ = std::thread(&SenderManager::retryLoop, this);
}

void SenderManager::stop() {
    this->running_ = false;
    if (this->retryThread_.joinable())
        this->retryThread_.join();
}

void SenderManager::sendHeartbeat(const std::string& myName) {
    std::string buf = "HEARTBEAT " + myName;

    this->sendBroadcast(buf);
}

void SenderManager::sendTalk(const std::string& text, const sockaddr_in& to) {
    uint32_t    id  = nextId();
    std::string buf = "TALK " + std::to_string(id) + " " + text;

    this->sendTo(buf, to);
    this->addPending(id, buf, to);
}

void SenderManager::sendAck(uint32_t id, const sockaddr_in& to) {
    std::string buf = "ACK " + std::to_string(id);
    this->sendTo(buf, to);
}

void SenderManager::sendNack(uint32_t id, const std::string& error, const sockaddr_in& to) {
    std::string buf = "NACK " + std::to_string(id);
    if (!error.empty()) {
        buf += " " + error;
    } else {
        buf += " Don’t Panic! 🛸";
    }

    this->sendTo(buf, to);
}

void SenderManager::sendFile(const std::string& filePath, const sockaddr_in& to) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        perror("Error ao abrir arquivo");
        return;
    }

    file.seekg(0, std::ios::end);
    uint64_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (file_size <= 0) {
        perror("Arquivo vazio ou erro ao obter tamanho");
        file.close();
        return;
    }

    uint32_t    id = nextId();
    std::string file_name;

    auto pos = filePath.find_last_of("/\\");
    if (pos != std::string::npos)
        file_name = filePath.substr(pos + 1);
    else
        file_name = filePath;

    std::string fileCmd =
        "FILE " + std::to_string(id) + " " + file_name.c_str() + " " + std::to_string(file_size);

    this->sendTo(fileCmd, to);
    this->addPending(id, fileCmd, to);

    this->pendingFile_[id] = PendingFile{filePath, to};

    file.close();
}

void SenderManager::processFileSend(PendingFile pf) {
    std::ifstream file(pf.filePath, std::ios::binary);
    if (!file) {
        perror("Error ao abrir arquivo");
        return;
    }

    uint32_t id = nextId();

    std::vector<char> buf(CHUNK_SIZE);
    uint32_t          seq = 0;

    while (file) {
        file.read(buf.data(), CHUNK_SIZE);
        std::streamsize n = file.gcount();
        if (n <= 0)
            break;

        std::string data(buf.data(), n);
        std::string chunkCmd =
            "CHUNK " + std::to_string(id) + " " + std::to_string(seq) + " " + data;

        this->sendTo(chunkCmd, pf.to);
        this->addPending(id, chunkCmd, pf.to);

        seq++;
        id = nextId();
    }

    std::string fileHash = computeSHA256(pf.filePath);

    std::string endCmd = "END " + std::to_string(id) + " " + fileHash;
    this->sendTo(endCmd, pf.to);
    this->addPending(id, endCmd, pf.to);

    file.close();
}

void SenderManager::handleAck(uint32_t id) {
    {
        std::lock_guard<std::mutex> lk(this->pendingMtx_);
        this->pendingMap_.erase(id);
    }

    // TODO: Isso aqui deveria ter um mutex
    auto itFile = this->pendingFile_.find(id);
    if (itFile != this->pendingFile_.end()) {
        this->processFileSend(itFile->second);

        this->pendingFile_.erase(itFile);
        return;
    }
}

void SenderManager::handleNack(uint32_t id) {
    std::lock_guard<std::mutex> lk(this->pendingMtx_);
    auto                        it = this->pendingMap_.find(id);
    if (it != this->pendingMap_.end())
        this->sendTo(it->second.buf, it->second.to);
}

void SenderManager::retryLoop() {
    while (this->running_) {
        auto now = std::time(nullptr);
        {
            std::lock_guard<std::mutex> lk(this->pendingMtx_);
            for (auto it = this->pendingMap_.begin(); it != this->pendingMap_.end();) {
                auto& p = it->second;
                if (now - p.lastSent > TIMEOUT) {
                    if (p.retries < MAX_RETRY) {
                        sendTo(p.buf, p.to);
                        p.lastSent = now;
                        p.retries++;
                        ++it;
                    } else {
                        // excedeu retries, desiste
                        std::clog << "[SenderManager] WARNING: packet id=" << it->first
                                  << " data=" << it->second.buf << " reached max retries ("
                                  << MAX_RETRY << "), giving up\n";
                        it = pendingMap_.erase(it);
                    }
                } else {
                    ++it;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT));
    }
}

void SenderManager::sendTo(const std::string& buf, const sockaddr_in& to) {
    this->socket_.sendMessage(buf, &to);
}

void SenderManager::sendBroadcast(const std::string& buf) {
    this->socket_.sendBroadcast(buf);
}

void SenderManager::addPending(uint32_t id, const std::string& buf, const sockaddr_in& to) {
    std::lock_guard<std::mutex> lk(this->pendingMtx_);
    auto                        now = std::time(nullptr);
    this->pendingMap_[id]           = Pending{buf, 0, to, now};
}

void SenderManager::startHeartbeat(const std::string& name, int intervalSec) {
    if (this->heartbeatRunning_)
        return;

    this->heartbeatRunning_ = true;
    this->heartbeatThread_  = std::thread([this, name, intervalSec]() {
        while (this->heartbeatRunning_) {
            sendHeartbeat(name);
            std::this_thread::sleep_for(std::chrono::seconds(intervalSec));
        }
    });
}

void SenderManager::stopHeartbeat() {
    if (!this->heartbeatRunning_)
        return;

    this->heartbeatRunning_ = false;
    if (this->heartbeatThread_.joinable())
        this->heartbeatThread_.join();
}

uint32_t SenderManager::nextId() {
    return this->nextId_++;
}
