#pragma once
#include "interfaces.hpp"

namespace colorbot {

class HSVColorDetector final : public IColorDetector {
public:
    explicit HSVColorDetector(const ColorRange& range, int fovRadius);
    
    Target detect(const cv::Mat& frame) override;
    void setColorRange(const ColorRange& range) override;
    void setFOV(int radius) override { fovRadius_ = radius; }

private:
    ColorRange colorRange_;
    int fovRadius_;
    cv::Mat hsv_;
    cv::Mat mask_;
    Target lastTarget_;
};

}
