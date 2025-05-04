#pragma once

#include <cstdint>
#include <string>

struct HeartbeatMsg {
    std::string name;
};

struct TalkMsg {
    uint32_t    id;
    std::string data;
};

struct FileMsg {
    uint32_t    id;
    std::string filename;
    std::size_t filesize;
};

struct ChunkMsg {
    uint32_t    id;
    uint32_t    seq;
    std::string data;
};

struct EndMsg {
    uint32_t    id;
    std::string hash;
};

struct AckMsg {
    uint32_t id;
};

struct NackMsg {
    uint32_t    id;
    std::string reason;
};

namespace MessageParser {

bool parseHeartbeat(const std::string& msg, HeartbeatMsg& out);
bool parseTalk(const std::string& msg, TalkMsg& out);
bool parseFile(const std::string& msg, FileMsg& out);
bool parseChunk(const std::string& msg, ChunkMsg& out);
bool parseEnd(const std::string& msg, EndMsg& out);
bool parseAck(const std::string& msg, AckMsg& out);
bool parseNack(const std::string& msg, NackMsg& out);

} // namespace MessageParser
