#pragma once
#include "Fonts/DXFont.h"
#include "../Animations/OpacityAnimation.h"
#include "../Sound/Radio/RadioStation.h"
#include "RadioIcons.h"



static class RadioInfoVisual
{
	static inline int opacityText = 0;
	static inline int opacityTextOutline = 0;

	static inline std::string radioStationName = "";
	static inline std::string radioArtistName = "";
	static inline std::string radioTrackName = "";





	static inline bool init = false;
	static inline std::string radioName = "";

	static inline bool isCustomRadio = false;
	static inline bool isNowShownOnTheWheel = false;

	static void DrawRadioIcon(CSprite2d* icon, float x1, float y1, float x2, float y2, int opacity) {
		CRect rect = { x1, y1, x2, y2 };
		if (icon) icon->Draw(rect, CRGBA(255, 255, 255, opacity));
	}

public:

	static void DrawRadioInfo(DXFont* fontArtist, DXFont* fontTrack, bool onTheWheel) {

		if (!fontArtist || !fontTrack)
			return;

		if (!onTheWheel && !isNowShownOnTheWheel) {
			opacityText = opacityAnimationText->GetValue(255, 500, 3000);
			opacityTextOutline = opacityAnimationOutline->GetValue(200, 500, 3000);
		}
		else {
			isNowShownOnTheWheel = true;
			opacityText = 255;
			opacityTextOutline = 200;
		}


		float iconWidth = 80;
		float iconHeight = 80;

		float text_y1 = 140;
		float text_y2 = 180;
		float text_y3 = 220;

		int screenWidth = RsGlobal.maximumWidth;

		float x1 = (screenWidth / 2) - iconWidth / 2;
		float y1 = 40;
		float x2 = x1 + iconWidth;
		float y2 = 40 + iconHeight;

		if (!onTheWheel && !isNowShownOnTheWheel) {
			CSprite2d* icon = RadioIcons::getIcon(isCustomRadio, false, radioName);
			if (!icon) {
				text_y1 -= iconHeight;
				text_y2 -= iconHeight;
				text_y3 -= iconHeight;
			}
			DrawRadioIcon(icon, x1, y1, x2, y2, opacityText);

			fontArtist->DrawCenteredOnTop(radioStationName.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), text_y1);
			fontTrack->DrawCenteredOnTop(radioArtistName.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), text_y2);
			fontTrack->DrawCenteredOnTop(radioTrackName.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), text_y3);
		}
		else {
			int wheelUpOffset = -110;

			fontArtist->DrawCentered(radioStationName.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), wheelUpOffset);
			fontTrack->DrawCentered(radioArtistName.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), wheelUpOffset + 40);
			fontTrack->DrawCentered(radioTrackName.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), wheelUpOffset + 80);
		}
	}

	static void SetInfo(std::string radioStationNameStr, std::string radioArtistNameStr, std::string radioTrackNameStr, std::string radioNameStr, bool isCustom) {
		radioStationName = radioStationNameStr;
		radioArtistName = radioArtistNameStr;
		radioTrackName = radioTrackNameStr;
		radioName = radioNameStr;

		isCustomRadio = isCustom;
	}

	static void ShowWithAnimation(std::string radioStationNameStr, std::string radioArtistNameStr, std::string radioTrackNameStr, std::string radioNameStr, bool isCustom) {

		isNowShownOnTheWheel = false;

		radioStationName = radioStationNameStr;
		radioArtistName = radioArtistNameStr;
		radioTrackName = radioTrackNameStr;
		radioName = radioNameStr;

		isCustomRadio = isCustom;

		opacityAnimationText->Activate();
		opacityAnimationOutline->Activate();
	}

	static void HideForcely() {
		isNowShownOnTheWheel = false;
		opacityText = 0;
	}



	static inline OpacityAnimation* opacityAnimationText = new OpacityAnimation();
	static inline OpacityAnimation* opacityAnimationOutline = new OpacityAnimation();
};