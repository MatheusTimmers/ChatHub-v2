#pragma once

#include "../../device/device_manager.hpp"
#include "../../transport/socket.hpp"
#include "../message.hpp"
#include "../sender_manager.hpp"
#include "file_receiver.hpp"

#include <atomic>
#include <functional>
#include <netinet/in.h>
#include <thread>

class ReceiverManager {
  public:
    ReceiverManager(Socket& sock, DeviceManager& dm, SenderManager& sender);
    ~ReceiverManager();

    void start();
    void stop();

    void setMessageHandler(std::function<void(const std::string& from, const std::string& text)> h);

  private:
    Socket&           socket_;
    DeviceManager&    device_manager_;
    SenderManager&    sender_;
    FileReceiver      file_receiver_;
    std::atomic<bool> running_;
    std::thread       recvThread_, recvBroadcastThread_;

    void receiveLoop();
    void receiveBroadcastLoop();
    void handle(const Message& msg);

    std::function<void(const std::string&, const std::string&)> messageHandler_;

    void onHeartbeat(const Message& msg);
    void onTalk(const Message& msg);
    void onFile(const Message& msg);
    void onChunk(const Message& msg);
    void onEnd(const Message& msg);
};
