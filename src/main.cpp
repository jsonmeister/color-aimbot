#include "engine.hpp"
#include "dxgi_capture.hpp"
#include "screen_capture.hpp"
#include "color_detector.hpp"
#include "aim_controller.hpp"
#include "rawhid_controller.hpp"
#include "config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>
#include <chrono>
#include <filesystem>

using namespace colorbot;

constexpr uint16_t STEELSERIES_VID = 0x1038;
constexpr uint16_t RIVAL3_PID = 0x1824;


struct TriggerSettings {
    int minDelay = 75;
    int maxDelay = 125;
    int xRange = 15;
    int yRange = 15;
    int assistRange = 6; // Increased range
};

class ColorDetector {
public:
    ColorDetector(const cv::Scalar& lower, const cv::Scalar& upper)
        : lowerColor_(lower), upperColor_(upper), kernel_(cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3))) {}
    
    struct Target {
        int x = -1, y = -1;
        cv::Rect rect;
        std::vector<cv::Point> contour;
        bool valid = false;
    };
    
    Target detect(const cv::Mat& frame, int centerX, int centerY) {
        cv::cvtColor(frame, hsv_, cv::COLOR_BGR2HSV);
        cv::inRange(hsv_, lowerColor_, upperColor_, mask_);
        cv::dilate(mask_, dilated_, kernel_, cv::Point(-1, -1), 5);
        cv::threshold(dilated_, thresh_, 60, 255, cv::THRESH_BINARY);
        
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(thresh_, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        Target target;
        if (contours.empty()) return target;
        
        float minDistance = std::numeric_limits<float>::max();
        
        for (const auto& contour : contours) {
            cv::Rect r = cv::boundingRect(contour);
            int cx = r.x + r.width / 2;
            int cy = r.y + r.height / 2;
            
            float distance = std::sqrt(
                static_cast<float>((cx - centerX) * (cx - centerX) + (cy - centerY) * (cy - centerY))
            );
            
            if (distance < minDistance) {
                minDistance = distance;
                target.x = cx;
                target.y = r.y; // Top of the box (Head reference)
                target.rect = r;
                target.contour = contour;
                target.valid = true;
            }
        }
        
        return target;
    }
    
    const cv::Mat& getMask() const { return thresh_; }
    
private:
    cv::Scalar lowerColor_, upperColor_;
    cv::Mat kernel_, hsv_, mask_, dilated_, thresh_;
};

class MouseController {
public:
    MouseController() : remainderX_(0.0f), remainderY_(0.0f) {}
    
    bool connect(uint16_t vid, uint16_t pid) {
        return mouse_.connect(vid, pid);
    }
    
    void disconnect() {
        mouse_.disconnect();
    }
    
    void move(float x, float y) {
        x += remainderX_;
        y += remainderY_;
        
        int moveX = static_cast<int>(x);
        int moveY = static_cast<int>(y);
        
        remainderX_ = x - moveX;
        remainderY_ = y - moveY;
        
        if (moveX != 0 || moveY != 0) {
            int8_t mx = static_cast<int8_t>(std::clamp(moveX, -127, 127));
            int8_t my = static_cast<int8_t>(std::clamp(moveY, -127, 127));
            mouse_.move(mx, my);
        }
    }
    
    void click() {
        mouse_.click();
    }
    
private:
    RawHIDController mouse_;
    float remainderX_, remainderY_;
};



