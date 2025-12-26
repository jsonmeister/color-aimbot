#pragma once

#include "interfaces.hpp"

namespace colorbot {

class SmoothAimController final : public IAimController {
public:
    explicit SmoothAimController(const AimSettings& settings);
    
    Movement calculate(const Target& target, int frameWidth, int frameHeight) override;
    void setSmoothing(float value) override;
    void setHeadOffset(int offset) override { headOffset_ = offset; }

private:
    static int8_t clamp(int value);
    
    float smoothing_;
    int headOffset_;
};

}
