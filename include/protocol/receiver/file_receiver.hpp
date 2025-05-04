#pragma once

#include <netinet/in.h>
#include <string>
#include <unordered_map>
#include <vector>

struct FileReceiveContext {
    std::string                                   filename;
    std::vector<std::pair<uint32_t, std::string>> chunks;
};

class FileReceiver {
  public:
    void startReceive(std::string fileId, const std::string& filename);
    void writeChunk(std::string fileId, uint32_t seq, const std::string& data);
    bool finishReceive(std::string fileId);

  private:
    std::unordered_map<std::string, FileReceiveContext> contexts_;
};
