#pragma once
#include <plugin.h>
#include <CPad.h>
#include "CMenuManager.h"

static class Mouse
{

    // og by _AG
    static CVector2D GetMouseInput(float mult) {
        float x = CPad::GetPad(0)->NewMouseControllerState.X / (180.0f);
        float y = CPad::GetPad(0)->NewMouseControllerState.Y / (180.0f);

        // Little hack
        x = std::clamp(x, -1.0f, 1.0f);
        y = std::clamp(y, -1.0f, 1.0f);

        int sx = 1;
        int sy = 1;

        if (FrontEndMenuManager.bInvertMouseX)
            sx = -1;

        if (FrontEndMenuManager.bInvertMouseY)
            sy = -1;

        x *= mult;
        y *= mult;

        return CVector2D(x * sx, y * sy);
    }

public:


    static inline CVector2D currentMousePosition = CVector2D(RsGlobal.maximumWidth / 2, RsGlobal.maximumHeight / 2); // initial pos at screen center
    static inline CVector2D centeredMousePosition = CVector2D(RsGlobal.maximumWidth / 2, RsGlobal.maximumHeight / 2); // initial pos at screen center

    static void UpdateMousePosition(float mult = 0.1f) {
        CVector2D relativeMouseInput = GetMouseInput(mult);

        float screenWidth = RsGlobal.maximumWidth * 0.99f;
        float screenHeight = RsGlobal.maximumHeight * 0.99f;

        currentMousePosition.x += relativeMouseInput.x * screenWidth;  // Scale by screen width
        currentMousePosition.y += relativeMouseInput.y * screenHeight; // Scale by screen height

        // Clamp to screen bounds
        currentMousePosition.x = std::clamp(currentMousePosition.x, 0.0f, static_cast<float>(screenWidth));
        currentMousePosition.y = std::clamp(currentMousePosition.y, 0.0f, static_cast<float>(screenHeight));
    }

    static void CenterMousePosition() {
        centeredMousePosition = CVector2D(RsGlobal.maximumWidth / 2, RsGlobal.maximumHeight / 2);
        currentMousePosition = centeredMousePosition;
    }

};

