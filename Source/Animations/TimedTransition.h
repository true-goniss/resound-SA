/*
    by gon_iss (c) 2025
*/
#pragma once
#include <cstdint>
#include <cmath>
#include "CTimer.h"
#include <algorithm>
#include "../Utils/Utils.h"
#include "../Utils/Timer.h"

using namespace std;

//#pragma once
//#include <functional>
//#include "../Utils/Timer.h"
//
//using DefaultTimer = Timer<CurrentTime>;
//
//class TimedTransition {
//protected:
//    DefaultTimer timer;
//    std::function<float(float)> transitionFunc;
//
//    bool isActive = false;
//    int transitionTime;
//    int stableTime = 0;
//    bool isTransitionOut = false;
//
//public:
//
//    explicit TimedTransition(std::function<float(float)> func = [](float x) { return x; }) : transitionFunc(func) {
//        transitionTime = timer.GetTime(); // default value
//    }
//
//    //TimedTransition(int transitionTime, int stableTime, std::function<float(float)> func = [](float x) { return x; })
//    //    : isActive(false), transitionFunc(func), transitionTime(transitionTime), stableTime(stableTime)
//    //{
//
//    //};
//
//    //TimedTransition(std::function<float(float)> func = [](float x) { return x; })
//    //    : isActive(false), transitionFunc(func) {}
//
//    virtual ~TimedTransition() = default;
//
//    virtual void Activate() {
//        isActive = true;
//        timer.Start();
//    }
//
//    virtual void Deactivate() { 
//        isActive = false; 
//        timer.Zero();
//    }
//
//    bool IsActive() const { 
//        return isActive;
//    }
//
//    virtual void Update() = 0; // derived classes define transition behavior
//};



//
//using DefaultTimer = Timer<CurrentTime>;
//
//
//#include <functional>
//
//class TimedTransition {
//protected:
//    DefaultTimer timer;
//    bool isActive;
//    std::function<float(float)> transitionFunc;
//    int transitionTime = 0;
//
//public:
//    TimedTransition(std::function<float(float)> func = [](float x) { return x; })
//        : isActive(false), transitionFunc(func) {
//        
//    }
//
//    virtual ~TimedTransition() = default;
//
//    virtual void Activate() {
//        isActive = true;
//        timer.Start();
//    }
//
//    virtual void Deactivate() { isActive = false; }
//
//    bool IsActive() const { return isActive; }
//
//    virtual void GetValue() {
//        //float timePerc = timer.IsComplete(transitionTime) ? 100.0f : transitionFunc(timer.GetTime() * 100.0f / transitionTime);
//    }
//};
//
//


#include <functional>
#include <cmath>
#include <iostream>



