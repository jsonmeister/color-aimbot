#include "serial_port.hpp"
#include <iostream>
#include <vector>

namespace colorbot {

SerialPort::~SerialPort() {
    disconnect();
}

std::string SerialPort::autoDetectArduino() {
    std::vector<std::string> ports;
    
    for (int i = 2; i <= 20; i++) {
        std::string port = "COM" + std::to_string(i);
        HANDLE h = CreateFileA(("\\\\.\\"+port).c_str(), GENERIC_READ|GENERIC_WRITE, 
                               0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h != INVALID_HANDLE_VALUE) {
            CloseHandle(h);
            ports.push_back(port);
        }
    }
    
    for (const auto& p : ports) if (p != "COM2") return p;
    return ports.empty() ? "" : ports[0];
}

bool SerialPort::connect(const std::string& port, int baudRate) {
    std::string actualPort = port;
    
    if (port.empty() || port == "AUTO") {
        actualPort = autoDetectArduino();
        if (actualPort.empty()) {
            lastError_ = "No COM ports found";
            return false;
        }
        std::cout << "  [+] Auto-detected: " << actualPort << "\n";
    }
    
    handle_ = CreateFileA(("\\\\.\\"+actualPort).c_str(), GENERIC_READ|GENERIC_WRITE,
                          0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    
    if (handle_ == INVALID_HANDLE_VALUE) {
        if (port != "AUTO") {
            actualPort = autoDetectArduino();
            if (!actualPort.empty() && actualPort != port) {
                handle_ = CreateFileA(("\\\\.\\"+actualPort).c_str(), GENERIC_READ|GENERIC_WRITE,
                                      0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            }
        }
        if (handle_ == INVALID_HANDLE_VALUE) {
            lastError_ = "Failed to open: " + actualPort;
            return false;
        }
    }
    
    DCB dcb{};
    dcb.DCBlength = sizeof(DCB);
    GetCommState(handle_, &dcb);
    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    
    if (!SetCommState(handle_, &dcb)) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
        lastError_ = "Failed to configure port";
        return false;
    }
    
    COMMTIMEOUTS to{1, 1, 1, 1, 1};
    SetCommTimeouts(handle_, &to);
    PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);
    
    connected_ = true;
    return true;
}

void SerialPort::disconnect() {
    if (handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
    }
    connected_ = false;
}

bool SerialPort::write(const char* data, size_t size) {
    if (!connected_) return false;
    DWORD written;
    return WriteFile(handle_, data, (DWORD)size, &written, nullptr) && written == size;
}

int SerialPort::read(char* buffer, size_t size) {
    if (!connected_) return -1;
    DWORD bytesRead;
    return ReadFile(handle_, buffer, (DWORD)size, &bytesRead, nullptr) ? (int)bytesRead : -1;
}

}
