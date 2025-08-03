/*
	by gon_iss (c) 2024
*/

#pragma once
#include "../Utils/Utils.h"

class OpacityAnimation
{
	unsigned int animationTime;
	bool animationFadeOut;
	int opacity;

public:

	bool isAnimationNow;

	OpacityAnimation() {
		animationFadeOut = false;
		animationTime = CurrentTime();
		isAnimationNow = false;
		opacity = 0;
	}

	int GetValue(int maxOpacity, int fadeTime, int showingTime) {
		if (!isAnimationNow) return 0;

		float timePerc = 0;

		if (animationFadeOut) {
			timePerc = Utils::getElapsedTimePercentage(fadeTime, animationTime);
			opacity = (Utils::normalisePercent(100 - timePerc) / 100) * maxOpacity;

			if (timePerc > 99) {
				animationFadeOut = false;
				isAnimationNow = false;
			}
		}
		else {
			timePerc = Utils::getElapsedTimePercentage(fadeTime, animationTime);
			opacity = (Utils::normalisePercent(timePerc) / 100) * maxOpacity;

			if (timePerc > 99 && Utils::getElapsedTimeMs(animationTime) > (fadeTime + showingTime)) {
				animationTime = CurrentTime();
				animationFadeOut = true;
			}
		}

		return opacity;
	}

	void Activate() {
		animationTime = CurrentTime();
		animationFadeOut = false;
		isAnimationNow = true;
	}
};