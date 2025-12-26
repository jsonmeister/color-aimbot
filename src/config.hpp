#pragma once
#include "interfaces.hpp"
#include <string>
#include <map>
#include <vector>

namespace colorbot {

struct ProfileSettings {
    float xSpeed = 0.4f;
    float ySpeed = 0.4f;
    int xFov = 100;
    int yFov = 75;
    float targetOffset = 8.0f;
    int deadzone = 2;
    bool humanize = false;
    int patternVariation = 0;
    bool rcs = false;
    bool always_active = false;
};

struct VisualSettings {
    bool enabled = true;
    bool draw_fov = true;
    bool draw_target = true;
};

struct Config {
    ScreenRegion screen;
    ColorRange color;
    
    // Profiles
    std::string activeProfile = "legit";
    std::map<std::string, ProfileSettings> profiles;
    VisualSettings visuals;

    std::string comPort{"COM3"};
    int baudRate{115200};
    bool debugMode{false};
    
    static Config load(const std::string& path);
    void save(const std::string& path) const;
};

}
