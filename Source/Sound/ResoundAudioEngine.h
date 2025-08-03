#pragma once

/*
	gon_iss (c) 2025
*/

#include <plugin.h>
#include <game_sa\CAudioEngine.h>
#include "../Utils/Timer.h"


enum SlowMoType /// gameobjects
{
	AUD_SLOWMO_RADIOWHEEL = 8
};

enum class MenuVolumeMemoryAddress {
	RADIO = 0xBA6798,
	SFX = 0xBA6797
};

ThiscallEvent <AddressList<0x4EF3E0, H_CALL>, PRIORITY_AFTER, ArgPickN<CAESound*, 0>, void(CAESound*)> onGetSlowMoFrequencyScalingFactor;

static class ResoundAudioEngine
{
	static inline SoundFade* soundFadeSlowMoSFX = new SoundFade(true, true);
	static inline bool isSlowMoActive = false;

	static constexpr float maxVolume = 0.23f;
	static inline float sfxBasicVolume = maxVolume;
	static inline Timer<CurrentTime> tmrDisableSlowMoSoundFade;

	static float GetSettingsVolume(MenuVolumeMemoryAddress address) {
		unsigned char* volumeSA = reinterpret_cast<unsigned char*>(address);

		float maxVal = address == MenuVolumeMemoryAddress::RADIO ? maxVolume : 1.f;

		if (volumeSA != nullptr) {
			unsigned int intValue = (*volumeSA) & 0x7F;
			float volumeValue = Utils::Lerp(0.0f, maxVal, (float)intValue / 64.0f);
			return volumeValue;
		}

		return maxVolume;
	}

	static void SetSFXVolumeSA(float volume) {
		CAudioEngine* audioEngine = reinterpret_cast<CAudioEngine*>(0xB6BC90);

		if (audioEngine) {

			uint8_t sfxVolume = static_cast<uint8_t>(volume * 64.0f);
			audioEngine->SetEffectsMasterVolume(sfxVolume);
		}
	}

	static inline void SoundFadeProcess_SA_SFX() {
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			
			if (soundFadeSlowMoSFX->isActive) {
				float targetVolume = soundFadeSlowMoSFX->GetValue(sfxBasicVolume * 0.25f, sfxBasicVolume, 500, 550);
				SetSFXVolumeSA(targetVolume);
			}
		}
	}

	static inline void OnSlowMoActivation(SlowMoType type) {
		sfxBasicVolume = GetSFXVolumeSA();

		if (type == SlowMoType::AUD_SLOWMO_RADIOWHEEL) {
			soundFadeSlowMoSFX->Activate();
		}
	}

	static inline void OnSlowMoDeActivation(SlowMoType type) {
		tmrDisableSlowMoSoundFade.Start();
		soundFadeSlowMoSFX->Deactivate();
	}

public:

	static inline bool init = false;

	static void Initialize() {
		if (init) return;
		init = true;

		std::thread soundFadeSFXProcessThread(&SoundFadeProcess_SA_SFX);
		
		soundFadeSFXProcessThread.detach();

		void* targetAddress = (void*)0x4EF440;
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
		return maxVolume;
	}

	static float GetRadioVolumeSA() {
		return GetSettingsVolume(MenuVolumeMemoryAddress::RADIO);
	}

	static float GetSFXVolumeSA() {
		return GetSettingsVolume(MenuVolumeMemoryAddress::SFX);
	}
};