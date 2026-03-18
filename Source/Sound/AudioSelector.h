#pragma once

#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <random>
#include "../Utils/Utils.h"

/**
 * @brief Handles track selection
 */
class AudioSelector {
public:
    /**
     * @param historyLimit An amount of tracks to never repeat
     */
    AudioSelector(size_t historyLimit = 10) : m_historyLimit(historyLimit) {}

    /**
     * @brief Choose the next track with history check.
     * @param folder Folder path.
     * @return Relative file path
     */
    std::string SelectNextTrack(const std::string& folder) {
        if (folder.empty()) return "";

        std::string selectedFile = "";
        int attempts = 0;
        const int maxAttempts = 15;

        while (attempts < maxAttempts) {
            selectedFile = Utils::pick_random_music_file(folder);
            if (selectedFile.empty()) break;

            // choose if it's never been played recently
            if (!IsRecentlyPlayed(selectedFile)) {
                break;
            }
            attempts++;
        }

        if (!selectedFile.empty()) {
            AddToHistory(selectedFile);
        }

        return (folder + "\\" + selectedFile);
    }

    /**
     * @brief Find a file with a keyword
     */
    std::string SelectByKeyword(const std::string& folder, const std::string& keyword) {
        if (folder.empty()) return "";
        if (keyword.empty()) return SelectNextTrack(folder);

        std::string foundFile = Utils::tryFindRandomFileWithContainedName(keyword, folder);

        return (folder + "\\" + foundFile);
    }

    /**
     * @brief Choose randomly regardless of the history
     */
    std::string SelectRandom(const std::string& folder) {
        if (folder.empty()) return "";
        return (folder + "\\" + Utils::pick_random_music_file(folder));
    }

    /**
     * @brief Clear history.
     */
    void Reset() {
        m_history.clear();
    }

private:
    bool IsRecentlyPlayed(const std::string& filename) const {
        if (m_history.empty()) return false;
        return std::find(m_history.begin(), m_history.end(), filename) != m_history.end();
    }

    void AddToHistory(const std::string& filename) {
        m_history.push_back(filename);
        if (m_history.size() > m_historyLimit) {
            m_history.pop_front();
        }
    }

    std::deque<std::string> m_history;
    size_t m_historyLimit;
};