//enum class TransitionType {
//    DownToBasic,  // Transition from basic to min and back to basic
//    UpToBasic,    // Transition from min to basic and back to min
//    Up,           // Transition from min -> max -> min (up to max and back down)
//    Down,         // Transition from max -> min -> max (down to min and back up)
//    Custom        // Custom transition type (add additional types if needed)
//};
//
//class TimedTransition {
//protected:
//    DefaultTimer timer;
//    bool isActive;
//    std::function<float(float)> transitionFunc;
//    int transitionTime = 0;
//    std::vector<float> phaseRatios;  // Multiple phases with dynamic ratios
//    int basicValue, minValue, maxValue;
//
//public:
//    TimedTransition(TransitionType transitionType, int basicValue, int minValue, int maxValue, int transitionTime, std::vector<float> phaseRatios)
//        : isActive(false), basicValue(basicValue), minValue(minValue), maxValue(maxValue), transitionTime(transitionTime), phaseRatios(phaseRatios) {
//        // Set the transition function based on the transition type
//        switch (transitionType) {
//        case TransitionType::DownToBasic:
//            transitionFunc = [this](float time) { return GetDownToBasicValue(time); };
//            break;
//        case TransitionType::UpToBasic:
//            transitionFunc = [this](float time) { return GetUpToBasicValue(time); };
//            break;
//        case TransitionType::Up:
//            transitionFunc = [this](float time) { return GetUpValue(time); };
//            break;
//        case TransitionType::Down:
//            transitionFunc = [this](float time) { return GetDownValue(time); };
//            break;
//        case TransitionType::Custom:
//            transitionFunc = [this](float time) { return GetCustomValue(time); };
//            break;
//        default:
//            transitionFunc = [](float time) { return time; };
//        }
//    }
//
//    virtual ~TimedTransition() = default;
//
//    virtual void Activate() {
//        isActive = true;
//        timer.Start();
//    }
//
//    virtual void Deactivate() {
//        isActive = false;
//    }
//
//    bool IsActive() const { return isActive; }
//
//    // This is where the transition value calculation happens based on the elapsed time
//    float GetValue() {
//        if (!isActive) return basicValue;
//
//        float timePerc = (timer.GetTime() * 100.0f) / transitionTime;
//        timePerc = min(timePerc, 100.0f); // Cap the time percentage to 100%
//
//        return transitionFunc(timePerc);
//    }
//
//private:
//    // Transition from basic -> min -> basic (DownToBasic)
//    float GetDownToBasicValue(float timePerc) {
//        float totalTime = 0;
//        for (size_t i = 0; i < phaseRatios.size(); ++i) {
//            float phaseTime = phaseRatios[i] * transitionTime;
//            totalTime += phaseTime;
//
//            if (timePerc <= totalTime) {
//                // Current phase
//                float phaseStartPerc = (i == 0) ? 0 : phaseRatios[i - 1] * 100.0f;
//                float phaseTimePerc = (timePerc - phaseStartPerc) / (phaseTime * 100.0f);
//                return CalculatePhaseValue(i, phaseTimePerc);
//            }
//        }
//        return basicValue;  // Default fallback
//    }
//
//    // Transition from min -> basic -> min (UpToBasic)
//    float GetUpToBasicValue(float timePerc) {
//        float totalTime = 0;
//        for (size_t i = 0; i < phaseRatios.size(); ++i) {
//            float phaseTime = phaseRatios[i] * transitionTime;
//            totalTime += phaseTime;
//
//            if (timePerc <= totalTime) {
//                // Current phase
//                float phaseStartPerc = (i == 0) ? 0 : phaseRatios[i - 1] * 100.0f;
//                float phaseTimePerc = (timePerc - phaseStartPerc) / (phaseTime * 100.0f);
//                return CalculatePhaseValue(i, phaseTimePerc);
//            }
//        }
//        return basicValue;  // Default fallback
//    }
//
//    // Transition from min -> max -> min (Up)
//    float GetUpValue(float timePerc) {
//        if (timePerc < 50.0f) {
//            // Transitioning up to max
//            return minValue + (maxValue - minValue) * (timePerc / 50.0f);
//        }
//        else {
//            // Transitioning back down to min
//            return maxValue - (maxValue - minValue) * ((timePerc - 50.0f) / 50.0f);
//        }
//    }
//
//    // Transition from max -> min -> max (Down)
//    float GetDownValue(float timePerc) {
//        if (timePerc < 50.0f) {
//            // Transitioning down to min
//            return maxValue - (maxValue - minValue) * (timePerc / 50.0f);
//        }
//        else {
//            // Transitioning back up to max
//            return minValue + (maxValue - minValue) * ((timePerc - 50.0f) / 50.0f);
//        }
//    }
//
//    // Custom transition type (can be extended with custom behavior)
//    float GetCustomValue(float timePerc) {
//        return basicValue;  // Placeholder: implement custom logic as needed
//    }
//
//    // Helper function to calculate the value based on the phase index and time percentage
//    float CalculatePhaseValue(int phaseIndex, float timePerc) {
//        float phaseValue = 0.0f;
//        if (phaseIndex == 0) {
//            // First phase: basic -> min (for DownToBasic) or min -> basic (for UpToBasic)
//            phaseValue = basicValue - (basicValue - minValue) * timePerc;
//        }
//        else if (phaseIndex == 1) {
//            // Second phase: min -> basic (for DownToBasic) or basic -> min (for UpToBasic)
//            phaseValue = minValue + (basicValue - minValue) * timePerc;
//        }
//        else if (phaseIndex == 2) {
//            // Third phase, if any: similar logic
//            phaseValue = basicValue;  // Example; logic for this phase should be adjusted as needed
//        }
//        return phaseValue;
//    }
//};











