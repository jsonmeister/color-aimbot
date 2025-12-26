#pragma once
#include <Windows.h>
#include <string>

namespace colorbot {

class SerialPort {
public:
    ~SerialPort();
    
    static std::string autoDetectArduino();
    
    bool connect(const std::string& port, int baudRate = 115200);
    void disconnect();
    bool isConnected() const { return connected_; }
    bool write(const char* data, size_t size);
    int read(char* buffer, size_t size);
    std::string getLastError() const { return lastError_; }

private:
    HANDLE handle_ = INVALID_HANDLE_VALUE;
    bool connected_ = false;
    std::string lastError_;
};

}
