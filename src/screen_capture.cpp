#include "screen_capture.hpp"

namespace colorbot {

BitBltScreenCapture::BitBltScreenCapture(const ScreenRegion& region) : region_(region) {
    initialize();
    lastTime_ = std::chrono::high_resolution_clock::now();
}

BitBltScreenCapture::~BitBltScreenCapture() {
    cleanup();
}

void BitBltScreenCapture::initialize() {
    screenWidth_ = GetSystemMetrics(SM_CXSCREEN);
    screenHeight_ = GetSystemMetrics(SM_CYSCREEN);
    updateCapturePosition();
    
    screenDC_ = GetDC(nullptr);
    memoryDC_ = CreateCompatibleDC(screenDC_);
    bitmap_ = CreateCompatibleBitmap(screenDC_, region_.width, region_.height);
    oldBitmap_ = SelectObject(memoryDC_, bitmap_);
    
    bitmapInfo_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo_.bmiHeader.biWidth = region_.width;
    bitmapInfo_.bmiHeader.biHeight = -region_.height;
    bitmapInfo_.bmiHeader.biPlanes = 1;
    bitmapInfo_.bmiHeader.biBitCount = 32;
    bitmapInfo_.bmiHeader.biCompression = BI_RGB;
    
    buffer_ = cv::Mat(region_.height, region_.width, CV_8UC4);
}

void BitBltScreenCapture::cleanup() {
    if (oldBitmap_) SelectObject(memoryDC_, oldBitmap_);
    if (bitmap_) DeleteObject(bitmap_);
    if (memoryDC_) DeleteDC(memoryDC_);
    if (screenDC_) ReleaseDC(nullptr, screenDC_);
    
    oldBitmap_ = nullptr;
    bitmap_ = nullptr;
    memoryDC_ = nullptr;
    screenDC_ = nullptr;
}

void BitBltScreenCapture::updateCapturePosition() {
    captureX_ = (screenWidth_ / 2) - (region_.width / 2) + region_.offsetX;
    captureY_ = (screenHeight_ / 2) - (region_.height / 2) + region_.offsetY;
}

void BitBltScreenCapture::setRegion(const ScreenRegion& region) {
    cleanup();
    region_ = region;
    initialize();
}

cv::Mat BitBltScreenCapture::capture() {
    BitBlt(memoryDC_, 0, 0, region_.width, region_.height, 
           screenDC_, captureX_, captureY_, SRCCOPY);
    GetDIBits(memoryDC_, bitmap_, 0, region_.height, buffer_.data, &bitmapInfo_, DIB_RGB_COLORS);
    
    cv::Mat result;
    cv::cvtColor(buffer_, result, cv::COLOR_BGRA2BGR);
    
    frameCount_++;
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime_).count();
    
    if (elapsed >= 1000) {
        fps_ = static_cast<float>(frameCount_) * 1000.0f / elapsed;
        frameCount_ = 0;
        lastTime_ = now;
    }
    
    return result;
}

}
