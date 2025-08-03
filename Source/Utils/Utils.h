#pragma once
#include <game_sa/CTimer.h>
#include <game_sa/CCamera.h>
#include "CClock.h"
#include "Timer.h"

#include <filesystem>
#include <random>
#include <map>

namespace fs = std::filesystem;

static class Utils
{
    /* -----------------------------  text --------------------------------- */


    static inline bool ends_with(const std::string str, const std::string& suffix)
    {
        auto suf_sz = suffix.size();
        auto str_sz = str.size();
        return (suf_sz <= str_sz) && (str.substr(str_sz - suf_sz) == suffix);
    }

    static std::string RemoveSubstring(const std::string& input, const std::string& substringToRemove) {
        std::string result = input;
        size_t index = result.find(substringToRemove);

        if (index != std::string::npos) {

            result.erase(index, substringToRemove.length());
        }

        return result;
    }

    static std::string toLower(const std::string& str) {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
        return lowerStr;
    }

    public:

    static bool ContainsCaseInsensitive(const std::string& str, const std::string& substr) {
        std::string lowerStr = str;
        std::string lowerSubstr = substr;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
        std::transform(lowerSubstr.begin(), lowerSubstr.end(), lowerSubstr.begin(), ::tolower);
        return lowerStr.find(lowerSubstr) != std::string::npos;
    }

    static std::string remove_music_extension(const std::string& input) {
        std::string res = input;
        res = Utils::RemoveSubstring(res, ".mp3");
        res = Utils::RemoveSubstring(res, ".wav");
        res = Utils::RemoveSubstring(res, ".ogg");

        return res;
    }

    static std::pair<std::string, std::string> GetTrackArtistAndName(const std::string& input) {

        size_t index = input.find_last_of("-−–——");

        if (index != std::string::npos) {

            std::string firstPart = input.substr(0, index);
            std::string secondPart = input.substr(index + 1);

            return std::make_pair(firstPart, secondPart);
        }
        else {

            return std::make_pair(input, std::string());
        }
    }

    /*  -----------------------------  math --------------------------------- */

    static float getElapsedTimePercentage(float amountOfTimeMs, float initialTime) {
        int currentTime = CurrentTime();
        int elapsedTime = currentTime - initialTime;

        float percentage = static_cast<float>(elapsedTime) / static_cast<float>(amountOfTimeMs) * 100.0f;

        return normalisePercent(percentage);
    }

    static float getElapsedTimerTimePercentage(Timer<CurrentTime> tmr, float amountOfTimeMs, float initialTime) {
        int currentTime = tmr.GetTime();
        int elapsedTime = currentTime - initialTime;

        float percentage = static_cast<float>(elapsedTime) / static_cast<float>(amountOfTimeMs) * 100.0f;

        return normalisePercent(percentage);
    }

    static int getElapsedTimeMs(float initialTime) {
        int currentTime = CurrentTime();
        return currentTime - initialTime;
    }

    static bool timePassedEnough(unsigned int lastKeyPressedTime, unsigned int timeThreshold) {
        return CurrentTime() > (lastKeyPressedTime + timeThreshold);
    }

    static int timeToMilliseconds(const std::string& time) {
        int minutes, seconds;
        char colon;
        std::stringstream ss(time);
        ss >> minutes >> colon >> seconds;
        return minutes * 60 * 1000 + seconds * 1000;
    }

    static float normalisePercent(float percentage) {
        return (percentage < 0.0f) ? 0.0f : ((percentage > 100.0f) ? 100.0f : percentage);
    }

    // linear interpolation
    static float Lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    static float Selectf(float condition, float a, float b) {
        return (condition >= 0) ? Lerp(a, b, condition) : Lerp(b, a, -condition);
    }

    // map a value from one range to another
    static float MapRange(float value, float inMin, float inMax, float outMin, float outMax) {
        return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    }

    static int getRandomInt(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(min, max);
        return dis(gen);
    }

    static int getRandomIntWithDifferentProbabilities(std::vector<double> weights) {

        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<int> dis(weights.begin(), weights.end());

        return dis(gen) + 1; // + 1 to shift the range from 0-4 to 1-5
    }

    static bool getRandomBoolWithProbability(float trueProbability) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::bernoulli_distribution dist(trueProbability);

