#include "../include/user_net.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <nome> <porta>\n";
        return 1;
    }

    std::string nome = argv[1];
    int porta = std::stoi(argv[2]);

    UserNet unet(nome, porta);
    unet.start();
}

