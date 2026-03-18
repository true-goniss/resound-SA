#pragma once
/*
    by gon_iss (c) 2024
*/
#include <fstream>

#include "RadioStation.h"
#include "../SoundPlayer.h"
#include "../../Utils/Utils.h"

class RadioStation_IIIVC : public RadioStation
{
    float currentVolume = 0;

    public:

    RadioStation_IIIVC(std::string folder, SettingsRadioStation* settings) : RadioStation(folder, settings) {
        singleFileTracks = Utils::ReadSingleFileTracksInfo(this->path, "singleFileTracksInfo.txt");

        InitPlayer(musicPlayer, this->path, true);

        //musicPlayer = new SoundPlayer(this->path, true);
    }

    std::pair<std::string, std::string> TryGetArtistTitle() const override {
        if (singleFileTracks.empty()) {
            return std::make_pair("", "");
        }

        int posMs = musicPlayer->getTrackPositionMs();

        for (Utils::SingleFileTrackInfo info : singleFileTracks)
        {
            int start = info.startTimeMs;
            int end = info.endTimeMs;

            if (posMs > start && posMs < end) {
                return std::make_pair(info.artist, info.title);
            }
        }

        return std::make_pair("", "");
    }

    void Randomize() override {
        musicPlayer->randomizePosition();
        Mute();
        //RadioStation::Randomize();
    }

    void UpdateVolume(bool isMissionTalkingNow) override {

        if (muted) {
            if(musicPlayer)
                musicPlayer->setVolume(0);

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
        }
    }

    void Mute() override {
        musicPlayer->setVolumeMute();
        //RadioStation::Mute();
    }

    void Unmute() override {
        musicPlayer->setVolumeUnmute(this->basicVolume);
        //RadioStation::Unmute();
    }

    void Stop() override {
        musicPlayer->stopTrack();
        //RadioStation::Stop();
    }

    void PlayMusicTrack() override {
        trackname = musicPlayer->playNewTrack();
        CheckVolumeOnPlay(musicPlayer);
        // RadioStation::PlayMusicTrack();
    }

    protected:

    std::vector<Utils::SingleFileTrackInfo> singleFileTracks;
};

