#pragma once

#include <opencv2/opencv.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <functional>

namespace colorbot {

struct Target {
    int x{0};
    int y{0};
    float confidence{0.0f};
    bool valid{false};
};

struct Movement {
    int8_t deltaX{0};
    int8_t deltaY{0};
};

struct ScreenRegion {
    int width{250};
    int height{250};
    int offsetX{0};
    int offsetY{0};
};

struct ColorRange {
    cv::Scalar lower{140, 110, 150};
    cv::Scalar upper{150, 195, 255};
};

struct AimSettings {
    int fovRadius{100};
    float smoothing{0.3f};
    int headOffset{-5};
};

class IScreenCapture {
public:
    virtual ~IScreenCapture() = default;
    virtual cv::Mat capture() = 0;
    virtual float getFPS() const = 0;
    virtual void setRegion(const ScreenRegion& region) = 0;
};

class IColorDetector {
public:
    virtual ~IColorDetector() = default;
    virtual Target detect(const cv::Mat& frame) = 0;
    virtual void setColorRange(const ColorRange& range) = 0;
    virtual void setFOV(int radius) = 0;
};

class IAimController {
public:
    virtual ~IAimController() = default;
    virtual Movement calculate(const Target& target, int frameWidth, int frameHeight) = 0;
    virtual void setSmoothing(float value) = 0;
    virtual void setHeadOffset(int offset) = 0;
};

class IMouseController {
public:
    virtual ~IMouseController() = default;
    virtual bool connect(const std::string& port, int baudRate = 115200) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual void move(int8_t x, int8_t y) = 0;
    virtual void click() = 0;
    virtual std::string getLastError() const = 0;
};

class IInputHandler {
public:
    virtual ~IInputHandler() = default;
    virtual bool isKeyPressed(int vkCode) const = 0;
    virtual bool isKeyHeld(int vkCode) const = 0;
};

using ScreenCaptureFactory = std::function<std::unique_ptr<IScreenCapture>(const ScreenRegion&)>;
using ColorDetectorFactory = std::function<std::unique_ptr<IColorDetector>(const ColorRange&, int fov)>;
using AimControllerFactory = std::function<std::unique_ptr<IAimController>(const AimSettings&)>;
using MouseControllerFactory = std::function<std::unique_ptr<IMouseController>()>;

}
