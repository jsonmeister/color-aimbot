#include "arduino_mouse.hpp"
#include <random>
#include <chrono>

namespace colorbot {

namespace {
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> jitter(50, 200);
}

bool ArduinoMouseController::connect(const std::string& port, int baudRate) {
    if (!serial_.connect(port, baudRate)) return false;
    Sleep(100);
    return true;
}

void ArduinoMouseController::disconnect() {
    serial_.disconnect();
}

void ArduinoMouseController::move(int8_t x, int8_t y) {
    char cmd[3] = {x, y, 0x00};
    serial_.write(cmd, 3);
    Sleep(0);
}

void ArduinoMouseController::click() {
    char cmd[3] = {0, 0, 0x01};
    serial_.write(cmd, 3);
}

void ArduinoMouseController::press() {
    char cmd[3] = {0, 0, 0x02};
    serial_.write(cmd, 3);
}

void ArduinoMouseController::release() {
    char cmd[3] = {0, 0, 0x03};
    serial_.write(cmd, 3);
}

}
