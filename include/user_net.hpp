#pragma once

#include "device/device_manager.hpp"
#include "protocol/receiver/receiver_manager.hpp"
#include "protocol/sender_manager.hpp"
#include "transport/socket.hpp"
#include "ui/user_interface.hpp"

#include <atomic>
#include <string>

#define TIME_INACTIVE_DEVICE_LOOP 5 // seconds
#define TIMEOUT_DEVICE 10           // seconds
#define HEARTBEAT_INTERVAL 5        // seconds

class UserNet {
  public:
    UserNet(std::string name, int port);
    ~UserNet();
    void start();
    void stop();

  private:
    std::atomic<bool> running_{false};

    std::string name_;
    int         port_;
    Socket      socket_;

    DeviceManager   device_manager_;
    SenderManager   sender_;
    ReceiverManager receiver_;
    UserInterface   ui_;

    void addDevice(const std::string& name, const std::string& ip, int port);
    void userInterfaceLoop();
};
