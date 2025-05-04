#include "../../../include/protocol/receiver/receiver_manager.hpp"

#include "../../../include/parser/message_parser.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

ReceiverManager::ReceiverManager(Socket& sock, DeviceManager& dm, SenderManager& sender)
    : socket_(sock), device_manager_(dm), sender_(sender), running_(false) {}

ReceiverManager::~ReceiverManager() {
    stop();
}

void ReceiverManager::start() {
    running_    = true;
    recvThread_ = std::thread(&ReceiverManager::receiveLoop, this);
}

void ReceiverManager::stop() {
    running_ = false;

    if (recvThread_.joinable())
        recvThread_.join();
}

void ReceiverManager::setMessageHandler(
    std::function<void(const std::string& from, const std::string& text)> h) {
    this->messageHandler_ = std::move(h);
}

void ReceiverManager::receiveLoop() {
    while (running_) {
        std::string buf;
        sockaddr_in from{};
        int         n = socket_.recvMessage(buf, &from);
        if (n <= 0)
            continue;
        auto    type = identifyType(buf);
        Message msg  = buildMessage(type, buf, from);
        handle(msg);
    }
}

void ReceiverManager::handle(const Message& msg) {
    switch (msg.type) {
    case MessageType::HEARTBEAT:
        this->onHeartbeat(msg);
        break;
    case MessageType::TALK:
        this->onTalk(msg);
        break;
    case MessageType::FILE:
        this->onFile(msg);
        break;
    case MessageType::CHUNK:
        this->onChunk(msg);
        break;
    case MessageType::END:
        this->onEnd(msg);
        break;
    case MessageType::ACK:
        this->sender_.handleAck(msg.id);
        break;
    case MessageType::NACK:
        this->sender_.handleNack(msg.id);
        break;
    default:
        break;
    }
}

void ReceiverManager::onHeartbeat(const Message& msg) {
    HeartbeatMsg hb{};
    if (!MessageParser::parseHeartbeat(msg.payload, hb)) {
        return;
    }

    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &msg.from.sin_addr, ipstr, sizeof(ipstr));
    uint16_t port = ntohs(msg.from.sin_port);

    this->device_manager_.addOrUpdate(hb.name, ipstr, port);
    this->sender_.sendAck(msg.id, msg.from);
}

void ReceiverManager::onTalk(const Message& msg) {
    TalkMsg tk{};
    if (!MessageParser::parseTalk(msg.payload, tk)) {
        return;
    }

    std::string name = this->device_manager_.getNameByAddr(msg.from);
    if (this->messageHandler_)
        this->messageHandler_(name, tk.data);

    this->sender_.sendAck(tk.id, msg.from);
}

void ReceiverManager::onFile(const Message& msg) {
    FileMsg fm{};
    if (!MessageParser::parseFile(msg.payload, fm)) {
        return;
    }

    std::string name = this->device_manager_.getNameByAddr(msg.from);
    this->file_receiver_.startReceive(name, fm.filename);
    this->sender_.sendAck(fm.id, msg.from);
}

void ReceiverManager::onChunk(const Message& msg) {
    ChunkMsg cm{};
    if (!MessageParser::parseChunk(msg.payload, cm)) {
        return;
    }

    std::string name = this->device_manager_.getNameByAddr(msg.from);
    this->file_receiver_.writeChunk(name, cm.seq, cm.data);
    this->sender_.sendAck(cm.id, msg.from);
}

void ReceiverManager::onEnd(const Message& msg) {
    EndMsg em{};
    if (!MessageParser::parseEnd(msg.payload, em)) {
        return;
    }

    std::string name = this->device_manager_.getNameByAddr(msg.from);
    this->file_receiver_.finishReceive(name);
    this->sender_.sendAck(em.id, msg.from);
}
