#pragma once
#include <game_sa/CSprite2d.h>
#include <extensions/Screen.h>
#include "Visual/Textures/Textures.h"

static class CassettePlayerTextures
{
	static inline CSprite2d* backgroundSprite;
	static inline CSprite2d* frameSprite;

	public:

	static void DrawBackground(float coordRight_X1, float coordBottom_Y1, float coordRight_X2, float coordBottom_Y2) {

		if (!backgroundSprite) {

			backgroundSprite = new CSprite2d();
			backgroundSprite->m_pTexture = Textures::LoadPNGTexture("cassette_player", "background");
		}

		CRect rect(SCREEN_COORD_RIGHT(coordRight_X1), SCREEN_COORD_BOTTOM(coordBottom_Y1),
			SCREEN_COORD_RIGHT(coordRight_X2), SCREEN_COORD_BOTTOM(coordBottom_Y2));

		backgroundSprite->Draw(rect, CRGBA(255, 255, 255, 255));
	}

	static void DrawFrame(float coordRight_X1, float coordBottom_Y1, float coordRight_X2, float coordBottom_Y2) {

		if (!frameSprite) {

			frameSprite = new CSprite2d();
			frameSprite->m_pTexture = Textures::LoadPNGTexture("cassette_player", "frame");
		}

		CRect rect(SCREEN_COORD_RIGHT(coordRight_X1), SCREEN_COORD_BOTTOM(coordBottom_Y1),
			SCREEN_COORD_RIGHT(coordRight_X2), SCREEN_COORD_BOTTOM(coordBottom_Y2));

		frameSprite->Draw(rect, CRGBA(255, 255, 255, 255));
	}

	static void Delete() {
		backgroundSprite->Delete();
		frameSprite->Delete();
	}
};