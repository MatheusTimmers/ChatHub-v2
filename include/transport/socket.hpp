#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <string>

// TODO: Deixar isso em  configuração mais bonita
#define BUFFER_SIZE 1024
#define BROADCAST_PORT 9000

class Socket {
  private:
    struct sockaddr_in addr_;
    struct sockaddr_in broadcastAddr_;
    int                sockfd;
    int                sockfd_bcast_;

    bool bindSocket(std::string& ip, uint16_t port);
    bool initBroadcast();

  public:
    ~Socket();

    void openSocket(std::string& ip, uint16_t port);
    void closeSocket();

    int recvBroadcast(std::string& out, sockaddr_in* from);
    int recvMessage(std::string& msg, sockaddr_in* from);
    int sendMessage(const std::string& msg, const sockaddr_in* addr);
    int sendBroadcast(const std::string& msg);

    int         getSocketFd();
    std::string getLocalIp();
    sockaddr_in getAddres();
};
