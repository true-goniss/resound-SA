#pragma once
/*
    by gon_iss (c) 2024
*/

/*
    A radio station similar to VC / III stations but that's divided by several files that played consistently for resources usage optimization. Tracklist is .txt file with the same name as radio part
*/

#include "RadioStation.h"
#include <map>
#include "../../Utils/Utils.h"

class RadioStation_ConsistentlyPlayed : public RadioStation
{
    std::map<int, std::string> parts;
    int currentPart = -1;

    public:

    RadioStation_ConsistentlyPlayed(std::string folder, SettingsRadioStation* settings) : RadioStation(folder, settings) {

        InitPlayer(this->musicPlayer, this->path, false);

        parts = Utils::GetFolderPartFilesMap(this->path);

        std::thread managePlaybackThread(&RadioStation_ConsistentlyPlayed::ManagePlayback, this);
        managePlaybackThread.detach();
    }

    void Launch() override {

    }

    void Randomize() override {
        int maxPart = parts.rbegin()->first;
        int randomPart = Utils::getRandomInt(1, maxPart);

        musicPlayer->playTrackBASS(parts[randomPart]);
        musicPlayer->randomizePosition();
        UpdateTrackNames();
    }

    void PlayMusicTrack() override {

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

    protected:

    void ManagePlayback() {

        while (true) {

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (forcePaused) {
                continue;
            }

            if (!musicPlayer->isPlayingOrActive()) {

                int maxPart = parts.rbegin()->first;

                if (currentPart == -1) {
                    currentPart = 1;
                }
                else {
                    currentPart++;

                    if (currentPart > maxPart) {
                        currentPart = 1;
                    }
                }

                musicPlayer->playTrackBASS( parts[currentPart] );
                UpdateTrackNames();
            }
        }

    }

    void UpdateTrackNames() {
        std::string tracknamesFileName = Utils::remove_music_extension(parts[currentPart]) + ".txt";
        singleFileTracks = Utils::ReadSingleFileTracksInfo(this->path, tracknamesFileName);
    }

    std::vector<Utils::SingleFileTrackInfo> singleFileTracks;
};

