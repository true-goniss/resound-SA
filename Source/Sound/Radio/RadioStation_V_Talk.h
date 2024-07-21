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
	
	public:

	RadioStation_V_Talk(std::string folder, SettingsRadioStation* settings) : RadioStation(folder, settings) {

		InitPlayer(this->musicPlayer, this->path, false);
		//InitPlayer(this->idPlayer, this->id_path, false);

		//std::thread managePlaybackThread(&RadioStation_V_Talk::ManagePlayback, this);
		//managePlaybackThread.detach();
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

