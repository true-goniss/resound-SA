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
        RadioStation::Randomize();
    }

    void Mute() override {
        RadioStation::Mute();
    }

    void Unmute() override {
        RadioStation::Unmute();
    }

    void Stop() override {
        RadioStation::Stop();
    }

    void PlayMusicTrack() override {
        RadioStation::PlayMusicTrack();
    }

    protected:

    std::vector<Utils::SingleFileTrackInfo> singleFileTracks;
};

