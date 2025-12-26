#include "rawhid_controller.hpp"
#include <hidapi.h>
#include <iostream>

namespace colorbot {

RawHIDController::~RawHIDController() {
    disconnect();
}

bool RawHIDController::connect(uint16_t vid, uint16_t pid) {
    if (hid_init() != 0) return false;
    
    device_ = hid_open(vid, pid, nullptr);
    if (!device_) {
        std::cout << "  [!] HID device not found: " << std::hex << vid << ":" << pid << std::dec << "\n";
        return false;
    }
    
    hid_set_nonblocking(device_, 1);
    std::cout << "  [+] HID device connected\n";
    return true;
}

void RawHIDController::disconnect() {
    if (device_) {
        hid_close(device_);
        device_ = nullptr;
    }
    hid_exit();
}

void RawHIDController::move(int8_t x, int8_t y) {
    if (!device_) return;
    
    memset(buffer_, 0, sizeof(buffer_));
    buffer_[0] = 0x00;
    buffer_[1] = static_cast<uint8_t>(x);
    buffer_[2] = static_cast<uint8_t>(y);
    buffer_[3] = 0x00;
    
    int res = hid_write(device_, buffer_, 65);
    
    static int debugCount = 0;
    if (debugCount++ % 50 == 0) {
        std::cout << "    HID Write: x=" << (int)x << " y=" << (int)y << " res=" << res << "\n";
    }
}

void RawHIDController::click() {
    if (!device_) return;
    
    memset(buffer_, 0, sizeof(buffer_));
    buffer_[0] = 0x00;
    buffer_[1] = 0;
    buffer_[2] = 0;
    buffer_[3] = 0x01;
    
    hid_write(device_, buffer_, 65);
}

std::vector<std::pair<uint16_t, uint16_t>> RawHIDController::listDevices() {
    std::vector<std::pair<uint16_t, uint16_t>> devices;
    if (hid_init() != 0) return devices;
    
    struct hid_device_info* devs = hid_enumerate(0, 0);
    struct hid_device_info* cur = devs;
    
    while (cur) {
        devices.push_back({cur->vendor_id, cur->product_id});
        cur = cur->next;
    }
    
    hid_free_enumeration(devs);
    hid_exit();
    return devices;
}

}
