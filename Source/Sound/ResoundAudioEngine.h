#pragma once
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

		//std::thread manage(Async::main);
		//manage.detach();

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

		//plugin::patch(targetAddress, GetSlowMoFrequencyScalingFactorHook);
		//onGetSlowMoFrequencyScalingFactor += [](CAESound* pThis) {
		//	return 0.0f;
		//};

		//tthread.detach();

		//Reverb::ReverbInit();
		//Reverb::ApplyReverbEffectToChannels();
	}



	//float ModifySlowMoFrequencyScaling(CAESound* pThis)
	//{
	//	// Call the original function to preserve default behavior
	//	float originalScalingFactor = 1.f;

	//	// Modify the frequency scaling factor (pitch multiplier)
	//	float customScalingFactor = 1.0f;

	//	if (CTimer::GetIsSlowMotionActive()) {
	//		customScalingFactor = 2.0f; // Example: double the pitch during slow-motion
	//	}

	//	// Optionally, you can add more conditions to adjust the scaling factor for different scenarios
	//	//if (CCamera::GetActiveCamera().m_nMode == eCamMode::MODE_CAMERA) {
	//	//	customScalingFactor = 1.0f; // Reset the scaling factor if in a specific camera mode
	//	//}

	//	// Return the custom scaling factor (multiply the original scaling factor by our custom factor)
	//	return originalScalingFactor * customScalingFactor;
	//}


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



//if (isSlowMoActive) {
//	SetSFXVolumeSA(soundFadeSlowMoSFX->GetValue(sfxBasicVolume * 0.1f, sfxBasicVolume, 1100, 200000));
//}
//else {
//	//if (!tmrDisableSlowMoSoundFade.IsComplete(1100, true)) {
//	//	continue;
//	//}
//	//else {
//	//	SetSFXVolumeSA(soundFadeSlowMoSFX->GetValue(sfxBasicVolume * 0.1f, sfxBasicVolume, 1100, 200000));
//	//}

//	//SetSFXVolumeSA(1.0f);
//}