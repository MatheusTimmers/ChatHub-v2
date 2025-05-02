#include "../include/socket.hpp"

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Socket::~Socket() {
    this->closeSocket();
}

void Socket::openSocket(int port) {
    if ((this->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    if (this->initBroadcast()) {
        perror("Erro ao fazer init do broadcast.");
        exit(EXIT_FAILURE);
    }

    if (this->bindSocket(port)) {
        perror("Erro ao fazer bind no servidor.");
        exit(EXIT_FAILURE);
    }
}

void Socket::closeSocket() {
    if (this->sockfd >= 0) {
        close(this->sockfd);
        this->sockfd = -1;
    }
}

bool Socket::bindSocket(int port) {
    sockaddr_in addr;
    memset(&this->addr_, 0, sizeof(this->addr_));
    this->addr_.sin_family      = AF_INET;
    this->addr_.sin_port        = htons(port);
    this->addr_.sin_addr.s_addr = INADDR_ANY;

    return bind(this->sockfd, (struct sockaddr*)&this->addr_, sizeof(sockaddr_in)) == 0;
}

bool Socket::initBroadcast() {
    int broadcastEnable = 1;
    if (setsockopt(this->sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                   sizeof(broadcastEnable)) < 0) {
        return false;
    }

    memset(&this->broadcastAddr_, 0, sizeof(this->broadcastAddr_));
    this->broadcastAddr_.sin_family      = AF_INET;
    this->broadcastAddr_.sin_port        = htons(BROADCAST_PORT);
    this->broadcastAddr_.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    return true;
}

int Socket::sendBroadcast(const std::string& buffer) {
    return this->sendMessage(buffer, &this->broadcastAddr_);
}

int Socket::sendMessage(const std::string& buffer, const sockaddr_in* addr) {
    int                    n    = -1;
    const struct sockaddr* dest = reinterpret_cast<const struct sockaddr*>(addr);

    n = sendto(sockfd, buffer.data(), buffer.size(), 0, dest, sizeof(sockaddr_in));
    if (n < 0) {
        perror("Erro ao enviar mensagem");
        return -1;
    }

    return n;
}

int Socket::recvMessage(std::string& buffer) {
    socklen_t        addr_len = sizeof(this->addr_);
    struct sockaddr* dest     = reinterpret_cast<struct sockaddr*>(&this->addr_);

    buffer.resize(BUFFER_SIZE);
    int n = recvfrom(this->sockfd, buffer.data(), BUFFER_SIZE, 0, dest, &addr_len);
    buffer.resize(n);

    return 0;
}

int Socket::getSocketFd() {
    return this->sockfd;
};
