#pragma once
#include "serial_port.hpp"

namespace colorbot {

class ArduinoMouseController {
public:
    bool connect(const std::string& port, int baudRate = 115200);
    void disconnect();
    
    void move(int8_t x, int8_t y);
    void click();
    void press();
    void release();
    
    bool isConnected() const { return serial_.isConnected(); }

private:
    SerialPort serial_;
};

}
