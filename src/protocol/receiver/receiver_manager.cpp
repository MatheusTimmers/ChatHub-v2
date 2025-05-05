#include "../../../include/protocol/receiver/receiver_manager.hpp"

#include "../../../include/parser/message_parser.hpp"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <string>

ReceiverManager::ReceiverManager(Socket& sock, DeviceManager& dm, SenderManager& sender)
    : socket_(sock), device_manager_(dm), sender_(sender), running_(false) {}

ReceiverManager::~ReceiverManager() {
    this->stop();
}

void ReceiverManager::start() {
    this->running_             = true;
    this->recvThread_          = std::thread(&ReceiverManager::receiveLoop, this);
    this->recvBroadcastThread_ = std::thread(&ReceiverManager::receiveBroadcastLoop, this);
}

void ReceiverManager::stop() {
    this->running_ = false;

    if (this->recvThread_.joinable())
        this->recvThread_.join();

    if (this->recvBroadcastThread_.joinable())
        this->recvBroadcastThread_.join();
}

void ReceiverManager::setMessageHandler(
    std::function<void(const std::string& from, const std::string& text)> h) {
    this->messageHandler_ = std::move(h);
}

void ReceiverManager::receiveLoop() {
    while (this->running_) {
        std::string buf;
        sockaddr_in from{};

        int n = this->socket_.recvMessage(buf, &from);
        if (n <= 0)
            continue;
        auto    type = identifyType(buf);
        Message msg  = buildMessage(type, buf, from);
        this->handle(msg);
    }
}

void ReceiverManager::receiveBroadcastLoop() {
    while (this->running_) {
        std::string buf;
        sockaddr_in from{};

        int n = this->socket_.recvBroadcast(buf, &from);
        if (n <= 0)
            continue;

        sockaddr_in addr = this->socket_.getAddres();
        if (from.sin_addr.s_addr == addr.sin_addr.s_addr && from.sin_port == addr.sin_port) {
            continue;
        }

        Message msg = buildMessage(MessageType::HEARTBEAT, buf, from);
        this->onHeartbeat(msg);
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
        this->onAck(msg);
        break;
    case MessageType::NACK:
        this->onNack(msg);
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

    this->device_manager_.addOrUpdate(hb.name, msg.from);
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

    std::string error;
    std::string name = this->device_manager_.getNameByAddr(msg.from);
    if (!this->file_receiver_.finishReceive(name, em.hash, error)) {
        this->sender_.sendNack(em.id, error, msg.from);
        return;
    }

    if (this->messageHandler_)
        this->messageHandler_(name, "Enviou um arquivo para você");

    this->sender_.sendAck(em.id, msg.from);
}

void ReceiverManager::onAck(const Message& msg) {
    AckMsg ack{};
    if (!MessageParser::parseAck(msg.payload, ack)) {
        return;
    }

    this->sender_.handleAck(ack.id);
}

void ReceiverManager::onNack(const Message& msg) {
    NackMsg nack{};
    if (!MessageParser::parseNack(msg.payload, nack)) {
        return;
    }

    std::string name = this->device_manager_.getNameByAddr(msg.from);
    if (this->messageHandler_)
        this->messageHandler_(name, nack.reason);

    this->sender_.handleNack(nack.id);
}
