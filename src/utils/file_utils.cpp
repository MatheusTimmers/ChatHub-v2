#include "../../include/utils/file_utils.hpp"
#include <openssl/evp.h>
#include <fstream>
#include <sstream>
#include <iomanip>

std::string computeSHA256(const std::string& path) {
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) return {};

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(mdctx);
        return {};
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        EVP_MD_CTX_free(mdctx);
        return {};
    }

    char buf[8192];
    while (file.good()) {
        file.read(buf, sizeof(buf));
        std::streamsize n = file.gcount();
        if (n > 0) {
            EVP_DigestUpdate(mdctx, buf, static_cast<size_t>(n));
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int  hash_len = 0;
    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        return {};
    }

    EVP_MD_CTX_free(mdctx);

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hash_len; ++i) {
        ss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return ss.str();
}
