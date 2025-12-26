#pragma once
#include "interfaces.hpp"
#include "arduino_mouse.hpp"
#include "config.hpp"
#include <memory>
#include <Windows.h>

namespace colorbot {

class Engine {
public:
    Engine(std::unique_ptr<IScreenCapture> capture,
           std::unique_ptr<IColorDetector> detector,
           std::unique_ptr<IAimController> aim,
           std::unique_ptr<ArduinoMouseController> mouse,
           const Config& config);
    
    void run();
    void stop() { running_ = false; }

private:
    std::unique_ptr<IScreenCapture> capture_;
    std::unique_ptr<IColorDetector> detector_;
    std::unique_ptr<IAimController> aim_;
    std::unique_ptr<ArduinoMouseController> mouse_;
    Config config_;
    bool running_ = false;
    bool enabled_ = false;
};

}
