#pragma once

#include <netinet/in.h>
#include <string>

// TODO: Deixar isso em  configuração mais bonita
#define BUFFER_SIZE 1024
#define BROADCAST_PORT 9000

class Socket {
    private:
        struct sockaddr_in addr_;
        struct sockaddr_in broadcastAddr_;
        int sockfd;

        bool bindSocket(int port);
        bool initBroadcast();

    public:
        ~Socket();

        void openSocket(int port);
        void closeSocket();

        int recvMessage(std::string& msg, sockaddr_in* from);
        int sendMessage(const std::string& msg, const sockaddr_in* addr);
        int sendBroadcast(const std::string&  msg);

        int getSocketFd();
};
