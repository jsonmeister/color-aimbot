#include "config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

namespace colorbot {

namespace {
    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    std::string extractString(const std::string& json, const std::string& key) {
        std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]+)\"");
        std::smatch match;
        if (std::regex_search(json, match, pattern)) return match[1];
        return "";
    }
    
    int extractInt(const std::string& json, const std::string& key, int defaultVal) {
        std::regex pattern("\"" + key + "\"\\s*:\\s*(-?\\d+)");
        std::smatch match;
        if (std::regex_search(json, match, pattern)) return std::stoi(match[1]);
        return defaultVal;
    }
    
    float extractFloat(const std::string& json, const std::string& key, float defaultVal) {
        std::regex pattern("\"" + key + "\"\\s*:\\s*([\\d.]+)");
        std::smatch match;
        if (std::regex_search(json, match, pattern)) return std::stof(match[1]);
        return defaultVal;
    }
    
    std::vector<int> extractIntArray(const std::string& json, const std::string& key) {
        std::regex pattern("\"" + key + "\"\\s*:\\s*\\[([^\\]]+)\\]");
        std::smatch match;
        std::vector<int> result;
        if (std::regex_search(json, match, pattern)) {
            std::regex numPattern("\\d+");
            std::string arr = match[1];
            auto begin = std::sregex_iterator(arr.begin(), arr.end(), numPattern);
            auto end = std::sregex_iterator();
            for (auto i = begin; i != end; ++i) result.push_back(std::stoi((*i).str()));
        }
        return result;
    }
}

Config Config::load(const std::string& path) {
    Config cfg;
    std::string json = readFile(path);
    if (json.empty()) return cfg;
    
    cfg.screen.width = extractInt(json, "captureWidth", 250);
    cfg.screen.height = extractInt(json, "captureHeight", 250);
    cfg.screen.offsetX = extractInt(json, "offsetX", 0);
    cfg.screen.offsetY = extractInt(json, "offsetY", -10);
    
    auto lower = extractIntArray(json, "hsvLower");
    if (lower.size() >= 3) cfg.color.lower = cv::Scalar(lower[0], lower[1], lower[2]);
    
    auto upper = extractIntArray(json, "hsvUpper");
    if (upper.size() >= 3) cfg.color.upper = cv::Scalar(upper[0], upper[1], upper[2]);
    
    cfg.activeProfile = extractString(json, "profile");
    if (cfg.activeProfile.empty()) cfg.activeProfile = "legit";

    auto parseProfile = [&](const std::string& name) -> ProfileSettings {
        ProfileSettings p;
        // Simple manual extraction from nested object (hacky but works for flat json)
        // Ideally use a real JSON parser, but sticking to regex helper
        // We find the block for the profile first
        std::string section = "\"" + name + "\"";
        size_t pos = json.find(section);
        if (pos == std::string::npos) return p;
        size_t start = json.find("{", pos);
        size_t end = json.find("}", start);
        if (start == std::string::npos || end == std::string::npos) return p;
        std::string block = json.substr(start, end - start + 1);

        p.xSpeed = extractFloat(block, "xSpeed", 0.15f);
        p.ySpeed = extractFloat(block, "ySpeed", 0.15f);
        p.xFov = extractInt(block, "xFov", 60);
        p.yFov = extractInt(block, "yFov", 45);
        p.targetOffset = extractFloat(block, "targetOffset", 8.0f);
        p.deadzone = extractInt(block, "deadzone", 2);
        
        std::regex boolPattern("\"humanize\"\\s*:\\s*true");
        p.humanize = std::regex_search(block, boolPattern);
        p.patternVariation = extractInt(block, "patternVariation", 0);

        std::regex rcsPattern("\"rcs\"\\s*:\\s*true");
        p.rcs = std::regex_search(block, rcsPattern);

        std::regex activePattern("\"always_active\"\\s*:\\s*true");
        p.always_active = std::regex_search(block, activePattern);

        return p;
    };

    cfg.profiles["legit"] = parseProfile("legit");
    cfg.profiles["semi"] = parseProfile("semi");
    cfg.profiles["rage"] = parseProfile("rage");

    // Visuals
    std::string vSection = "\"visuals\"";
    size_t vPos = json.find(vSection);
    if (vPos != std::string::npos) {
        size_t start = json.find("{", vPos);
        size_t end = json.find("}", start);
        std::string block = json.substr(start, end - start + 1);
        
        std::regex enabledP("\"enabled\"\\s*:\\s*true");
        cfg.visuals.enabled = std::regex_search(block, enabledP);
        std::regex fovP("\"draw_fov\"\\s*:\\s*true");
        cfg.visuals.draw_fov = std::regex_search(block, fovP);
        std::regex targetP("\"draw_target\"\\s*:\\s*true");
        cfg.visuals.draw_target = std::regex_search(block, targetP);
    }
    
    std::string port = extractString(json, "comPort");
    if (!port.empty()) cfg.comPort = port;
    cfg.baudRate = extractInt(json, "baudRate", 115200);
    
    std::regex debugPattern("\"debugMode\"\\s*:\\s*(true|false)");
    std::smatch debugMatch;
    if (std::regex_search(json, debugMatch, debugPattern))
        cfg.debugMode = (debugMatch[1] == "true");
    
    return cfg;
}

void Config::save(const std::string& path) const {
    // Basic save not implemented fully as C++ usually reads. 
    // UI handles the writing.
}
}
}
