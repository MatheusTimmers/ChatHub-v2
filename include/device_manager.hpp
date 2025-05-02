#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <ctime>

#define TIMEOUT_DEVICE 15 // seconds

struct DeviceInfo {
    std::string name;
    std::string ip;
    int port;
    std::time_t last_alive;
};

class DeviceManager{
private:
    std::unordered_map<std::string, DeviceInfo> devices_;
    mutable std::mutex mutex_;

public:
    // Adiciona um novo device na lista
    void addDevice(const std::string& name, const std::string& ip, int port);
    // Remove devices inativos
    void removeInactiveDevices();

    // Retorna lista de devices
    std::vector<DeviceInfo> listDevices() const;
};
