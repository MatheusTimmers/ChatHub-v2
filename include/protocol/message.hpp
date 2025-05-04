#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <string>

enum class MessageType { HEARTBEAT, TALK, FILE, CHUNK, END, ACK, NACK, UNKNOWN };

struct Message {
    MessageType type;
    uint32_t    id; 
    std::string payload;
    sockaddr_in from{}; // TODO: ALterar o nome from
};

inline MessageType identifyType(const std::string& buf) {
    if (buf.find("HEARTBEAT", 0) == 0)
        return MessageType::HEARTBEAT;

    if (buf.find("TALK", 0) == 0)
        return MessageType::TALK;

    if (buf.find("FILE", 0) == 0)
        return MessageType::FILE;

    if (buf.find("CHUNK", 0) == 0)
        return MessageType::CHUNK;

    if (buf.find("END", 0) == 0)
        return MessageType::END;

    if (buf.find("ACK", 0) == 0)
        return MessageType::ACK;

    if (buf.find("NACK", 0) == 0)
        return MessageType::NACK;

    return MessageType::UNKNOWN;
}

inline Message buildMessage(MessageType type, const std::string& buf, sockaddr_in from) {
    Message msg{};
    msg.type        = type;
    msg.from        = from;
    msg.payload     = buf;
    return msg;
}
