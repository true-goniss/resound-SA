
#pragma once
#include <vector>
#include <game_sa/CSprite2d.h>
#include <extensions/Screen.h>

#include "Visual/Textures/Textures.h"

static class CassetteInterfaceTextures
{
	static inline CSprite2d* currentSprite;
	static inline std::string currentDrawnName;

public:

	static inline std::string textureCurrent = "";
	static inline std::string texturePrevious = "";

	static void Draw(float coordRight_X1, float coordBottom_Y1, float coordRight_X2, float coordBottom_Y2) {

		if (!currentSprite || currentDrawnName != textureCurrent) {
			currentDrawnName = textureCurrent;
			currentSprite = new CSprite2d();

			currentSprite->m_pTexture = Textures::LoadPNGTexture("cassette_player", textureCurrent.c_str());
		}

		CRect rect(SCREEN_COORD_RIGHT(coordRight_X1), SCREEN_COORD_BOTTOM(coordBottom_Y1),
			SCREEN_COORD_RIGHT(coordRight_X2), SCREEN_COORD_BOTTOM(coordBottom_Y2));

		currentSprite->Draw(rect, CRGBA(255, 255, 255, 255));
	}

	static void SetTexture(const std::string& textureName) {
		textureCurrent = textureName;
	}

	static void Delete() {
		currentSprite->Delete();
	}
};