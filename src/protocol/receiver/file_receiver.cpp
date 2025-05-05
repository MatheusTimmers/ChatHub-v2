#include "../../../include/protocol/receiver/file_receiver.hpp"

#include "../../../include/utils/file_utils.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

void FileReceiver::startReceive(const std::string& fileId, const std::string& filename) {
    this->contexts_[fileId] = FileReceiveContext{filename, {}};
}

void FileReceiver::writeChunk(const std::string& fileId, uint32_t seq, const std::string& data) {
    auto it = contexts_.find(fileId);
    if (it == contexts_.end())
        return;
    it->second.chunks.emplace_back(seq, data);
}

bool FileReceiver::finishReceive(const std::string& fileId, const std::string& hash,
                                 std::string& error) {
    auto it = contexts_.find(fileId);
    if (it == contexts_.end())
        return false;

    auto& ctx = it->second;
    std::sort(ctx.chunks.begin(), ctx.chunks.end(),
              [](const std::pair<uint32_t, std::string>& a,
                 const std::pair<uint32_t, std::string>& b) { return a.first < b.first; });

    std::ofstream file("twteste.txt", std::ios::binary);

    if (!file) {
        std::cerr << "[FileReceiver] Erro ao abrir arquivo para escrita: " << ctx.filename
                  << std::endl;
        error = "Não foi possivel criar arquivo no destino";

        return false;
    }

    for (auto& p : ctx.chunks) {
        file.write(p.second.data(), p.second.size());
    }
    file.put('\n');
    file.close();

    std::string actualHash = computeSHA256("twteste.txt");
    if (actualHash != hash) {
        std::cerr << "[FileReceiver] checksum mismatch: expected " << hash << " but got "
                  << actualHash << "\n";

        error = "Dados foram corrompidos";

        //if (std::remove(ctx.filename.c_str()) != 0) {
        //    std::perror("[FileReceiver] falha ao deletar arquivo corrompido");
        //}

        // não apaga o contexto que ele vai tentar enviar de novo
        return false;
    }

    this->contexts_.erase(it);
    return true;
}
