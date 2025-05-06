#pragma once

#include <atomic>
#include <cstdint>
#include <ctime>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct DeviceInfo {
    std::string name;
    std::string ip;
    int         port;
    sockaddr_in addr;
    std::time_t last_alive;
    std::unordered_set<int> received_ids;
};

class DeviceManager {
  private:
    std::string name_;

    std::unordered_map<std::string, DeviceInfo>  devices_;
    std::unordered_map<std::string, std::string> devicesByAddr_;
    mutable std::mutex                           mutex_;

    void removeInactiveDevices(int timeout, int interval);

    std::atomic<bool> cleanupRunning_{false};
    std::thread       cleanupThread_;

  public:
    DeviceManager(const std::string& name);
    void addOrUpdate(const std::string& name, const sockaddr_in& from);

    std::string getNameByAddr(const sockaddr_in& addr);
    DeviceInfo getDeviceInfoByName(const std::string& name);

    std::vector<DeviceInfo> listDevices() const;

    void startCleanup(int timeout, int interval);
    void stopCleanup();
};
