#pragma once
#include "plugin.h"
#include <game_sa/CSprite2d.h>
#include <unordered_map>

#include "../Visual/Textures/Textures.h"
#include "../Sound/Radio/RadioStation.h"

static class RadioIcons
{

	static inline bool init = false;

public:

	static inline CSprite2d* radio_selector = nullptr;

	static inline std::unordered_map<std::string, CSprite2d*> icons;
	static inline std::unordered_map<std::string, CSprite2d*> icons_round;
	static inline std::unordered_map<std::string, CSprite2d*> icons_custom_radio;
	static inline std::unordered_map<std::string, CSprite2d*> icons_custom_radio_round;

	static void Initialize(std::vector<std::string> SA_Radio_Stations, std::vector<RadioStation*> Custom_Radio_Stations) {
		if (!init) {
			init = true;
		}
		else { return; }

		CSprite2d* noIconSprite = new CSprite2d();
		noIconSprite->m_pTexture = Textures::LoadPNGTexture("radio_icons_dummy", "radio_hud_no_icon_grayscale");

		radio_selector = new CSprite2d();
		radio_selector->m_pTexture = Textures::LoadPNGTexture("radio_icons", "radio_selector");

		for (int i = 1; i <= 13; ++i) {
			std::string textureName = std::to_string(i);
			CSprite2d* sprite = new CSprite2d();
			sprite->m_pTexture = Textures::LoadPNGTexture("radio_icons", textureName.c_str());

			CSprite2d* sprite_round = new CSprite2d();
			sprite_round->m_pTexture = Textures::LoadPNGTexture("radio_icons", (textureName + "_round").c_str());

			icons[SA_Radio_Stations[i - 1]] = sprite;
			icons_round[SA_Radio_Stations[i - 1]] = sprite_round;
		}

		for (RadioStation* station : Custom_Radio_Stations) {

			RwTexture* tex = Textures::LoadPNGCustomFolder(station->path.c_str(), "icon");

			if (tex != NULL) {
				CSprite2d* sprite = new CSprite2d();
				sprite->m_pTexture = tex;

				icons_custom_radio[station->name] = sprite;
			}
			else {
				icons_custom_radio[station->name] = noIconSprite;
			}

			RwTexture* tex_round = Textures::LoadPNGCustomFolder(station->path.c_str(), "icon_round");

			if (tex_round != NULL) {
				CSprite2d* sprite_round = new CSprite2d();
				sprite_round->m_pTexture = tex_round;

				icons_custom_radio_round[station->name] = sprite_round;

				station->icon_round = sprite_round;
			}
			else {
				icons_custom_radio_round[station->name] = noIconSprite;

				station->icon_round = noIconSprite;
			}

		}
	}

	//static CSprite2d* getIcon(bool isCustomRadio, bool isRound, const std::string& name) {

	//	if (isCustomRadio) {
	//		if (isRound) {
	//			auto it = icons_custom_radio_round.find(name);
	//			if (it != icons_custom_radio_round.end()) return it->second;
	//		}
	//		else {
	//			auto it = icons_custom_radio.find(name);
	//			if (it != icons_custom_radio.end())  return it->second;
	//		}
	//	}
	//	else {
	//		if (isRound) {
	//			auto it = icons_round.find(name);
	//			if (it != icons_round.end())  return it->second;
	//		}
	//		else {
	//			auto it = icons.find(name);
	//			if (it != icons.end()) return it->second;
	//		}
	//	}

	//	return nullptr;
	//}

	static CSprite2d* getIcon(bool isCustomRadio, bool isRound, const std::string& name) {

		const std::unordered_map<std::string, CSprite2d*>* selectedMap = nullptr;
		if (isCustomRadio) {
			selectedMap = isRound ? &icons_custom_radio_round : &icons_custom_radio;
		}
		else {
			selectedMap = isRound ? &icons_round : &icons;
		}

		return findIcon(*selectedMap, name);
	}

	static CSprite2d* getIconRadioOff() {
		return getIcon(false, true, "Radio Off");
	}

protected:

	static CSprite2d* findIcon(const std::unordered_map<std::string, CSprite2d*>& iconMap, const std::string& name) {
		auto it = iconMap.find(name);
		return (it != iconMap.end()) ? it->second : nullptr;
	}

};

