#include "../../include/device/device_manager.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <vector>

void DeviceManager::startCleanup(int timeout, int interval) {
    if (this->cleanupRunning_)
        return;

    this->cleanupRunning_ = true;
    this->cleanupThread_ =
        std::thread(&DeviceManager::removeInactiveDevices, this, timeout, interval);
}

void DeviceManager::stopCleanup() {
    cleanupRunning_ = false;
    if (cleanupThread_.joinable())
        cleanupThread_.join();
}

void DeviceManager::removeInactiveDevices(int timeout, int interval) {
    while (cleanupRunning_) {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            auto                        now = std::time(nullptr);
            for (auto it = devices_.begin(); it != devices_.end();) {
                if (now - it->second.last_alive > timeout)
                    it = devices_.erase(it);
                else
                    ++it;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}

void DeviceManager::addOrUpdate(const std::string& name, const std::string& ip, uint16_t port) {
    std::lock_guard<std::mutex> lock(this->mutex_);
    std::string                 key = ip + ":" + std::to_string(port);
    auto                        now = std::time(nullptr);

    // NOTE: Isso tinha que ser feito pelo socket, deveriamos controlar apenas o ip e porta
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        std::clog << "[DeviceManager] Error: IP inválido `" << ip << "` para o dispositivo `"
                  << name << "`\n";
    }

    this->devices_[name]      = DeviceInfo{name, ip, port, addr, now};
    this->devicesByAddr_[key] = name;
}

std::vector<DeviceInfo> DeviceManager::listDevices() const {
    std::vector<DeviceInfo> result;
    {
        std::lock_guard<std::mutex> lock(this->mutex_);
        result.reserve(devices_.size());
        for (auto it = devices_.begin(); it != devices_.end();) {
            result.push_back(it->second);
        }
    }

    // ordena por nome antes de retornar
    std::sort(result.begin(), result.end(),
              [](const DeviceInfo& a, const DeviceInfo& b) { return a.name < b.name; });
    return result;
}

std::string DeviceManager::getNameByAddr(const sockaddr_in& addr) {
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ipstr, sizeof(ipstr));
    uint16_t port = ntohs(addr.sin_port);

    std::string key = std::string(ipstr) + ":" + std::to_string(port);

    auto it = devicesByAddr_.find(key);
    if (it != devicesByAddr_.end()) {
        return it->second;
    }

    return key; // se não encontrou retorna o ip mais porta
}

DeviceInfo DeviceManager::getDeviceInfoByName(const std::string& name) {
    auto it = devices_.find(name);
    if (it != devices_.end()) {
        return it->second;
    }

    return {};
}