int main() {
    SetConsoleTitleA("Research Tool");
    std::cout << "\n  [Colorbot v4.0 - Electron UI Integration]\n\n";
    
    std::string configPath = "config.json";
    Config config = Config::load(configPath);
    auto lastConfigTime = std::filesystem::exists(configPath) ? std::filesystem::last_write_time(configPath) : std::filesystem::file_time_type();
    
    // Initial profile load
    ProfileSettings aim = config.profiles[config.activeProfile];
    TriggerSettings trigger{75, 125, 15, 15, 6}; 
    
    std::cout << "  Profile: " << config.activeProfile << "\n";
    std::cout << "  Speed: " << aim.xSpeed << "x/" << aim.ySpeed << "y | Offset: " << aim.targetOffset << "\n\n";
    
    std::unique_ptr<IScreenCapture> capture;
    try {
        capture = std::make_unique<DXGIScreenCapture>(config.screen);
        std::cout << "  [+] DXGI capture\n";
    } catch (...) {
        capture = std::make_unique<BitBltScreenCapture>(config.screen);
        std::cout << "  [!] BitBlt fallback\n";
    }
    
    ColorDetector detector(config.color.lower, config.color.upper);
    MouseController mouse;
    
    if (!mouse.connect(STEELSERIES_VID, RIVAL3_PID)) {
        std::cout << "  [!] HID device not found\n";
        system("pause");
        return 1;
    }
    std::cout << "  [+] HID connected\n\n";
    std::cout << "  F1=Toggle | F2=Profile | Mouse5=Aim | ALT=Trigger | Numpad5=Exit\n\n";
    
    if (config.debugMode) {
        cv::namedWindow("Debug", cv::WINDOW_NORMAL);
        cv::resizeWindow("Debug", 400, 400);
    }
    
    bool running = true, enabled = false;
    const int centerX = config.screen.width / 2;
    const int centerY = config.screen.height / 2;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> humanDistY(-1, 1);
    std::uniform_int_distribution<> humanDistX(-1, 1);
    std::bernoulli_distribution headshotDist(0.7); 
    
    int patternTick = 0;
    int offsetVariationY = 0, offsetVariationX = 0;
    bool shouldHitHead = true;
    auto lastProbUpdate = std::chrono::high_resolution_clock::now();
    
    auto lastTrigger = std::chrono::high_resolution_clock::now();
    auto lastConfigCheck = std::chrono::steady_clock::now();
    
    while (running) {
        if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000) break;
        
        // --- Config Hot-Reload ---
        auto nowSteady = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(nowSteady - lastConfigCheck).count() > 1000) {
            if (std::filesystem::exists(configPath)) {
                auto currentWriteTime = std::filesystem::last_write_time(configPath);
                if (currentWriteTime > lastConfigTime) {
                    std::cout << "  [~] Config changed, reloading...\n";
                    config = Config::load(configPath);
                    aim = config.profiles[config.activeProfile]; // Update active aim settings directly
                    
                    // Update detector colors if changed
                    // For now assuming we just re-instantiate or update (Detector needs update method or just simpler)
                    // detector = ColorDetector(config.color.lower, config.color.upper); // Recreating detector is expensive? Structuring element is cached.
                    // Ideally we add setColors to Detector. For now, creating new is fast enough.
                    detector = ColorDetector(config.color.lower, config.color.upper);
                    
                    lastConfigTime = currentWriteTime;
                    std::cout << "  [+] Reloaded. Profile: " << config.activeProfile << "\n";
                }
            }
            lastConfigCheck = nowSteady;
        }

        if (GetAsyncKeyState(VK_F1) & 1) {
            enabled = !enabled;
            std::cout << (enabled ? "  [ON]\n" : "  [OFF]\n");
        }
        
        // F2 Cycle (Legacy override, might conflict with UI so maybe remove or keep sync?)
        // If UI is primary, F2 locally changes but config file might stay. 
        // Let's keep F2 for now.
        if (GetAsyncKeyState(VK_F2) & 1) {
            if (config.activeProfile == "legit") config.activeProfile = "semi";
            else if (config.activeProfile == "semi") config.activeProfile = "rage";
            else config.activeProfile = "legit";
            aim = config.profiles[config.activeProfile];
            std::cout << "  Profile: " << config.activeProfile << " (Speed: " << aim.xSpeed << ")\n";
        }
        
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastProbUpdate).count() > 2) {
            shouldHitHead = headshotDist(gen);
            lastProbUpdate = now;
        }
        
        cv::Mat frame = capture->capture();
        if (frame.empty()) { Sleep(1); continue; }
        
        auto target = detector.detect(frame, centerX, centerY);
        
        // Aim Point Calculation
        if (target.valid) {
            target.y += static_cast<int>(aim.targetOffset);
            if (!shouldHitHead && aim.humanize) {
                 target.y += 15; // Body aim
            }
        }
        
        bool inTriggerZone = false;
        double distToContour = 1000.0;
        
        if (target.valid) {
             cv::Point2f centerPt((float)centerX, (float)centerY);
             distToContour = cv::pointPolygonTest(target.contour, centerPt, true);
             if (distToContour >= 0) inTriggerZone = true;
        }
        
        // --- Triggerbot ---
        if (enabled && (GetAsyncKeyState(VK_MENU) & 0x8000) && target.valid) {
             auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTrigger).count();
             if (elapsed > 150) {
                  if (inTriggerZone) {
                     Sleep(std::uniform_int_distribution<>(trigger.minDelay, trigger.maxDelay)(gen));
                     mouse.click();
                     lastTrigger = now;
                  }
             }
        }

        // --- Aimbot & RCS ---
        bool aimKey = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000);
        bool shouldAim = enabled && (aimKey || aim.always_active);

        if (shouldAim && target.valid) {
             if (aim.humanize) {
                patternTick++;
                if (patternTick > 10) {
                    offsetVariationY = humanDistY(gen) * aim.patternVariation;
                    offsetVariationX = humanDistX(gen) * (aim.patternVariation / 2);
                    patternTick = 0;
                }
            }
            
            int aimX = target.x + offsetVariationX;
            int aimY = target.y + offsetVariationY;
            int xDiff = aimX - centerX;
            int yDiff = aimY - centerY;
            
            if (std::abs(xDiff) > aim.deadzone || std::abs(yDiff) > aim.deadzone) {
                float smoothFactor = 1.0f;
                double dist = std::sqrt(xDiff*xDiff + yDiff*yDiff);
                if (dist < 10.0) smoothFactor = 0.4f;
                else if (dist < 30.0) smoothFactor = 0.7f;
                
                mouse.move(aim.xSpeed * xDiff * smoothFactor, aim.ySpeed * yDiff * smoothFactor);
            }
        }

        if (enabled && aim.rcs && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
             mouse.move(0, 2);
             Sleep(10);
        }
        
        if (config.debugMode) {
             if (config.visuals.draw_fov) {
                 cv::ellipse(frame, cv::Point(centerX, centerY), cv::Size(aim.xFov, aim.yFov), 0, 0, 360, cv::Scalar(0, 255, 0), 1);
             }
             if (target.valid && config.visuals.draw_target) {
                 cv::rectangle(frame, target.rect, cv::Scalar(0, 0, 255), 2);
                 cv::line(frame, cv::Point(centerX, centerY), cv::Point(target.x, target.y), cv::Scalar(255, 0, 0), 1);
             }
             cv::imshow("Debug", frame);
             if (cv::waitKey(1) == 27) break;
        } else {
            Sleep(1);
        }
    }
    
    if (config.debugMode) cv::destroyAllWindows();
    mouse.disconnect();
    return 0;
}
