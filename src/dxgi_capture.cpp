#include "dxgi_capture.hpp"

namespace colorbot {

DXGIScreenCapture::DXGIScreenCapture(const ScreenRegion& region) : region_(region) {
    screenWidth_ = GetSystemMetrics(SM_CXSCREEN);
    screenHeight_ = GetSystemMetrics(SM_CYSCREEN);
    updateCapturePosition();
    initialized_ = initializeDXGI();
    lastTime_ = std::chrono::high_resolution_clock::now();
}

DXGIScreenCapture::~DXGIScreenCapture() {
    cleanup();
}

void DXGIScreenCapture::updateCapturePosition() {
    captureX_ = (screenWidth_ / 2) - (region_.width / 2) + region_.offsetX;
    captureY_ = (screenHeight_ / 2) - (region_.height / 2) + region_.offsetY;
}

void DXGIScreenCapture::setRegion(const ScreenRegion& region) {
    region_ = region;
    updateCapturePosition();
}

bool DXGIScreenCapture::initializeDXGI() {
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        0, nullptr, 0, D3D11_SDK_VERSION,
        &device_, &featureLevel, &context_
    );
    if (FAILED(hr)) return false;

    ComPtr<IDXGIDevice> dxgiDevice;
    hr = device_.As(&dxgiDevice);
    if (FAILED(hr)) return false;

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(&adapter);
    if (FAILED(hr)) return false;

    ComPtr<IDXGIOutput> output;
    hr = adapter->EnumOutputs(0, &output);
    if (FAILED(hr)) return false;

    ComPtr<IDXGIOutput1> output1;
    hr = output.As(&output1);
    if (FAILED(hr)) return false;

    hr = output1->DuplicateOutput(device_.Get(), &duplication_);
    if (FAILED(hr)) return false;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = region_.width;
    desc.Height = region_.height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    hr = device_->CreateTexture2D(&desc, nullptr, &stagingTexture_);
    if (FAILED(hr)) return false;

    buffer_ = cv::Mat(region_.height, region_.width, CV_8UC4);
    return true;
}

void DXGIScreenCapture::cleanup() {
    if (duplication_) duplication_->ReleaseFrame();
    duplication_.Reset();
    stagingTexture_.Reset();
    context_.Reset();
    device_.Reset();
    initialized_ = false;
}

cv::Mat DXGIScreenCapture::capture() {
    if (!initialized_) {
        return cv::Mat();
    }

    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    ComPtr<IDXGIResource> resource;
    
    HRESULT hr = duplication_->AcquireNextFrame(0, &frameInfo, &resource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return buffer_.clone();
    }
    if (FAILED(hr)) {
        cleanup();
        initialized_ = initializeDXGI();
        return buffer_.clone();
    }

    ComPtr<ID3D11Texture2D> desktopTexture;
    hr = resource.As(&desktopTexture);
    if (SUCCEEDED(hr)) {
        D3D11_BOX box;
        box.left = captureX_;
        box.top = captureY_;
        box.right = captureX_ + region_.width;
        box.bottom = captureY_ + region_.height;
        box.front = 0;
        box.back = 1;

        context_->CopySubresourceRegion(
            stagingTexture_.Get(), 0, 0, 0, 0,
            desktopTexture.Get(), 0, &box
        );

        D3D11_MAPPED_SUBRESOURCE mapped;
        hr = context_->Map(stagingTexture_.Get(), 0, D3D11_MAP_READ, 0, &mapped);
        if (SUCCEEDED(hr)) {
            for (int y = 0; y < region_.height; y++) {
                memcpy(
                    buffer_.ptr(y),
                    static_cast<uint8_t*>(mapped.pData) + y * mapped.RowPitch,
                    region_.width * 4
                );
            }
            context_->Unmap(stagingTexture_.Get(), 0);
        }
    }

    duplication_->ReleaseFrame();

    frameCount_++;
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime_).count();
    if (elapsed >= 1000) {
        fps_ = static_cast<float>(frameCount_) * 1000.0f / elapsed;
        frameCount_ = 0;
        lastTime_ = now;
    }

    cv::Mat result;
    cv::cvtColor(buffer_, result, cv::COLOR_BGRA2BGR);
    return result;
}

}
