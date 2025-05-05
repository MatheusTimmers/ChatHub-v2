#include "../../include/device/device_manager.hpp"
#include "../../include/utils/utils.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cstdint>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <vector>

DeviceManager::DeviceManager(const std::string& name) : name_(std::move(name)) {}

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

void DeviceManager::addOrUpdate(const std::string& name, const sockaddr_in& from) {
    std::lock_guard<std::mutex> lock(this->mutex_);

    // Verifica se nao ta adicionando ele mesmo
    if (name == this->name_)
        return;

    std::time_t                 now = std::time(nullptr);

    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &from.sin_addr, ipstr, sizeof(ipstr));
    uint16_t port = ntohs(from.sin_port);

    std::string key = std::string(ipstr) + ":" + std::to_string(port);

    this->devices_[name]      = DeviceInfo{name, ipstr, port, from, now};
    this->devicesByAddr_[key] = name;
}

std::vector<DeviceInfo> DeviceManager::listDevices() const {
    std::vector<DeviceInfo> result;
    {
        std::lock_guard<std::mutex> lock(this->mutex_);
        result.reserve(devices_.size());
        for (auto it = devices_.begin(); it != devices_.end(); it++) {
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
    std::string key = peerKey(addr);

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
