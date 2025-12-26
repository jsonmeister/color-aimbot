#include "engine.hpp"
#include <iostream>

namespace colorbot {

Engine::Engine(std::unique_ptr<IScreenCapture> capture,
               std::unique_ptr<IColorDetector> detector,
               std::unique_ptr<IAimController> aim,
               std::unique_ptr<ArduinoMouseController> mouse,
               const Config& config)
    : capture_(std::move(capture))
    , detector_(std::move(detector))
    , aim_(std::move(aim))
    , mouse_(std::move(mouse))
    , config_(config) {}

void Engine::run() {
    running_ = true;
    
    while (running_) {
        if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000) {
            running_ = false;
            break;
        }
        
        if (GetAsyncKeyState(VK_F1) & 1) {
            enabled_ = !enabled_;
            std::cout << (enabled_ ? "  [ON]\n" : "  [OFF]\n");
        }
        
        if (!enabled_) {
            Sleep(10);
            continue;
        }
        
        bool aimKeyHeld = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000);
        
        if (aimKeyHeld) {
            cv::Mat frame = capture_->capture();
            if (frame.empty()) continue;
            
            Target target = detector_->detect(frame);
            
            if (target.valid) {
                Movement delta = aim_->calculate(target, frame.cols, frame.rows);
                
                if (delta.deltaX != 0 || delta.deltaY != 0) {
                    mouse_->move(delta.deltaX, delta.deltaY);
                }
            }
        }
        
        Sleep(1);
    }
    
    std::cout << "  [EXIT]\n";
}

}
