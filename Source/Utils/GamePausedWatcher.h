#pragma once

/*
    gon_iss (c) 2025
*/

#include "CTimer.h"
#include "plugin.h"
#include <vector>
#include <functional>

using namespace plugin;

class GamePausedWatcher {
    static inline bool init = false;
    static inline bool wasPaused = true;

public:

    enum class EventType {
        Paused,
        Unpaused
    };

    using GamePauseStatusChangedHandler = std::function<void(
        GamePausedWatcher::EventType
    )>;

    static inline std::vector<GamePauseStatusChangedHandler> eventHandlers;

    static void AddHandler(const GamePauseStatusChangedHandler& handler) {
        eventHandlers.push_back(handler);
    }

    static void Initialize() {
        if (init) return;
        init = true;

        wasPaused = CTimer::m_CodePause || CTimer::m_UserPause;

        Events::gameProcessEvent += [] {
            bool isPaused = CTimer::m_CodePause || CTimer::m_UserPause;


            if (wasPaused != isPaused) {

                EventType eventType = isPaused ?
                    EventType::Paused :
                    EventType::Unpaused;

                DispatchEvent(eventType);

                wasPaused = isPaused;
            }
        };

        Events::onPauseAllSounds += [] {
            DispatchEvent(EventType::Paused);
        };

        Events::onResumeAllSounds += [] {
            DispatchEvent(EventType::Unpaused);
        };

        Events::drawMenuBackgroundEvent += [] {
            DispatchEvent(EventType::Paused);
        };
    }

    protected:

        static void DispatchEvent(EventType eventType) {

            for (auto& handler : eventHandlers) {
                if (handler) handler(eventType);
            }
        }
};