#include "aim_controller.hpp"
#include <algorithm>
#include <cmath>

namespace colorbot {

SmoothAimController::SmoothAimController(const AimSettings& settings)
    : smoothing_(settings.smoothing), headOffset_(settings.headOffset) {}

void SmoothAimController::setSmoothing(float value) {
    smoothing_ = std::clamp(value, 0.05f, 1.0f);
}

int8_t SmoothAimController::clamp(int value) {
    return static_cast<int8_t>(std::clamp(value, -127, 127));
}

Movement SmoothAimController::calculate(const Target& target, int frameWidth, int frameHeight) {
    const int centerX = frameWidth / 2;
    const int centerY = frameHeight / 2;
    
    const int relX = target.x - centerX;
    const int relY = (target.y + headOffset_) - centerY;
    
    const float smoothedX = relX * smoothing_;
    const float smoothedY = relY * smoothing_;
    
    return {
        clamp(static_cast<int>(smoothedX)),
        clamp(static_cast<int>(smoothedY))
    };
}

}
