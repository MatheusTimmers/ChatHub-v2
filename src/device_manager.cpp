#include "../include/device_manager.hpp"

#include <algorithm>
#include <mutex>
#include <vector>

void DeviceManager::addDevice(const std::string& name, const std::string& ip, int port) {
    std::lock_guard<std::mutex> lock(this->mutex_);

    auto device = this->devices_.find(name);
    auto now    = std::time(nullptr);

    if (device != this->devices_.end()) {
        device->second.ip         = ip;
        device->second.port       = port;
        device->second.last_alive = now;
    }

    this->devices_[name] = DeviceInfo{name, ip, port, now};
}

std::vector<DeviceInfo> DeviceManager::listDevices() const {
    std::vector<DeviceInfo> result;
    {
        std::lock_guard<std::mutex> lock(this->mutex_);
        result.reserve(devices_.size());
        for (const auto& [name, info] : devices_) {
            result.push_back(info);
        }
    }

    // ordena por nome antes de retornar
    std::sort(result.begin(), result.end(),
              [](const DeviceInfo& a, const DeviceInfo& b) { return a.name < b.name; });
    return result;
}

void DeviceManager::removeInactiveDevices() {
    std::time_t                 now = std::time(nullptr);
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = devices_.begin(); it != devices_.end();) {
        auto diff = now - it->second.last_alive;

        if (diff > TIMEOUT_DEVICE) {
            it = devices_.erase(it);
        } else {
            ++it;
        }
    }
}
