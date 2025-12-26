#pragma once

#include "interfaces.hpp"
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <atomic>
#include <chrono>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace colorbot {

using Microsoft::WRL::ComPtr;

class DXGIScreenCapture final : public IScreenCapture {
public:
    explicit DXGIScreenCapture(const ScreenRegion& region);
    ~DXGIScreenCapture() override;

    DXGIScreenCapture(const DXGIScreenCapture&) = delete;
    DXGIScreenCapture& operator=(const DXGIScreenCapture&) = delete;

    cv::Mat capture() override;
    float getFPS() const override { return fps_.load(); }
    void setRegion(const ScreenRegion& region) override;

private:
    bool initializeDXGI();
    void cleanup();
    void updateCapturePosition();

    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<IDXGIOutputDuplication> duplication_;
    ComPtr<ID3D11Texture2D> stagingTexture_;
    
    ScreenRegion region_;
    int captureX_{0};
    int captureY_{0};
    int screenWidth_{0};
    int screenHeight_{0};
    
    cv::Mat buffer_;
    std::atomic<float> fps_{0.0f};
    std::chrono::high_resolution_clock::time_point lastTime_;
    int frameCount_{0};
    
    bool initialized_{false};
};

}
