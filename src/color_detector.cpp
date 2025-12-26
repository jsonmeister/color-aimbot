#include "color_detector.hpp"
#include <cmath>
#include <algorithm>

namespace colorbot {

HSVColorDetector::HSVColorDetector(const ColorRange& range, int fovRadius)
    : colorRange_(range), fovRadius_(fovRadius) {}

void HSVColorDetector::setColorRange(const ColorRange& range) {
    colorRange_ = range;
}

Target HSVColorDetector::detect(const cv::Mat& frame) {
    Target target;
    if (frame.empty()) return target;
    
    const int centerX = frame.cols / 2;
    const int centerY = frame.rows / 2;
    
    cv::cvtColor(frame, hsv_, cv::COLOR_BGR2HSV);
    cv::inRange(hsv_, colorRange_.lower, colorRange_.upper, mask_);
    
    // Find contours for better target detection
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask_, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    if (contours.empty()) return target;
    
    // Find closest contour center to screen center
    float minDist = static_cast<float>(fovRadius_);
    cv::Point bestPoint(-1, -1);
    
    for (const auto& contour : contours) {
        if (contour.size() < 3) continue;
        
        cv::Moments m = cv::moments(contour);
        if (m.m00 < 10) continue;  // Skip tiny contours
        
        int cx = static_cast<int>(m.m10 / m.m00);
        int cy = static_cast<int>(m.m01 / m.m00);
        
        // Get top of contour (head position)
        auto topPoint = *std::min_element(contour.begin(), contour.end(),
            [](const cv::Point& a, const cv::Point& b) { return a.y < b.y; });
        
        int targetX = cx;
        int targetY = topPoint.y + 3;  // Small offset for headshot
        
        float dx = static_cast<float>(targetX - centerX);
        float dy = static_cast<float>(targetY - centerY);
        float dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist < minDist) {
            minDist = dist;
            bestPoint = cv::Point(targetX, targetY);
        }
    }
    
    if (bestPoint.x >= 0) {
        // Target tracking - prefer staying on same target
        if (lastTarget_.valid) {
            float lastDx = static_cast<float>(bestPoint.x - lastTarget_.x);
            float lastDy = static_cast<float>(bestPoint.y - lastTarget_.y);
            float lastDist = std::sqrt(lastDx * lastDx + lastDy * lastDy);
            
            // If new target is far from last, apply hysteresis
            if (lastDist > 30) {
                float toCenterX = static_cast<float>(lastTarget_.x - centerX);
                float toCenterY = static_cast<float>(lastTarget_.y - centerY);
                float toCenterDist = std::sqrt(toCenterX * toCenterX + toCenterY * toCenterY);
                
                // Keep last target if still closer
                if (toCenterDist < minDist * 0.8f) {
                    bestPoint = cv::Point(lastTarget_.x, lastTarget_.y);
                }
            }
        }
        
        target.x = bestPoint.x;
        target.y = bestPoint.y;
        target.confidence = 1.0f - (minDist / fovRadius_);
        target.valid = true;
        lastTarget_ = target;
    } else {
        lastTarget_.valid = false;
    }
    
    return target;
}

}
