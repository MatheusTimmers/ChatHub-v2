#include "../include/user_net.hpp"
#include <sstream>
#include <thread>

UserNet::UserNet(std::string name, int port) 
    : name_(std::move(name)), port_(port)
{}

UserNet::~UserNet() {
    this->stop();
}

void UserNet::stop() {
    this->running_ = false;
    this->socket_.closeSocket();
}

void UserNet::start() {
    this->running_ = true;
    this->socket_.openSocket(this->port_);

    // TODO: Passar para uma classe separada
    this->sendHeartbeat();

    //std::thread(&UserNet::HandleMessage, this).detach();
    // std::thread(&UserNet::UserInterfaceLoop, this).detach();

    // std::thread(&UserNet::HeartbeatLoop, this).detach();
    std::thread(&UserNet::removeInactiveLoop, this).detach();

    // std::thread(&UserNet::SendLoop, this).detach();
}

void UserNet::removeInactiveLoop() {
    while (this->running_) {
        std::this_thread::sleep_for(std::chrono::seconds(TIME_INACTIVE_DEVICE_LOOP));
        this->device_manager_.removeInactiveDevices();
    }
}

void UserNet::sendHeartbeat() {
    std::ostringstream oss;
    oss << "HEARTBEAT " << this->name_;
    this->socket_.sendBroadcast(oss.str().c_str());
}