        return dist(gen);
    }

    static float getDistance(float x1, float y1, float x2, float y2) {
        return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    }

    /*  -------------------------  filesystem ------------------------------ */

    struct SingleFileTrackInfo {
        std::string artist;
        std::string title;
        int startTimeMs;
        int endTimeMs;
    };

    static std::vector<SingleFileTrackInfo> ReadSingleFileTracksInfo(const std::string& path, const std::string& fileRelativePath) {

        std::vector<SingleFileTrackInfo> singleFileTracks;

        std::ifstream file(path + "\\" + fileRelativePath);

        if (!file.is_open()) return singleFileTracks;

        std::string line;
        std::string startTimeStr, endTimeStr, artist, title;
        bool isTimeRange = false;

        while (std::getline(file, line)) {
            if (!isTimeRange) {
                size_t colonCount = std::count(line.begin(), line.end(), ':');
                size_t hyphenCount = std::count(line.begin(), line.end(), '-');
                if (colonCount == 2 && hyphenCount == 1) {

                    size_t hyphenPos = line.find('-');
                    startTimeStr = line.substr(0, hyphenPos);
                    endTimeStr = line.substr(hyphenPos + 1);
                    isTimeRange = true;

                }
            }
            else {

                std::pair artist_track = Utils::GetTrackArtistAndName(line);

                SingleFileTrackInfo info;
                info.startTimeMs = Utils::timeToMilliseconds(startTimeStr);
                info.endTimeMs = Utils::timeToMilliseconds(endTimeStr);
                info.artist = artist_track.first;
                info.title = artist_track.second;

                singleFileTracks.push_back(info);

                isTimeRange = false;
            }
        }

        file.close();

        return singleFileTracks;
    }

    static int GetPartNumberFromFilename(const std::string& filename) {
        std::string lowerFilename = toLower(filename);

        size_t partPos = lowerFilename.find("part_");
        size_t dotPos = lowerFilename.find(".");

        if (partPos != std::string::npos && dotPos != std::string::npos) {
            partPos += 5; // move the position to after "part_"
            std::string numberStr = lowerFilename.substr(partPos, dotPos - partPos);
            try {
                return std::stoi(numberStr);
            }
            catch (const std::invalid_argument& e) {
            }
        }
        return -1;
    }

    static std::map<int, std::string> GetFolderPartFilesMap(const std::string& folderPath) {

        std::map<int, std::string> filesMap;

        for (const auto& entry : fs::directory_iterator(folderPath)) {
            std::string filename = entry.path().filename().string();

            if (Utils::ContainsCaseInsensitive(filename, ".txt")) continue;

            int number = GetPartNumberFromFilename(filename);
            if (number != -1) {
                filesMap[number] = filename;
            }
        }

        return filesMap;
    }


    static std::string ensureTrailingSlash(const std::string& path) {
        if (!path.empty() && (path.back() == '\\' || path.back() == '/')) {
            return path;
        }
        else {
            return path + '\\';
        }
    }

    static bool FileCheck(const char* name) {
        struct stat buffer;
        return (stat(name, &buffer) == 0);
    }

    static std::string GetCurrentDirectory()
    {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        std::string::size_type pos = std::string(buffer).find_last_of("\\/");

        return std::string(buffer).substr(0, pos);
    }

    static bool DirectoryCheckRelative(std::string path) {
        path = ensureTrailingSlash(GetCurrentDirectory()) + ensureTrailingSlash(path);
        
        return fs::is_directory(path);
    }

    static std::string getRandomFileFromPath(const std::string& path) {
        std::string selected_file = "";

        int n = 0;
        WIN32_FIND_DATAA fdata;

        HANDLE hf = FindFirstFileA(path.c_str(), &fdata);
        if (hf != INVALID_HANDLE_VALUE)
        {
            do
            {
                static std::mt19937 rng(std::time(nullptr));
                if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    (std::uniform_int_distribution<>(1, ++n)(rng) == 1))
                    selected_file = fdata.cFileName; // replace with probability 1/n
            }
            while (FindNextFile(hf, &fdata));

            FindClose(hf);
        }

        return selected_file;
    }

    static std::string pick_random_music_file(const std::string& path)
    {
        std::string path1 = path;
        std::string path2 = path;
        std::string path3 = path;

        static std::string filter = "*.mp3";
        if (!ends_with(path1, filter))
        {
            static const std::string seperator = "\\";
            if (!ends_with(path1, seperator)) path1 += seperator;
            path1 += filter;
        }

        static std::string filter2 = "*.wav";
        if (!ends_with(path2, filter2))
        {
            static const std::string seperator = "\\";
            if (!ends_with(path2, seperator)) path2 += seperator;
            path2 += filter2;
        }

        static std::string filter3 = "*.ogg";
        if (!ends_with(path3, filter3))
        {
            static const std::string seperator = "\\";
            if (!ends_with(path3, seperator)) path3 += seperator;
            path3 += filter3;
        }

        std::string selected_file = "";

        selected_file = getRandomFileFromPath(path1);

        if (selected_file == "") selected_file = getRandomFileFromPath(path2);

        if (selected_file == "") selected_file = getRandomFileFromPath(path3);

        return selected_file;
    }

    static bool directoryExists(const std::string& directory) {
        return fs::exists(directory) && fs::is_directory(directory);
    }

    static std::string tryFindRandomFolderName(const std::string& directoryPath) {
        std::vector<std::string> folderNames;

        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_directory()) {
                folderNames.push_back(entry.path().filename().string());
            }
        }

        if (folderNames.empty()) {
            return "";
        }

        int randomIndex = getRandomInt(0, folderNames.size() - 1);
        return folderNames[randomIndex];
    }

    static std::string tryFindRandomFileWithContainedName(const std::string& filename, const std::string& folderPath) {

        std::vector<std::string> matchingFiles;

        std::string filenameWithoutExtension = "";

        size_t dotPos = filename.find_last_of('.');
        if (dotPos == std::string::npos) {
            filenameWithoutExtension = filename;
        }
        else {
            filenameWithoutExtension = filename.substr(0, dotPos);
        }

        if (!directoryExists(folderPath)) return "";

        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::string currentFilename = entry.path().filename().string();

                std::string currentFilenameWithoutExtension = currentFilename.substr(0, currentFilename.find_last_of('.'));
                if (ContainsCaseInsensitive(currentFilenameWithoutExtension, filenameWithoutExtension)) {
                    matchingFiles.push_back(currentFilename);
                }
            }
        }

        if (!matchingFiles.empty()) {
            return matchingFiles[getRandomInt(0, matchingFiles.size() - 1)];
        }

        return ""; // File not found
    }

    /* -------------- misc -------------- */

    static bool isRadioStationAGTAIIIVCType(const std::string& path) {
        return FileCheck((Utils::GetCurrentDirectory() + "\\resound\\radio\\" + path + "\\singleFileTracksInfo.txt").c_str());
    }

    static bool isRadioStationA_V_Talk_Type(const std::string& path) {

    }

    static int GameHours() {
        return static_cast<int>(CClock::ms_nGameClockHours);
    }

    static int GameDays() {
        return static_cast<int>(CClock::ms_nGameClockDays);
    }

    static void MessageBoxShow(const std::string& message) {
        std::wstring wideStr = ConvertToWideString(message);
        MessageBoxW(nullptr, wideStr.c_str(), L"Error", MB_ICONERROR);
    }

    // Convert narrow string to wide string
    static std::wstring ConvertToWideString(const std::string& narrowStr) {
        int bufferSize = MultiByteToWideChar(CP_UTF8, 0, narrowStr.c_str(), -1, nullptr, 0);
        std::wstring wideStr(bufferSize, L'\0'); // Use wide character type
        MultiByteToWideChar(CP_UTF8, 0, narrowStr.c_str(), -1, &wideStr[0], bufferSize);
        return wideStr;
    }

    static inline std::string getCurrentTimeString() {
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    };

    /* ----------  game ----------- */

    static inline void SetGameSpeed(float speedMultiplier) {
        if (speedMultiplier > 0.0f) {
            CTimer::ms_fTimeScale = speedMultiplier;
        }
    }

    static inline float nPrevCamHor = 0.0f;
    static inline float nPrevCamVer = 0.0f;

    static inline void ResetCameraMovement() {
        TheCamera.m_fMouseAccelHorzntl = nPrevCamHor;
        TheCamera.m_fMouseAccelVertical = nPrevCamVer;
        nPrevCamHor = 0.0f;
        nPrevCamVer = 0.0f;
    }

    static inline void DisableCameraMovement() {
        nPrevCamHor = TheCamera.m_fMouseAccelHorzntl;
        nPrevCamVer = TheCamera.m_fMouseAccelVertical;

        TheCamera.m_fMouseAccelHorzntl = 0.0f;
        TheCamera.m_fMouseAccelVertical = 0.0f;
    }
};