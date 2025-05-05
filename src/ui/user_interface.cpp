#include "../../include/ui/user_interface.hpp"

#include <iostream>
#include <sstream>

Command UserInterface::readCommand() {
    std::cout << ">> ";
    std::string line;

    if (!std::getline(std::cin, line)) {
        return Command{};
    }

    std::istringstream iss(line);
    std::string        cmd;
    iss >> cmd;

    Command result;
    if (cmd == "devices") {
        result.type = CommandType::LIST_USERS;

    } else if (cmd == "talk") {
        result.type = CommandType::CHAT;
        iss >> result.target;
        std::getline(iss, result.text);
        if (!result.text.empty() && result.text.front() == ' ')
            result.text.erase(0, 1);

    } else if (cmd == "sendfile") {
        result.type = CommandType::SEND_FILE;
        iss >> result.target >> result.filePath;

    } else if (cmd == "exit") {
        result.type = CommandType::EXIT;

    } else {
        std::cout << "Comando desconhecido: " << cmd << std::endl;
    }
    return result;
}

void UserInterface::displayDevices(const std::vector<DeviceInfo>& users) {
    if (users.empty()) {
        std::cout << "Voce esta sozinho. Nao tem ninguem aqui 🦗" << std::endl;
    } else {
        std::cout << "Usuarios ativos:" << std::endl;
        for (auto& u : users) {
            std::cout << u.name << " @ " << u.ip << ":" << u.port << " LastAlive: " << u.last_alive
                      << std::endl;
        }
    }
}

void UserInterface::displayMessage(const std::string& from, const std::string& msg) {
    std::cout << from << ": " << msg << std::endl;
    std::cout << ">> ";
    std::cout << std::flush;
}
