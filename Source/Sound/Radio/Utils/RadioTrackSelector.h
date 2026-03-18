#pragma once
#include <string>
#include <vector>
#include "../../../Utils/Utils.h"

class Radio_V_TrackSelector {
public:
    // Finds intro specific to current track (e.g., DJ mentioning artist name)
    static std::string GetIntroForTrack(const std::string& trackName, const std::string& introPath, const std::string& generalPath) {
        std::string specific = Utils::tryFindRandomFileWithContainedName(trackName, introPath);
        if (!specific.empty()) return specific;

        return Utils::pick_random_music_file(generalPath);
    }

    // Finds outro for transition to next block (News, Ad, or track end)
    static std::string GetOutroForNextBlock(const std::string& keyword, const std::string& toPath, const std::string& generalPath) {
        if (!keyword.empty()) {
            std::string specific = Utils::tryFindRandomFileWithContainedName(keyword, toPath);
            if (!specific.empty()) return specific;
        }
        return Utils::pick_random_music_file(generalPath);
    }

    // Returns time announcement file based on game hours
    static std::string GetTimeAnnouncement(const std::string& timePath) {
        std::string keyword = "";
        int hour = Utils::GameHours();

        if (hour >= 19 && hour <= 23) keyword = "EVENING";
        else if (hour >= 5 && hour <= 9) keyword = "MORNING";

        if (!keyword.empty()) {
            return Utils::tryFindRandomFileWithContainedName(keyword, timePath);
        }
        return "";
    }

    // Checks if news should play after certain number of tracks
    static bool ShouldPlayNews(int tracksPlayed, int threshold) {
        return tracksPlayed >= threshold;
    }
};