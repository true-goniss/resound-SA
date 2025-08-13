/*
    by gon_iss (c) 2024
*/

#pragma once
#include "../../Utils/Utils.h"
#include "../../Settings/SettingsRadioStation.h"
#include "../../Animations/SoundFade.h"
#include "../SoundPlayer.h"
#include "../ResoundAudioEngine.h"
#include "../../Utils/GamePausedWatcher.h"

class RadioStation
{
    public:

    //TODO bug stop retune sound if not in radio car

    //CSprite2d* icon = nullptr;
    CSprite2d* icon_round = nullptr;

    SettingsRadioStation* settings;
    SoundPlayer* musicPlayer;

    std::vector<SoundPlayer*> soundPlayers;

    std::string folder;
    std::string name;
    std::string path;
    std::string trackname = "";
    std::string timeSpeeches = "";
    std::string timeState = "none";
    std::string previoustimeState = "";


    float basicVolume = ResoundAudioEngine::GetMaxVolume();

    bool muted = false;
    bool forcePaused = false;
    bool timePlayed = false;
    bool timeOnIntroOrOutroFlag = false;
    bool timeNeedsToBePlayed = false;
    bool isTunedNow = false;

    SoundFade* introAndOutroMusicFade = new SoundFade(true);
    float introFadeMinVolumeMult = 0.25f;
    int introFadeLengthMs = 500;
    int wholeIntroFadeLength = 5000;

    RadioStation(std::string folder, SettingsRadioStation* settings) : folder(folder), settings(settings) {
        this->folder = folder;
        this->name = folder;
        this->path = "resound\\radio\\" + folder;// + "\\";

        GamePausedWatcher::AddHandler([this](GamePausedWatcher::EventType eventType) {
            switch (eventType)
            {
                case GamePausedWatcher::EventType::Paused:
                    this->Pause();
                    break;
                case GamePausedWatcher::EventType::Unpaused:
                    this->Unpause();
                    break;
            }
        });

        Events::reInitGameEvent += [this] {
            Randomize();
        };

        //Mute();
    }

    int GetStationId() const {
        static std::hash<std::string> hasher;
        return static_cast<int>(hasher(name)) % 100000 + 14;
    }

    void InitPlayer(SoundPlayer*& player, const std::string& path, bool looped) {
        player = new SoundPlayer(path, looped);

        musicPlayer->setVolume(basicVolume);
        soundPlayers.push_back(player);
    }

    virtual void Randomize() {
        musicPlayer->randomizePosition();
        Mute();
    }

    virtual void Stop() {
        StopAllPlayers();
    }

    virtual void Pause() {
        AdjustVolumeValueFromSettings();

        if (this->forcePaused) return;

        this->forcePaused = true;
        PauseAllPlayers();
    }

    virtual void Unpause() {
        if (!this->forcePaused) return;

        AdjustVolumeValueFromSettings();

        this->forcePaused = false;
        UnpauseAllPlayers();
    }

    virtual void PlayMusicTrack() {
        trackname = musicPlayer->playNewTrack();
        CheckVolumeOnPlay(musicPlayer);
    }

    virtual void Launch() {
        PlayMusicTrack();
        Randomize();
    }

    virtual std::pair<std::string, std::string> TryGetArtistTitle() const {
        return std::make_pair("", "");
    }

    virtual void Mute() {
        AdjustVolumeValueFromSettings();

        muted = true;
        MuteAllPlayers();
    }

    virtual void Unmute() {
        AdjustVolumeValueFromSettings();

        muted = false;
        UnmuteAllPlayers();
    }

    void CheckVolumeOnPlay(SoundPlayer* player) {

        AdjustVolumeValueFromSettings();

        if (muted) {
            player->setVolumeMute();
        }
        else {
            player->setVolume(basicVolume);
        }
    }

    void SetVolumeAllPlayers(float volume) {
        for (SoundPlayer* sndPlayer : soundPlayers) {
            if (sndPlayer && !sndPlayer->isEmpty) sndPlayer->setVolume(volume);
        }
    }

    protected:

    void UnpauseAllPlayers() {
        for (SoundPlayer* sndPlayer : soundPlayers) {
            if (sndPlayer && !sndPlayer->isEmpty) sndPlayer->playContinueTrack();
        }
    }

    void PauseAllPlayers() {
        for (SoundPlayer* sndPlayer : soundPlayers) {
            if (sndPlayer && !sndPlayer->isEmpty) sndPlayer->pauseTrack();
        }
    }

    void MuteAllPlayers() {
        for (SoundPlayer* sndPlayer : soundPlayers) {
            if (sndPlayer && !sndPlayer->isEmpty) sndPlayer->setVolumeMute();
        }
    }

    void UnmuteAllPlayers() {
        for (SoundPlayer* sndPlayer : soundPlayers) {
            if (sndPlayer && !sndPlayer->isEmpty) sndPlayer->setVolumeUnmute(basicVolume);
        }
    }

    void StopAllPlayers() {
        for (SoundPlayer* sndPlayer : soundPlayers) {
            if (sndPlayer && !sndPlayer->isEmpty) sndPlayer->stopTrack();
        }
    }

    void AdjustVolumeValueFromSettings() {
        basicVolume = ResoundAudioEngine::GetRadioVolumeSA();
    }
};