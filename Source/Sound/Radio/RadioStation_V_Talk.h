#pragma once

/*
	by gon_iss (c) 2024
*/

/*
	A radio station that plays talk tracks (instead of music) randomly, and 'id's between them 
*/

#include "RadioStation.h"
#include "Sound/SoundPlayer.h"

class RadioStation_V_Talk : public RadioStation
{
	float currentVolume = 0;

	public:

	RadioStation_V_Talk(std::string folder, SettingsRadioStation* settings) : RadioStation(folder, settings) {

		InitPlayer(this->musicPlayer, this->path, false);
		//InitPlayer(this->idPlayer, this->id_path, false);

		//std::thread managePlaybackThread(&RadioStation_V_Talk::ManagePlayback, this);
		//managePlaybackThread.detach();
	}

	void UpdateVolume(bool isMissionTalkingNow) override {

		if (muted) {
			if (musicPlayer)
				musicPlayer->setVolume(0);

			if (idPlayer)
				idPlayer->setVolume(0);

			return;
		}

		float targetVolume = isMissionTalkingNow ? VOL_MULT_DUCKED : VOL_MULT_NORMAL;

		if (abs(currentVolume - targetVolume) > 0.01f) {
			float step = FADE_SPEED * 16.0f;
			if (currentVolume < targetVolume)
				currentVolume = min(currentVolume + step, targetVolume);
			else
				currentVolume = max(currentVolume - step, targetVolume);

			if (musicPlayer) {
				musicPlayer->setVolume(currentVolume * basicVolume);
			}

			if (idPlayer) {
				idPlayer->setVolume(currentVolume * basicVolume);
			}
		}
	}

	std::pair<std::string, std::string> TryGetArtistTitle() const override {
		if (musicPlayer->isPlayingOrActive()) {
			return Utils::GetTrackArtistAndName(Utils::remove_music_extension(musicPlayer->lastplayedTrack));
		}
		else {
			return std::make_pair("", "");
		}
	}

	const std::string id_path = this->path + "\\id\\";
	SoundPlayer* idPlayer = nullptr;

	protected:

	bool nowPlayingId = false;

	void ManagePlayback() {

		while (true) {

			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (!musicPlayer->isPlayingOrActive() && !nowPlayingId) {
				nowPlayingId = true;
				idPlayer->playNextTrack();
				CheckVolumeOnPlay(idPlayer);

				continue;
			}

			if (!idPlayer->isPlayingOrActive() && nowPlayingId) {
				nowPlayingId = false;
				musicPlayer->playNextTrack();
				CheckVolumeOnPlay(musicPlayer);

				continue;
			}
		}
	}
};

