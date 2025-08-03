/*
	by gon_iss (c) 2024
*/

#pragma once
#include <game_sa/CTimer.h>
#include "Utils/Utils.h"

class ShowFromDownAnimation
{
	unsigned int animationTime;
	float verticalShowingPercent;

public:

	bool isAnimationNow;
	bool directionUp;

	ShowFromDownAnimation() {
		animationTime = CurrentTime();
		isAnimationNow = false;
		directionUp = false;
	}

	float GetValue(int showTime) {
		float timePerc = Utils::getElapsedTimePercentage(showTime, animationTime);

		if (timePerc == 100) isAnimationNow = false;

		if (isAnimationNow) {

			verticalShowingPercent = directionUp ? Utils::normalisePercent(timePerc) : Utils::normalisePercent(100 - timePerc);
		}
		else {
			verticalShowingPercent = directionUp ? 100 : 0;
		}

		return verticalShowingPercent;
	}

	void Activate(bool _directionUp) {

		if (directionUp == _directionUp || isAnimationNow) return;

		animationTime = CurrentTime();
		isAnimationNow = true;
		directionUp = _directionUp;
	}
};