#pragma once
#include "Fonts/DXFont.h"
#include "../Animations/OpacityAnimation.h"
#include "../Sound/Radio/RadioStation.h"

using namespace std;

static class RadioInfoVisual
{
	static inline int opacityText = 0;
	static inline int opacityTextOutline = 0;

	static inline std::string radioStationName = "";
	static inline std::string radioArtistName = "";
	static inline std::string radioTrackName = "";

	static inline std::unordered_map<std::string, CSprite2d*> icons;
	static inline std::unordered_map<std::string, CSprite2d*> icons_custom_radio;

	static inline bool iconsInit = false;
	static inline std::string radioName = "";

	static inline bool isCustomRadio = false;

	static void addIcon(const std::string& name, CSprite2d* icon) {
		icons[name] = icon;
	}

	static void addIconCustomRadio(const std::string& name, CSprite2d* icon) {
		icons_custom_radio[name] = icon;
	}

	static CSprite2d* getIcon(const std::string& name) {

		if (isCustomRadio) {
			auto it = icons_custom_radio.find(name);
			if (it != icons_custom_radio.end()) {
				return it->second;
			}
		}
		else {
			auto it = icons.find(name);
			if (it != icons.end()) {
				return it->second;
			}
		}

		return nullptr;
	}

	static void DrawRadioIcon(CSprite2d* icon, float x1, float y1, float x2, float y2, int opacity) {
		CRect rect = { x1, y1, x2, y2 };
		if (icon) icon->Draw(rect, CRGBA(255, 255, 255, opacity));
	}

public:

	static void InitializeIcons(std::vector<std::string> SA_Radio_Stations, std::vector<RadioStation*> Custom_Radio_Stations) {
		if (!iconsInit) {
			iconsInit = true;

			for (int i = 1; i <= 13; ++i) {
				std::string textureName = std::to_string(i);
				CSprite2d* sprite = new CSprite2d();
				sprite->m_pTexture = Textures::LoadPNGTexture("radio_icons", textureName.c_str());
				addIcon(SA_Radio_Stations[i - 1], sprite);
			}

			for (RadioStation* station : Custom_Radio_Stations) {
				RwTexture* tex = Textures::LoadPNGCustomFolder(station->path.c_str(), "icon");

				if (tex != NULL) {
					CSprite2d* sprite = new CSprite2d();
					sprite->m_pTexture = tex;

					addIconCustomRadio(station->name, sprite);
				}
			}
		}
	}

	static void DrawRadioInfo(DXFont* fontArtist, DXFont* fontTrack) {

		opacityText = opacityAnimationText->GetValue(255, 500, 3000);
		opacityTextOutline = opacityAnimationOutline->GetValue(200, 500, 3000);

		float iconWidth = 80;
		float iconHeight = 80;

		float text_y1 = 140;
		float text_y2 = 180;
		float text_y3 = 220;

		CSprite2d* icon = getIcon(radioName);
		if (!icon) {
			text_y1 -= iconHeight;
			text_y2 -= iconHeight;
			text_y3 -= iconHeight;
		}

		fontArtist->DrawCenteredOnTop(radioStationName.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), text_y1);
		fontTrack->DrawCenteredOnTop(radioArtistName.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), text_y2);
		fontTrack->DrawCenteredOnTop(radioTrackName.c_str(), true, 2, D3DCOLOR_RGBA(0, 0, 0, opacityTextOutline), D3DCOLOR_RGBA(240, 240, 240, opacityText), text_y3);

		int screenWidth = RsGlobal.maximumWidth;

		float x1 = (screenWidth / 2) - iconWidth / 2;
		float y1 = 40;
		float x2 = x1 + iconWidth;
		float y2 = 40 + iconHeight;

		DrawRadioIcon(icon, x1, y1, x2, y2, opacityText);
	}

	static void ShowWithAnimation(std::string radioStationNameStr, std::string radioArtistNameStr, std::string radioTrackNameStr, std::string radioNameStr, bool isCustom) {

		radioStationName = radioStationNameStr;
		radioArtistName = radioArtistNameStr;
		radioTrackName = radioTrackNameStr;
		radioName = radioNameStr;

		isCustomRadio = isCustom;

		opacityAnimationText->Activate();
		opacityAnimationOutline->Activate();
	}

	static inline OpacityAnimation* opacityAnimationText = new OpacityAnimation();
	static inline OpacityAnimation* opacityAnimationOutline = new OpacityAnimation();
};