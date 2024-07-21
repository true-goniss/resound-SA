#pragma once
#include <game_sa/CSprite2d.h>
#include <extensions/Screen.h>
#include "Visual/Textures/Textures.h"
#include "Visual/Fonts/DXFont.h"
#include "Utils/ShapeHelper.h"
#include "Animations/OpacityAnimation.h"

using namespace plugin;

static class TrackInfoVisual
{
	static inline CSprite2d* cassetteIcon;
	static inline CSprite2d* cassetteIconRoller;
	static inline float rollerAngle = 100;

	static inline void DrawCassetteIcon(float x1, float y1, float x2, float y2, float opacity) {
		if (!cassetteIcon) {
			cassetteIcon = new CSprite2d();
			cassetteIcon->m_pTexture = Textures::LoadPNGTexture("cassette_icon", "icon");
		}

		CRect rect(SCREEN_COORD_CENTER_LEFT(x1), SCREEN_COORD(y1),
			SCREEN_COORD_CENTER_LEFT(x2), SCREEN_COORD(y2));

		cassetteIcon->Draw(rect, CRGBA(255, 255, 255, opacity));
	}

	static inline void DrawCassetteIconRoller(float centerX, float centerY, float targetWidth, float targetHeight, float playbackSpeed, float opacity)
	{
		float l_angle = fmodf(rollerAngle, 360.0f);
		if (l_angle < 0.0f)
			l_angle += 360.0;
		l_angle = 360.0f - l_angle;

		rollerAngle += (5.05f * playbackSpeed);

		if (!cassetteIconRoller) {
			cassetteIconRoller = new CSprite2d();
			cassetteIconRoller->m_pTexture = Textures::LoadPNGTexture("cassette_icon", "icon_roller");
		}

		float x = centerX - targetWidth * 0.5f;
		float y = centerY - targetHeight * 0.5f;

		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, cassetteIconRoller->m_pTexture->raster);

		float x1 = SCREEN_COORD_CENTER_LEFT(x);
		float y1 = SCREEN_COORD_TOP(y);
		float x2 = SCREEN_COORD_CENTER_LEFT(x + targetWidth);
		float y2 = SCREEN_COORD_TOP(y + targetHeight);

		CRect rect(x1, y1, x2, y2);

		float u1 = 0.0f;
		float v1 = 0.0f;
		float u2 = 1.0f;
		float v2 = 0.0f;
		float u3 = 0.0f;
		float v3 = 1.0f;
		float u4 = 1.0f;
		float v4 = 1.0f;

		CRGBA color = CRGBA(255, 255, 255, opacity);
		CSprite2d::SetVertices(rect, color, color, color, color, u1, v1, u2, v2, u3, v3, u4, v4);
		ShapeHelper::RotateVertices(cassetteIconRoller->maVertices, 4, SCREEN_COORD_CENTER_LEFT(x + targetWidth * 0.5f), SCREEN_COORD_TOP(y + targetHeight * 0.5f), rollerAngle);

		RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, cassetteIconRoller->maVertices, 4);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
	}

	static inline std::string cassetteArtist = "";
	static inline std::string cassetteTrackName = "";

	static inline int opacityText = 0;
	static inline int opacityTextOutline = 0;

public:

	static inline OpacityAnimation* opacityAnimationText = new OpacityAnimation();
	static inline OpacityAnimation* opacityAnimationOutline = new OpacityAnimation();

	static void ShowWithAnimation(std::string cassetteArtistStr, std::string cassetteTrackNameStr) {
		cassetteArtist = cassetteArtistStr;
		cassetteTrackName = cassetteTrackNameStr;

		opacityAnimationText->Activate();
		opacityAnimationOutline->Activate();
	}

	static void ShowWithAnimation() {
		if (cassetteArtist == "" && cassetteTrackName == "") return;

		opacityAnimationText->Activate();
		opacityAnimationOutline->Activate();

	}

	static void DrawTrackInfo(DXFont* fontArtist, DXFont* fontTrack) {

		opacityText = opacityAnimationText->GetValue(255, 500, 3000);
		opacityTextOutline = opacityAnimationOutline->GetValue(200, 500, 3000);

		fontArtist->DrawCenteredOnTop(cassetteArtist.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), 140);
		fontTrack->DrawCenteredOnTop(cassetteTrackName.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), 180);

		float cassIconWidth = 470 / 5.8;
		float cassIconHeight = 299 / 4;

		float x1 = cassIconWidth / 2;
		float y1 = 40;
		float x2 = -cassIconWidth;
		float y2 = 40 + cassIconHeight;


		DrawCassetteIcon(x1 + 20, y1, x2 + 20, y2, opacityText);
		DrawCassetteIconRoller(25, y1 + 30.5, 16, 16, 1.0, opacityText);
		DrawCassetteIconRoller(-25, y1 + 30.5, 16, 16, 1.0, opacityText);

	}

	static void Delete() {
		cassetteIcon->Delete();
		cassetteIconRoller->Delete();
	}
};