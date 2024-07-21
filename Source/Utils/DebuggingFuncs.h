#pragma once
#include <map>
#include <mutex>
#include <extensions/Screen.h>
#include <game_sa/CFont.h>
#include <fstream>
#include <string>

class DebuggingFuncs
{
    static inline bool showDebuggingText = false;

    static struct DebugLabelInfo {
        float verticalOffset;

        std::chrono::steady_clock::time_point lastPrintTime;

        DebugLabelInfo(float offset) : verticalOffset(offset) {}
        DebugLabelInfo() : verticalOffset(0), lastPrintTime(std::chrono::steady_clock::now()) {}
    };

    static inline std::map<const char*, DebugLabelInfo> labelInfoMap;

public:

    static void logMessage(const std::string& message) {
        std::ofstream outputFile("resound_log.txt", std::ios::app);
        if (outputFile.is_open()) {
            outputFile << message << std::endl;
            outputFile.close();
        }
    }

    static void PrintDebugText(const char* label, const char* text) {
        if (!showDebuggingText) return;

        auto it = labelInfoMap.find(label);

        if (it == labelInfoMap.end()) {
            labelInfoMap[label] = DebugLabelInfo(SCREEN_COORD(60.0f) * labelInfoMap.size());
            it = labelInfoMap.find(label);
        }

        auto currentTime = std::chrono::steady_clock::now();
        auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - it->second.lastPrintTime).count();

        if (timeElapsed < 300) {
            return;
        }

        char textResult[128];
        sprintf(textResult, "%s: %s", label, text);
        CFont::SetBackgroundColor(CRGBA(0, 0, 0, 128));
        CFont::SetColor(CRGBA(255, 255, 255, 255));
        CFont::SetScale(SCREEN_MULTIPLIER(0.5f), SCREEN_MULTIPLIER(1.0f));
        CFont::SetEdge(1);
        CFont::SetJustify(true);
        CFont::SetFontStyle(FONT_MENU);
        CFont::PrintString(SCREEN_COORD(200.0f), it->second.verticalOffset, textResult);
    }

    static void PrintDebugText(const char* label, float value) {
        PrintDebugText(label, (std::to_string(value)).c_str());
    }

    static const char* GetCurrentTimeAsConstChars() {
        const int bufferSize = 20;
        char buffer[bufferSize];
        std::time_t currentTime = std::time(nullptr);
        std::strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", std::localtime(&currentTime));

        return buffer;
    }
};
