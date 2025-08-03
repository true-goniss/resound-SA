#pragma once

/*
    gon_iss (c) 2025
*/

#include <cmath>
#include "RadioWheelParams.h" // M_PI

using namespace std;

struct WheelGeometry {
    float radius;
    float iconSize;
};

class WheelGeometryCalculator {
public:

    static WheelGeometry Calculate(
        int itemCount,
        float screenWidth,
        float screenHeight,
        float minIconSize = 50.0f,
        float maxIconSize = 100.0f
    ) {
        WheelGeometry geometry;
        const float minScreen = min(screenWidth, screenHeight);

        // Basic params
        const float minRadius = minScreen * 0.225f;   // 20-25% of the screen
        const float maxRadius = minScreen * 0.45f;   // 45% of the screen
        float targetRadius = minRadius;

        // spacing factor (compact up to 20 radiostations)
        const float spacingFactor = (itemCount <= 20) ? 1.05f : 1.15f;

        // Saving radius and icons size up to 20 stations
        if (itemCount <= 20) {
            targetRadius = minRadius;
            geometry.iconSize = maxIconSize;
        }
        // Increasing the radius and decreasing the icons after 20 stations
        else if (itemCount <= 35) {
            float t = (itemCount - 20.0f) / 15.0f; // Normalise
            targetRadius = minRadius + t * (maxRadius - minRadius);
            geometry.iconSize = maxIconSize * max(0.7f, 1.0f - t * 0.3f);
        }
        // Extreme values mode (35+)
        else {
            targetRadius = maxRadius;
            geometry.iconSize = max(minIconSize, maxIconSize * 0.65f);
        }

        // Dynamic correction for the screen limits
        const float circumference = 2 * M_PI * targetRadius;
        const float requiredSpace = itemCount * geometry.iconSize * spacingFactor;

        if (requiredSpace > circumference) {
            // Radius correction when overflow
            geometry.radius = requiredSpace / (2 * M_PI);
            geometry.radius = min(geometry.radius, minScreen * 0.5f); // Max 50% of the screen

            // Decreasing the icons even more if the radius is at the max
            if (geometry.radius >= minScreen * 0.5f) {
                const float maxPossibleSize = (2 * M_PI * geometry.radius)
                    / (itemCount * spacingFactor);
                geometry.iconSize = max(minIconSize, maxPossibleSize * 0.9f);
            }
        }
        else {
            geometry.radius = targetRadius;
        }

        return geometry;
    }
};