enum class TransitionType {
    DownToBasic,  // Transition from basic to min and back to basic
    UpToBasic,    // Transition from min to basic and back to min
    Up,           // Transition from min -> max -> min (up to max and back down)
    Down,         // Transition from max -> min -> max (down to min and back up)
    Custom        // Custom transition type (add additional types if needed)
};


class TimedTransition {
protected:
    Timer<CurrentTime> timer;
    bool isActive;
    std::function<float(float)> transitionFunc;
    int transitionTime = 0;
    std::vector<float> phaseRatios;  // Multiple phases with dynamic ratios
    int basicValue, minValue, maxValue;

public:
    TimedTransition(TransitionType transitionType, int basicValue, int minValue, int maxValue, int transitionTime, std::vector<float> phaseRatios)
        : isActive(false), basicValue(basicValue), minValue(minValue), maxValue(maxValue), transitionTime(transitionTime), phaseRatios(phaseRatios) {
        // Set the transition function based on the transition type
        switch (transitionType) {
        case TransitionType::DownToBasic:
            transitionFunc = [this](float time) { return GetDownToBasicValue(time); };
            break;
        case TransitionType::UpToBasic:
            transitionFunc = [this](float time) { return GetUpToBasicValue(time); };
            break;
        case TransitionType::Up:
            transitionFunc = [this](float time) { return GetUpValue(time); };
            break;
        case TransitionType::Down:
            transitionFunc = [this](float time) { return GetDownValue(time); };
            break;
        case TransitionType::Custom:
            transitionFunc = [this](float time) { return GetCustomValue(time); };
            break;
        default:
            transitionFunc = [](float time) { return time; };
        }
    }

    virtual ~TimedTransition() = default;

    virtual void Activate() {
        isActive = true;
        timer.Start();
    }

    virtual void Deactivate() {
        isActive = false;
    }

    bool IsActive() const { return isActive; }

    // This is where the transition value calculation happens based on the elapsed time
    float GetValue() {
        if (!isActive) return basicValue;

        float timePerc = (timer.GetTime() * 100.0f) / transitionTime;
        timePerc = min(timePerc, 100.0f); // Cap the time percentage to 100%

        return transitionFunc(timePerc);
    }

private:
    // Transition from basic -> min -> basic (DownToBasic)
    float GetDownToBasicValue(float timePerc) {
        return basicValue - (basicValue - minValue) * (timePerc / 100.0f);
    }

    // Transition from min -> basic -> min (UpToBasic)
    float GetUpToBasicValue(float timePerc) {
        return minValue + (basicValue - minValue) * (timePerc / 100.0f);
    }

    // Transition from min -> max -> min (Up)
    float GetUpValue(float timePerc) {
        if (timePerc < 50.0f) {
            // Transitioning up to max
            return minValue + (maxValue - minValue) * (timePerc / 50.0f);
        }
        else {
            // Transitioning back down to min
            return maxValue - (maxValue - minValue) * ((timePerc - 50.0f) / 50.0f);
        }
    }

    // Transition from max -> min -> max (Down)
    float GetDownValue(float timePerc) {
        if (timePerc < 50.0f) {
            // Transitioning down to min
            return maxValue - (maxValue - minValue) * (timePerc / 50.0f);
        }
        else {
            // Transitioning back up to max
            return minValue + (maxValue - minValue) * ((timePerc - 50.0f) / 50.0f);
        }
    }

    // Custom transition type (can be extended with custom behavior)
    float GetCustomValue(float timePerc) {
        return basicValue;  // Placeholder: implement custom logic as needed
    }
};