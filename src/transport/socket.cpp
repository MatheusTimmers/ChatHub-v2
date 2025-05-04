#include "../../include/transport/socket.hpp"

#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Socket::~Socket() {
    this->closeSocket();
}

void Socket::openSocket(std::string& ip, uint16_t port) {
    if ((this->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    if ((this->sockfd_bcast_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Erro ao criar socket do broadcast");
        exit(EXIT_FAILURE);
    }

    int on = 1;
    if ((setsockopt(this->sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) ||
        setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        exit(EXIT_FAILURE);
    }

    if (!this->initBroadcast()) {
        perror("Erro ao fazer init do broadcast.");
        exit(EXIT_FAILURE);
    }

    if (!this->bindSocket(ip, port)) {
        perror("Erro ao fazer bind no servidor.");
        exit(EXIT_FAILURE);
    }
}

void Socket::closeSocket() {
    if (this->sockfd >= 0) {
        close(this->sockfd);
        this->sockfd = -1;
    }

    if (this->sockfd_bcast_ >= 0) {
        close(this->sockfd_bcast_);
        this->sockfd_bcast_ = -1;
    }
}

bool Socket::bindSocket(std::string& ip, uint16_t port) {
    memset(&this->addr_, 0, sizeof(this->addr_));
    this->addr_.sin_family = AF_INET;
    this->addr_.sin_port   = htons(port);

    if (ip.empty()) {
        ip = this->getLocalIp();
    }

    if (inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) != 1) {
        return false;
    }

    int n = bind(this->sockfd, (struct sockaddr*)&this->addr_, sizeof(this->addr_));
    if (n < 0) {
        return false; // IP inválido
    }

    sockaddr_in baddr{};
    baddr.sin_family      = AF_INET;
    baddr.sin_port        = htons(BROADCAST_PORT);
    baddr.sin_addr.s_addr = INADDR_ANY;

    return bind(this->sockfd_bcast_, (struct sockaddr*)&baddr, sizeof(baddr)) == 0;
}

bool Socket::initBroadcast() {
    int broadcastEnable = 1;
    if ((setsockopt(this->sockfd_bcast_, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                    sizeof(broadcastEnable)) < 0) ||
        setsockopt(sockfd_bcast_, SOL_SOCKET, SO_REUSEADDR, &broadcastEnable,
                   sizeof(broadcastEnable)) < 0) {
        return false;
    }

    memset(&this->broadcastAddr_, 0, sizeof(this->broadcastAddr_));
    this->broadcastAddr_.sin_family      = AF_INET;
    this->broadcastAddr_.sin_port        = htons(BROADCAST_PORT);
    this->broadcastAddr_.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    return true;
}

// TODO: Usar a mesma funcao para os send DRY
int Socket::sendBroadcast(const std::string& msg) {
    int                    n    = -1;
    const struct sockaddr* dest = reinterpret_cast<const struct sockaddr*>(&this->broadcastAddr_);

    n = sendto(this->sockfd, &msg[0], msg.size(), 0, dest, sizeof(sockaddr_in));
    if (n < 0) {
        perror("Erro ao enviar mensagem broadcast");
        return -1;
    }

    return n;
}

int Socket::sendMessage(const std::string& msg, const sockaddr_in* addr) {
    int                    n    = -1;
    const struct sockaddr* dest = reinterpret_cast<const struct sockaddr*>(addr);

    n = sendto(this->sockfd, &msg[0], msg.size(), 0, dest, sizeof(sockaddr_in));
    if (n < 0) {
        perror("Erro ao enviar mensagem");
        return -1;
    }

    return n;
}

int Socket::recvMessage(std::string& msg, sockaddr_in* from) {
    socklen_t        addr_len = sizeof(sockaddr_in);
    struct sockaddr* dest     = reinterpret_cast<struct sockaddr*>(from);

    msg.resize(BUFFER_SIZE);
    int n = recvfrom(this->sockfd, &msg[0], BUFFER_SIZE, 0, dest, &addr_len);

    if (n > 0) {
        msg.resize(n);
    } else {
        msg.clear();
    }

    return n;
}

int Socket::recvBroadcast(std::string& msg, sockaddr_in* from) {
    socklen_t        addr_len = sizeof(sockaddr_in);
    struct sockaddr* dest     = reinterpret_cast<struct sockaddr*>(from);

    msg.resize(BUFFER_SIZE);
    int n = recvfrom(this->sockfd_bcast_, &msg[0], BUFFER_SIZE, 0, dest, &addr_len);

    if (n > 0) {
        msg.resize(n);
    } else {
        msg.clear();
    }

    return n;
}

int Socket::getSocketFd() {
    return this->sockfd;
};

std::string Socket::getLocalIp() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        return {};

    sockaddr_in remote{};
    remote.sin_family = AF_INET;
    remote.sin_port   = htons(53);
    inet_pton(AF_INET, "8.8.8.8", &remote.sin_addr);

    if (connect(sock, (sockaddr*)&remote, sizeof(remote)) < 0) {
        return "";
    };

    sockaddr_in local{};
    socklen_t   len = sizeof(local);
    getsockname(sock, (sockaddr*)&local, &len);

    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &local.sin_addr, buf, sizeof(buf));

    close(sock);
    return std::string(buf);
}
