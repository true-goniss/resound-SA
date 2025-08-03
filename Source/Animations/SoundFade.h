/*
    by gon_iss (c) 2024
*/

#pragma once
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
    bool isFadeInNow;
    bool isFadeOutNow;
    float volume;
    bool curveDirection = true;
    bool indefinite = false;
    bool paused = false;
    bool wasUnpaused = false;
    float showingTime = 0;
    float fadeTime = 0;

public:

    bool isActive;

    SoundFade(bool curveDirection, bool indefinite = false) {
        this->curveDirection = curveDirection;
        isFadeInNow = false;
        isFadeOutNow = false;
        activationTime = CurrentTime();
        isActive = false;
        this->indefinite = indefinite;
        volume = 0.0f;
    }

    float GetValue(float curveVolume, float basicVolume, int fadeTime, int showingTime) {

        paused = (!isFadeInNow && !isFadeOutNow && indefinite && !wasUnpaused);

        if (paused) {
            return volume;
        }

        if (!isActive) {
            return curveDirection ? basicVolume : curveVolume;
        }

        //if (curveDirection) {
        //    if (!isActive || paused) return basicVolume;
        //}
        //else {
        //    if (!isActive || paused) return curveVolume;
        //}


        float timePerc = Utils::getElapsedTimePercentage(fadeTime, activationTime);

        if (isFadeOutNow) {

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

            return volume;
        }


        if (curveDirection) {
            volume = basicVolume - ((basicVolume - curveVolume) * ((timePerc / 100.0f)));
        }
        else {
            volume = basicVolume + ((curveVolume - basicVolume) * (1.0f - (timePerc / 100.0f)));
        }

        

        //if (!isFadeInNow && indefinite && !wasUnpaused) paused = true;

        this->showingTime = showingTime;
        this->fadeTime = fadeTime;

        if (timePerc > 99 && Utils::getElapsedTimeMs(activationTime) > (fadeTime + showingTime)) {
            activationTime = CurrentTime();
            isFadeOutNow = true;
        }
        else {
            isFadeInNow = Utils::getElapsedTimeMs(activationTime) < (fadeTime);
        }

        return volume;
    }

    void Deactivate() {
        isFadeOutNow = true;
        paused = false;
        wasUnpaused = true;
        activationTime = CurrentTime();
    }

    void Activate() {
        isActive = true;
        activationTime = CurrentTime();
        isFadeInNow = true;
        isFadeOutNow = false;
        wasUnpaused = false;
    }
};