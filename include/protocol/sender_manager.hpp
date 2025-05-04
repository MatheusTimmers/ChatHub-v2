#pragma once

#include "../transport/socket.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include <thread>
#include <unordered_map>

#define CHUNK_SIZE 1024
#define TIMEOUT 2
#define MAX_RETRY 5

class SenderManager {
  public:
    SenderManager(Socket& sock);
    ~SenderManager();

    void start();
    void stop();

    void startHeartbeat(const std::string& name, int intervalSec);
    void stopHeartbeat();

    void sendHeartbeat(const std::string& myName);
    void sendTalk(const std::string& text, const sockaddr_in& to);
    void sendFile(const std::string& filePath, const sockaddr_in& to);
    void sendAck(uint32_t id, const sockaddr_in& to);
    void sendNack(uint32_t id, const std::string& error, const sockaddr_in& to);

    void handleAck(uint32_t id);
    void handleNack(uint32_t id);

  private:
    struct PendingFile {
        std::string filePath;
        sockaddr_in to;
    };

    struct Pending {
        std::string buf;
        int         retries;
        sockaddr_in to;
        std::time_t lastSent;
    };

    Socket& socket_;

    std::atomic<bool> running_{false};
    std::thread       retryThread_;

    std::atomic<bool> heartbeatRunning_{false};
    std::thread       heartbeatThread_;
    std::mutex        pendingMtx_;

    std::uint32_t nextId_{1};

    std::unordered_map<uint32_t, Pending>     pendingMap_;
    std::unordered_map<uint32_t, PendingFile> pendingFile_;

    uint32_t nextId();

    void retryLoop();
    void sendTo(const std::string& buf, const sockaddr_in& to);
    void sendBroadcast(const std::string& buf);
    void addPending(uint32_t id, const std::string& buf, const sockaddr_in& to);

    void processFileSend(PendingFile pf);
};
