#include "rawhid_controller.hpp"
#include <hidapi.h>
#include <iostream>

namespace colorbot {

RawHIDController::~RawHIDController() {
    disconnect();
}

bool RawHIDController::connect(uint16_t vid, uint16_t pid) {
    if (hid_init() != 0) return false;
    
    struct hid_device_info* devs = hid_enumerate(vid, pid);
    struct hid_device_info* cur = devs;
    
    std::string rawhid_path;
    
    std::cout << "  [*] Scanning HID interfaces for VID:PID " << std::hex << vid << ":" << pid << std::dec << "\n";
    
    while (cur) {
        std::cout << "      Interface: UsagePage=0x" << std::hex << cur->usage_page 
                  << " Usage=0x" << cur->usage << std::dec << "\n";
        
        // RawHID uses Vendor-defined usage pages (0xFF00 - 0xFFFF)
        if (cur->usage_page >= 0xFF00 && rawhid_path.empty()) {
            rawhid_path = cur->path;
            std::cout << "      -> Selected as RawHID\n";
        }
        cur = cur->next;
    }
    
    hid_free_enumeration(devs);
    
    if (!rawhid_path.empty()) {
        device_ = hid_open_path(rawhid_path.c_str());
    } else {
        std::cout << "  [!] No Vendor HID interface found, trying default open...\n";
        device_ = hid_open(vid, pid, nullptr);
    }

    if (!device_) {
        std::cout << "  [!] HID device not found\n";
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
    // Report ID = 0 for RawHID, then payload starts at index 1
    // Arduino receives: rawhidData[0]=X, rawhidData[1]=Y, rawhidData[2]=Cmd
    buffer_[0] = 0x00;  // Report ID
    buffer_[1] = static_cast<uint8_t>(x);  // X -> rawhidData[0]
    buffer_[2] = static_cast<uint8_t>(y);  // Y -> rawhidData[1]  
    buffer_[3] = 0x00;  // Cmd=0 (move) -> rawhidData[2]
    
    int res = hid_write(device_, buffer_, 65);
    
    static int debugCount = 0;
    if (debugCount++ % 100 == 0) {
        std::cout << "    HID: x=" << (int)x << " y=" << (int)y << " res=" << res << "\n";
    }
}

void RawHIDController::click() {
    if (!device_) return;
    
    memset(buffer_, 0, sizeof(buffer_));
    buffer_[0] = 0x00;  // Report ID
    buffer_[1] = 0;     // X
    buffer_[2] = 0;     // Y
    buffer_[3] = 0x01;  // Cmd=1 (click)
    
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

