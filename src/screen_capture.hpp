#pragma once

#include "interfaces.hpp"
#include <Windows.h>
#include <chrono>
#include <atomic>

namespace colorbot {

class BitBltScreenCapture final : public IScreenCapture {
public:
    explicit BitBltScreenCapture(const ScreenRegion& region);
    ~BitBltScreenCapture() override;

    BitBltScreenCapture(const BitBltScreenCapture&) = delete;
    BitBltScreenCapture& operator=(const BitBltScreenCapture&) = delete;

    cv::Mat capture() override;
    float getFPS() const override { return fps_.load(); }
    void setRegion(const ScreenRegion& region) override;

private:
    void initialize();
    void cleanup();
    void updateCapturePosition();

    HDC screenDC_{nullptr};
    HDC memoryDC_{nullptr};
    HBITMAP bitmap_{nullptr};
    HGDIOBJ oldBitmap_{nullptr};
    BITMAPINFO bitmapInfo_{};
    
    ScreenRegion region_;
    int captureX_{0};
    int captureY_{0};
    int screenWidth_{0};
    int screenHeight_{0};
    
    cv::Mat buffer_;
    std::atomic<float> fps_{0.0f};
    std::chrono::high_resolution_clock::time_point lastTime_;
    int frameCount_{0};
};

}
