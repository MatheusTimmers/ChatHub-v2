#pragma once

#include "../device/device_manager.hpp"

#include <cstdint>
#include <string>
#include <vector>

enum class CommandType { NONE, LIST_USERS, CHAT, SEND_FILE, EXIT };

struct Command {
    CommandType type = CommandType::NONE;
    uint32_t    id   = 0;
    std::string target;
    std::string text;
    std::string filePath;
};

class UserInterface {
  public:
    Command readCommand();
    void    displayDevices(const std::vector<DeviceInfo>& users);
    void    displayMessage(const std::string& name, const std::string& msg);
};
