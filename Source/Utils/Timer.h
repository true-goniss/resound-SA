#pragma once
#include <iostream>
#include <chrono>
#include <thread>

using u32 = uint32_t;
using s32 = int32_t;

typedef u32(*TimeFunc)();

u32 CurrentTime() {
    return static_cast<u32>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
}

u32 CurrentGameTimeNonClipped() {
    return static_cast<uint64_t>(
        CTimer::m_snTimeInMillisecondsNonClipped
    );
}

template<TimeFunc T>
class Timer {
    u32 curTimeStamp = 0;
    static u32 GetTimeFunc() { return T(); }

public:
    Timer() { Zero(); }
    void Zero() { curTimeStamp = 0; }
    void Start(s32 timeOffset = 0) {
        curTimeStamp = GetTimeFunc() + timeOffset;
    }
    bool IsComplete(s32 targetTime, bool bMustBeStarted = true) const {
        if (bMustBeStarted && !IsStarted()) return false;
        return GetTimeFunc() >= (curTimeStamp + targetTime);
    }
    bool IsStarted() const { return curTimeStamp != 0; }
    u32 GetTime() const { return curTimeStamp; }
};