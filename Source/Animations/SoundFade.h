/*
    by gon_iss (c) 2024
*/

#pragma once
#include <game_sa/CTimer.h>
#include "Utils/Utils.h"


/*

curveDirection:

false:
      _______		---- maxVolume (curveVolume)
     /       \
____/		  \____ ---- basicVolume


true:
____		   ____ ---- basicVolume
    \		  /
     \_______/		---- minVolume (curveVolume)


*/


class SoundFade
{
    unsigned int activationTime;
    bool isFadeOutNow;
    float volume;
    bool curveDirection = true;

public:

    bool isActive;

    SoundFade(bool curveDirection) {
        this->curveDirection = curveDirection;
        isFadeOutNow = false;
        activationTime = CTimer::m_snTimeInMilliseconds;
        isActive = false;
        volume = 0.0f;
    }

    float GetValue(float curveVolume, float basicVolume, int fadeTime, int showingTime) {

        if (curveDirection) {
            if (!isActive) return basicVolume;
        }
        else {
            if (!isActive) return curveVolume;
        }

        float timePerc = 0;

        if (isFadeOutNow) {

            timePerc = Utils::getElapsedTimePercentage(fadeTime, activationTime);

            if (curveDirection) {
                volume = basicVolume - ((basicVolume - curveVolume) * (1.0f - (timePerc / 100.0f)));
            }
            else {
                volume = basicVolume + ((curveVolume - basicVolume) * timePerc / 100.0f);
            }

            if (timePerc > 99) {
                isFadeOutNow = false;
                isActive = false;
            }
        }
        else {

            timePerc = Utils::getElapsedTimePercentage(fadeTime, activationTime);

            if (curveDirection) {
                volume = basicVolume - ((basicVolume - curveVolume) * ((timePerc / 100.0f)));
            }
            else {
                volume = basicVolume + ((curveVolume - basicVolume) * (1.0f - (timePerc / 100.0f)));
            }

            if (timePerc > 99 && Utils::getElapsedTimeMs(activationTime) > (fadeTime + showingTime)) {
                activationTime = CTimer::m_snTimeInMilliseconds;
                isFadeOutNow = true;
            }
        }

        return volume;
    }

    void Activate() {
        isActive = true;
        activationTime = CTimer::m_snTimeInMilliseconds;
        isFadeOutNow = false;
    }
};