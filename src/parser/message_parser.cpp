#include "../../include/parser/message_parser.hpp"

#include <netinet/in.h>
#include <sstream>

namespace MessageParser {

bool parseHeartbeat(const std::string& msg, HeartbeatMsg& out) {
    std::istringstream iss(msg);
    std::string        type;
    if (!(iss >> type) || type != "HEARTBEAT")
        return false;
    if (!(iss >> out.name))
        return false;
    return true;
}

bool parseTalk(const std::string& msg, TalkMsg& out) {
    std::istringstream iss(msg);
    std::string        type;

    if (!(iss >> type) || type != "TALK")
        return false;

    if (!(iss >> out.id))
        return false;

    std::getline(iss, out.data);
    if (!out.data.empty() && out.data.front() == ' ')
        out.data.erase(0, 1);
    return true;
}

bool parseFile(const std::string& msg, FileMsg& out) {
    std::istringstream iss(msg);
    std::string        type;
    if (!(iss >> type) || type != "FILE")
        return false;

    if (!(iss >> out.id >> out.filename >> out.filesize))
        return false;
    return true;
}

bool parseChunk(const std::string& msg, ChunkMsg& out) {
    std::istringstream iss(msg);
    std::string        type;
    if (!(iss >> type) || type != "CHUNK")
        return false;

    if (!(iss >> out.id)) {
        return false;
    }

    if (!(iss >> out.seq)) {
        return false;
    }

    std::getline(iss, out.data);
    if (!out.data.empty() && out.data.front() == ' ')
        out.data.erase(0, 1);
    return true;
}

bool parseEnd(const std::string& msg, EndMsg& out) {
    std::istringstream iss(msg);
    std::string        type;
    if (!(iss >> type) || type != "END")
        return false;
    if (!(iss >> out.id >> out.hash))
        return false;
    return true;
}

bool parseAck(const std::string& msg, AckMsg& out) {
    std::istringstream iss(msg);
    std::string        type;
    if (!(iss >> type) || type != "ACK")
        return false;
    if (!(iss >> out.id))
        return false;
    return true;
}

bool parseNack(const std::string& msg, NackMsg& out) {
    std::istringstream iss(msg);
    std::string        type;
    if (!(iss >> type) || type != "NACK")
        return false;
    if (!(iss >> out.id))
        return false;
    std::getline(iss, out.reason);
    if (!out.reason.empty() && out.reason.front() == ' ')
        out.reason.erase(0, 1);
    return true;
}

} // namespace MessageParser
