#pragma once

#include "../SoundPlayer.h"
#include <functional>
#include <map>
#include <vector>

class SoundPlayerEventDispatcher {

public:

    using Callback = std::function<void()>;

    void TrackPlayerEvents(SoundPlayer* player) {
        trackers[player] = { player->isPlayingOrActive(), nullptr, nullptr, nullptr };
    }

    void SetOnTrackStarted(SoundPlayer* player, Callback callback) {
        GetEventsTracker(player).onStarted = callback;
    }

    //void SetOnNewTrackPlayed(SoundPlayer* player, std::function<void(std::string)> callback) {
    //    player->setOnNewTrackPlayed(callback);
    //}

    void SetOnTrackPlaying(SoundPlayer* player, Callback callback) {
        GetEventsTracker(player).onPlaying = callback;
    }

    void SetOnTrackEnded(SoundPlayer* player, Callback callback) {
        GetEventsTracker(player).onEnded = callback;
    }

    // Update all players's states
    void Update() {
        for (auto& pair : trackers) {
            UpdatePlayerState(pair.first, pair.second);
        }
    }

private:
    struct Tracker {
        bool wasActive;
        Callback onStarted;
        Callback onPlaying;
        Callback onEnded;
    };

    std::map<SoundPlayer*, Tracker> trackers;

    Tracker& GetEventsTracker(SoundPlayer* player) {

        auto it = trackers.find(player);
        if (it == trackers.end()) {
            TrackPlayerEvents(player);  // register on first call
        }
        return trackers[player];
    }

    void UpdatePlayerState(SoundPlayer* player, Tracker& tracker) {
        const bool isActiveNow = player->isPlayingOrActive();
        const bool isPaused = player->playingStateIsPaused();

        if (isPaused) return;

        if (!tracker.wasActive && isActiveNow) {
            if (tracker.onStarted) tracker.onStarted();
        }
        else if (tracker.wasActive && isActiveNow) {
            if (tracker.onPlaying) tracker.onPlaying();
        }
        else if (tracker.wasActive && !isActiveNow) {
            if (tracker.onEnded) tracker.onEnded();
        }

        tracker.wasActive = isActiveNow;
    }
};