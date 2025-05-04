#include "../include/user_net.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>

UserNet::UserNet(const std::string& name, const std::string& ip, uint16_t port)
    : name_(std::move(name)), ip_(std::move(ip)), port_(port), socket_(), device_manager_(name),
      sender_(socket_), receiver_(socket_, device_manager_, sender_), ui_() {
    this->receiver_.setMessageHandler([this](const std::string& from, const std::string& text) {
        this->ui_.displayMessage(from, text);
    });
}

UserNet::~UserNet() {
    this->stop();
}

void UserNet::start() {
    this->running_ = true;
    this->socket_.openSocket(this->ip_, this->port_);

    this->sender_.start();
    this->receiver_.start();

    this->sender_.startHeartbeat(this->name_, HEARTBEAT_INTERVAL);
    this->device_manager_.startCleanup(TIMEOUT_DEVICE, TIME_INACTIVE_DEVICE_LOOP);
    this->userInterfaceLoop();
    this->stop();
}

void UserNet::stop() {
    this->running_ = false;
    this->socket_.closeSocket();

    this->sender_.stopHeartbeat();
    this->device_manager_.stopCleanup();

    this->receiver_.stop();
    this->sender_.stop();
}

void UserNet::userInterfaceLoop() {
    while (this->running_) {
        Command cmd = this->ui_.readCommand();
        switch (cmd.type) {
        case CommandType::LIST_USERS:
            this->ui_.displayDevices(this->device_manager_.listDevices());
            break;
        case CommandType::CHAT: {
            this->sender_.sendTalk(cmd.text,
                                   this->device_manager_.getDeviceInfoByName(cmd.target).addr);
            break;
        }
        case CommandType::SEND_FILE:
            this->sender_.sendFile(cmd.filePath,
                                   this->device_manager_.getDeviceInfoByName(cmd.target).addr);
            break;
        case CommandType::EXIT:
            this->running_ = false;
            break;
        default:
            break;
        }
    }
}
