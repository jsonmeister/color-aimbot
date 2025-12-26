#pragma once
#include <string>
#include <vector>
#include <cstdint>

// Forward declaration for hidapi
struct hid_device_;
typedef struct hid_device_ hid_device;

namespace colorbot {

class RawHIDController {
public:
    ~RawHIDController();
    
    bool connect(uint16_t vid, uint16_t pid);
    void disconnect();
    bool isConnected() const { return device_ != nullptr; }
    
    void move(int8_t x, int8_t y);
    void click();
    
    static std::vector<std::pair<uint16_t, uint16_t>> listDevices();

private:
    hid_device* device_ = nullptr;
    uint8_t buffer_[64] = {0};
};

}
