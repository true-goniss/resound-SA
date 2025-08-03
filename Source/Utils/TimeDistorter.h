#pragma once
#include <thread>
#include <game_sa/CTimer.h>
#include "Utils.h"

using u32 = uint32_t;

#define NORMAL_TIME_WARP 1.0f
#define SLOWED_TIME_WARP 0.1f
#define DEFAULT_TIMEWARP_LERP 250.0f

enum class TimeDistortion : int {
    Normal = 1,
    Slowed = 2
};

float m_fLastTimeWarp;

constexpr float m_fLerpTimeMS = 250;

float fTargetWarp;

static class TimeDistorter
{
    static bool WarpTime(float fNewTime)
    {
        // Ensure `fNewTime` is slightly rounded to avoid floating-point precision issues
        constexpr float precisionThreshold = 0.001f;

        if (std::abs(m_fLastTimeWarp - fNewTime) > precisionThreshold)
        {
            m_fLastTimeWarp = fNewTime;
            Utils::SetGameSpeed(fNewTime);  // Apply the new time warp
            return false;
        }

        return true;
    }

public:

	static inline bool timeIsWarped = false;

    static void SetTargetTimeDistort(TimeDistortion whichDir) {
        float fTargetWarp = (whichDir == TimeDistortion::Normal) ? NORMAL_TIME_WARP : SLOWED_TIME_WARP;

        constexpr float threshold = 0.001f;

        if (std::abs(m_fLastTimeWarp - fTargetWarp) > threshold) {
            // Get the actual time elapsed for the frame
            u32 lastFrameTime = CTimer::m_snTimeInMilliseconds - CTimer::m_snPreviousTimeInMilliseconds;

            // Normalize the time step by dividing by the current game speed
            float normalizedTimeStep = float(lastFrameTime) / m_fLastTimeWarp;

            // Calculate the rate of interpolation using the normalized time step
            float fTimeRate = ((NORMAL_TIME_WARP - SLOWED_TIME_WARP) / m_fLerpTimeMS) * normalizedTimeStep;

            float fNewTime = m_fLastTimeWarp;
            if (m_fLastTimeWarp < fTargetWarp) {
                fNewTime += fTimeRate;
                fNewTime = min(fNewTime, fTargetWarp);
            }
            else if (m_fLastTimeWarp > fTargetWarp) {
                fNewTime -= fTimeRate;
                fNewTime = max(fNewTime, fTargetWarp);
            }

            if (WarpTime(fNewTime)) {
                timeIsWarped = true;
            }
        }
        else {
            WarpTime(fTargetWarp);
            timeIsWarped = (fTargetWarp != NORMAL_TIME_WARP);
        }
    }

	static void Reset(TimeDistortion whichDir) {
		float fTargetWarp = (whichDir == TimeDistortion::Normal) ? NORMAL_TIME_WARP : SLOWED_TIME_WARP;

		if (WarpTime(fTargetWarp))
		{
			timeIsWarped = false;
		}
	}
};
