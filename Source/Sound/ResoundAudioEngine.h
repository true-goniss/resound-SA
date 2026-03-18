#pragma once

/*
	gon_iss (c) 2025
*/

#include <plugin.h>
#include <game_sa\CAudioEngine.h>
#include <algorithm> // std::clamp


enum SlowMoType /// gameobjects
{
	AUD_SLOWMO_RADIOWHEEL = 8
};

enum class MenuVolumeMemoryAddress {
	RADIO = 0xBA6798,
	SFX = 0xBA6797
};

//static ThiscallEvent <AddressList<0x4EF3E0, H_CALL>, PRIORITY_AFTER, ArgPickN<CAESound*, 0>, void(CAESound*)> onGetSlowMoFrequencyScalingFactor;

static class ResoundAudioEngine
{
	static inline bool isSlowMoActive = false;

	static constexpr float SLOWMO_VOLUME_DUCK_MULT = 0.25f;
	static constexpr float SA_MAX_VOLUME_VALUE_CONST = 64.0f;
	static constexpr float RADIO_MAX_LIMIT = 0.23f; // should be configurable
	static inline float sfxOriginalVolume = RADIO_MAX_LIMIT;

	static float GetSettingsVolume(MenuVolumeMemoryAddress address) {
		unsigned char* volumeSA = reinterpret_cast<unsigned char*>(address);
		if (!volumeSA) return 1.0f;

		// SA value is 0-64
		float rawValue = static_cast<float>(*volumeSA & 0x7F);
		float normalized = std::clamp(rawValue / SA_MAX_VOLUME_VALUE_CONST, 0.0f, 1.0f);

		float limit = (address == MenuVolumeMemoryAddress::RADIO) ? RADIO_MAX_LIMIT : 1.0f;
		return normalized * limit;
	}

	static void SetSFXVolumeSA(float volume) {

		CAudioEngine* audioEngine = reinterpret_cast<CAudioEngine*>(0xB6BC90);

		if (audioEngine) {
			//  0.0-1.0 to 0-64
			uint8_t sfxVolume = static_cast<uint8_t>(std::clamp(volume, 0.0f, 1.0f) * SA_MAX_VOLUME_VALUE_CONST);
			audioEngine->SetEffectsMasterVolume(sfxVolume);
		}
	}

	static inline void OnSlowMoActivation(SlowMoType type) {
		if (type != SlowMoType::AUD_SLOWMO_RADIOWHEEL) return;

		sfxOriginalVolume = GetSFXVolumeSA();
		SetSFXVolumeSA(sfxOriginalVolume * SLOWMO_VOLUME_DUCK_MULT);
	}

	static inline void OnSlowMoDeActivation(SlowMoType type) {
		if (type != SlowMoType::AUD_SLOWMO_RADIOWHEEL) return;

		SetSFXVolumeSA(sfxOriginalVolume);
	}

public:

	static inline bool init = false;

	static void Initialize() {
		if (init) return;
		init = true;

		// void* targetAddress = (void*)0x4EF440;
	}

	static void ActivateSlowMoMode(SlowMoType type) {
		if (!isSlowMoActive) {
			isSlowMoActive = true;
			OnSlowMoActivation(type);
		}
	}

	static void DeActivateSlowMoMode(SlowMoType type) {
		if (isSlowMoActive) {
			isSlowMoActive = false;
			OnSlowMoDeActivation(type);
		}
	}

	static float GetMaxVolume() {
		return RADIO_MAX_LIMIT;
	}

	static float GetRadioVolumeSA() {
		return GetSettingsVolume(MenuVolumeMemoryAddress::RADIO);
	}

	static float GetSFXVolumeSA() {
		return GetSettingsVolume(MenuVolumeMemoryAddress::SFX);
	}
};