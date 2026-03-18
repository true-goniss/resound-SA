#pragma once
/*
    by gon_iss (c) 2026
*/

#include <functional>
#include <atomic>
#include <map>
#include "SoundPlayer.h"

struct AudioTrackTicket {
    SoundPlayer* player = nullptr;
    uint64_t id = 0;

    bool IsValid() const { return player != nullptr && id != 0; }
};

class AudioResourceManager {
public:
    AudioResourceManager() = delete;

    static void Init() { }

    /**
     * @brief Stop
     */
    static void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_activeTickets.clear();
    }

    /**
     * @brief Plays music and hands a ticket
     */
    static AudioTrackTicket EnqueuePlayback(SoundPlayer* player, std::function<void()> playbackTask) {
        if (!player) return {};

        uint64_t newId = ++m_ticketCounter;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_activeTickets[player] = newId;
        }

        if (playbackTask) {
            try {
                playbackTask();
            }
            catch (...) { }
        }

        return { player, newId };
    }

    /**
     * @brief Enqueue task at resource manager
     */
    static void Enqueue(std::function<void()> task) {
        if (task) {
            try {
                task();
            }
            catch (...) { }
        }
    }

    /**
     * @brief Get track position safely
     */
    static int GetTrackPositionMs(const AudioTrackTicket& ticket, int endThresholdMs = 100) {
        if (!ticket.IsValid()) return -1;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_activeTickets.count(ticket.player) == 0 || m_activeTickets[ticket.player] != ticket.id) {
                return -1;
            }
        }

        int pos = ticket.player->getTrackPositionMs();
        int length = ticket.player->getTrackLengthMs();

        if (length > 0 && pos >= (length - endThresholdMs)) {
            return -1;
        }

        return pos;
    }

    /**
     * @brief Always false for now
     */
    static bool IsBusy() {
        return false;
    }

private:
    inline static std::mutex m_mutex;
    inline static std::atomic<uint64_t> m_ticketCounter{ 0 };
    inline static std::map<SoundPlayer*, uint64_t> m_activeTickets;
};