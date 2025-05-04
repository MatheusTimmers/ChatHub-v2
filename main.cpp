#include "include/user_net.hpp"

#include <cstdint>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 5) {
        std::cerr << "Uso: " << argv[0] << " <nome> <porta> ou <nome> <porta> <ip>\n";
        return 1;
    }

    std::string nome  = argv[1];
    uint16_t    porta = std::stoi(argv[2]);
    std::string ip    = "";
    if (argc == 4) {
        ip = argv[3];
    }

    UserNet unet(nome, ip, porta);
    unet.start();
}
