#pragma once

#include "../include/device_manager.hpp"

#include "socket.hpp"
#include <string>

#define TIME_INACTIVE_DEVICE_LOOP 5 //seconds

class UserNet {
public:
    UserNet(std::string name, int port);
    ~UserNet();
    void start();
    void stop();

    void addDevice(const std::string& name, const std::string& ip, int port);

private:
    bool running_;

    std::string name_;
    int port_;
    Socket socket_;
 
    DeviceManager device_manager_;

    void removeInactiveLoop();

    // temp
    void sendHeartbeat();
};

