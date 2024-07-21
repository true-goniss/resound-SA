/*
    Plugin-SDK (Grand Theft Auto) source file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/

#pragma once
#include "plugin.h"
#include "RenderWare.h"
#include "d3dx9.h"
#ifndef GTASA
#include "rwd3d9.h"
#endif

using namespace plugin;

class DXFont {
    void DrawOutline(float outlinePx, RECT rect, D3DCOLOR color, const char* text) {
        float outline_x[] = {
            outlinePx,
            -outlinePx,
            0.0f,
            0.0f,
            outlinePx,
            -outlinePx,
            outlinePx,
            -outlinePx,
        };

        float outline_y[] = {
            0.0f,
            0.0f,
            outlinePx,
            -outlinePx,
            outlinePx,
            outlinePx,
            -outlinePx,
            -outlinePx,
        };

        for (int i = 0; i < 8; i++) {
            RECT outlineRect = rect;
            OffsetRect(&outlineRect, outline_x[i], outline_y[i]);
            m_pD3DXFont->DrawTextA(NULL, text, -1, &outlineRect, DT_CENTER, color);
        }
    }

public:

    ID3DXFont* m_pD3DXFont;

    void InitFont(const char* fontPath, const char* fontName, const int size) {

        char path[1024];
        sprintf(path, "resound\\fonts\\%s", fontPath);

        char name[256];
        sprintf(name, "%s", fontName);

        if (AddFontResourceEx(PLUGIN_PATH(path), FR_PRIVATE, NULL)) {
            HRESULT h = AddFont(size, 0, 1, 0, name, &m_pD3DXFont);
            //if (h != S_OK);
        }

    }

    long AddFont(int height, int width, int charSet, int quality, char fontName[128], LPD3DXFONT* font) {
        return D3DXCreateFontA(GetD3DDevice(), height, width, FW_NORMAL, 0, FALSE, charSet, OUT_DEFAULT_PRECIS, quality, DEFAULT_PITCH | FF_DONTCARE, fontName, font);
    }

    void DestroyFont() {
        if (m_pD3DXFont) {
            m_pD3DXFont->Release();
            m_pD3DXFont = NULL;
        }
    }

    void DrawCenteredOnTop(const char* text, bool outline, float outlinePx, D3DCOLOR outlineColor, D3DCOLOR textColor, int offsetFromTheTop) {
        if (m_pD3DXFont) {
            RECT rect;
            rect.left = 0;
            rect.top = 0;
            rect.bottom = RsGlobal.maximumHeight;
            rect.right = RsGlobal.maximumWidth;

            RECT textSizeRect = rect;
            m_pD3DXFont->DrawTextA(NULL, text, -1, &textSizeRect, DT_CALCRECT, 0);

            // center the text almost at the top
            int screenWidth = RsGlobal.maximumWidth;
            int textWidth = textSizeRect.right - textSizeRect.left;
            int textHeight = textSizeRect.bottom - textSizeRect.top;
            int x = (screenWidth - textWidth) / 2; // Center horizontally
            int y = offsetFromTheTop; // y offset

            RECT centeredRect = { x, y, x + textWidth, y + textHeight };

            if (outline) DrawOutline(outlinePx, centeredRect, outlineColor, text);
            m_pD3DXFont->DrawTextA(NULL, text, -1, &centeredRect, DT_TOP, textColor);
        }
    }

    DXFont() : m_pD3DXFont(nullptr) {
        m_pD3DXFont = NULL;

        Events::shutdownRwEvent.Add([this] { DestroyFont(); });
        Events::d3dLostEvent.Add([this] { DestroyFont(); });
    }

    ~DXFont() {
        DestroyFont();
    }
};