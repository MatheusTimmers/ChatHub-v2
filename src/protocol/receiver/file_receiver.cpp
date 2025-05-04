#include "../../../include/protocol/receiver/file_receiver.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

void FileReceiver::startReceive(std::string fileId, const std::string& filename) {
    this->contexts_[fileId] = FileReceiveContext{filename, {}};
}

void FileReceiver::writeChunk(std::string fileId, uint32_t seq, const std::string& data) {
    auto it = contexts_.find(fileId);
    if (it == contexts_.end())
        return;
    it->second.chunks.emplace_back(seq, data);
}

bool FileReceiver::finishReceive(std::string fileId) {
    auto it = contexts_.find(fileId);
    if (it == contexts_.end())
        return false;

    auto& ctx = it->second;
    std::sort(ctx.chunks.begin(), ctx.chunks.end(),
              [](const std::pair<uint32_t, std::string>& a,
                 const std::pair<uint32_t, std::string>& b) { return a.first < b.first; });

    std::ofstream file(ctx.filename, std::ios::binary);

    if (!file) {
        std::cerr << "[FileReceiver] Erro ao abrir arquivo para escrita: " << ctx.filename
                  << std::endl;
        return false;
    }

    for (auto& p : ctx.chunks) {
        file.write(p.second.data(), p.second.size());
    }
    file.close();

    this->contexts_.erase(it);
    return true;
}
