#pragma once

/*
    by gon_iss (c) 2024
*/

#include "CAudioEngine.h"
#include "RadioStation.h"
#include "../SoundPlayer.h"
#include "../../Animations/SoundFade.h"
#include "../../Utils/Keys.h"

using namespace std;

enum class RadioState {
    None,
    PlayMusicTrack,
    TrackMid,
    WaitingForOutro,
    SkippingOutro,
    PlayingIntro,
    MusicTrackEnded,
    PlayAdvert,
    AdvertEnded,
    PlayID,
    PlaySolo,
    PlayNews,
    PlayingOutro,
    OutroEnded
};

class RadioStation_V : public RadioStation
{
private:

    const std::string KEYWORD_NEWS = "NEWS";
    const std::string KEYWORD_AD = "AD";
    const std::string KEYWORD_EVENING = "EVENING";
    const std::string KEYWORD_MORNING = "MORNING";


    const int SLEEP_MAIN_LOOP_MS = 100;
    const int DEBUG_JUMP_PERCENT = 93;
    const float INTRO_SKIP_PROBABILITY = 0.37f;
    const float SOLO_PLAY_PROBABILITY = 0.2f;
    const int SLEEP_AFTER_NEWS_MS = 1000;
    const int MUSIC_FADE_OUT_OFFSET = 4000;
    const int EVENING_START_HOUR = 19;
    const int EVENING_END_HOUR = 23;
    const int MORNING_START_HOUR = 5;
    const int MORNING_END_HOUR = 9;

public:
    RadioStation_V(std::string folder, SettingsRadioStation* settings) : RadioStation(folder, settings) {

        InitPlayer(this->musicPlayer, this->path, false);
        InitPlayer(this->advertsPlayer, adverts_path, false);
        InitPlayer(this->newsPlayer, news_path, false);
        InitPlayer(this->advertsTransitionPlayer, to_path, false);
        InitPlayer(this->idPlayer, id_path, false);
        InitPlayer(this->introPlayer, intro_path, false);
        InitPlayer(this->soloPlayer, solo_path, false);
        InitPlayer(this->outroPlayer, general_path, false);

        std::thread managePlaybackThread(&RadioStation_V::ManagePlayback, this);
        managePlaybackThread.detach();
    }

    std::pair<std::string, std::string> TryGetArtistTitle() const override {
        if (musicPlayer->isPlayingOrActive() && isMusicTrackNow) {
            return Utils::GetTrackArtistAndName(Utils::remove_music_extension(musicPlayer->lastplayedTrack));
        }
        else {
            return std::make_pair("", "");
        }
    }

    std::string adverts_path = "resound\\radio_adverts\\adverts_v\\";
    std::string news_path = "resound\\radio_news\\news_v\\";
    std::string to_path = this->path + "\\to\\";
    std::string id_path = this->path + "\\id\\";
    std::string intro_path = this->path + "\\intro\\";
    std::string solo_path = this->path + "\\solo\\";
    std::string general_path = this->path + "\\general\\";
    std::string time_path = this->path + "\\time\\";

    RadioState currentState = RadioState::None;
    bool stateChanged = false;

    int countAdverts = 0;
    int countMusicTracks = 0;
    int countMusicTracksForNews = 0;
    int introFadeStartMs = 10000;
    int introFadeStartMaxMs = 20000;

    int wholeOutroFadeLength = 5000;
    int quanTracksBeforeNews = 10;
    int quanAdverts = 1;

    unsigned int outroPlayTime = 0;
    unsigned int trackStartTime = CurrentTime();

    bool isDebugging = true;

    bool isMusicTrackNow = false;
    bool isAdvertNow = false;
    bool outro_to_adverts = false;
    bool outro_to_news = false;
    bool outro_to_time = false;
    bool introPassed = false;
    bool newsPlayed = false;

    SoundPlayer* advertsPlayer = nullptr;
    SoundPlayer* advertsTransitionPlayer = nullptr;
    SoundPlayer* idPlayer = nullptr;
    SoundPlayer* introPlayer = nullptr;
    SoundPlayer* outroPlayer = nullptr;
    SoundPlayer* newsPlayer = nullptr;
    SoundPlayer* soloPlayer = nullptr;

    std::vector<double> quanTracksBeforeAdvertsProbabilities = { 0.2, 0.1, 0.3, 0.2, 0.2 };

    void ManagePlayback() {

        while (true) {

            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MAIN_LOOP_MS));

            if (isDebugging) {

                if (Keys::GetKeyJustDown(rsF4)) {
                    if (muted) continue;

                    if (isAdvertNow) {
                        advertsPlayer->pauseTrack();
                    }
                    else {
                        musicPlayer->pauseTrack();
                    }
                }

                if (Keys::GetKeyJustDown(rsF6)) {
                    if (muted) continue;
                }

                if (Keys::GetKeyJustDown(rsF7)) {
                    if (muted) continue;
                    musicPlayer->setPositionPercent(DEBUG_JUMP_PERCENT);
                }
            }

            if (forcePaused) {
                continue;
            }

            int trackPosMs = musicPlayer->getTrackPositionMs();

            if (stateChanged) {
                stateChanged = false;

                ProcessStateChanged();

                continue;
            }

            ProcessState(trackPosMs);
        }
    }

    void Pause() override { RadioStation::Pause(); }
    void Unpause() override { RadioStation::Unpause(); }

    void Randomize() override {
        timeState = "none";
        previoustimeState = "";
        timePlayed = false;
        timeNeedsToBePlayed = false;
        timeSpeeches = "";
        Stop();
        PlayMusicTrack();
        RadioStation::Randomize();
    }

    void Mute() override { RadioStation::Mute(); }
    void Unmute() override { RadioStation::Unmute(); }
    void Stop() override { RadioStation::Stop(); }

    int quanTracksBeforeAdverts = Utils::getRandomIntWithDifferentProbabilities(quanTracksBeforeAdvertsProbabilities);

    void UpdateQuanAdverts() {
        quanAdverts = Utils::getRandomInt(1, 2);
    }

    bool isEveningTime() {
        int hour = Utils::GameHours();
        return hour > EVENING_START_HOUR && hour < EVENING_END_HOUR;
    }

    bool isMorningTime() {
        int hour = Utils::GameHours();
        return hour > MORNING_START_HOUR && hour < MORNING_END_HOUR;
    }

    bool outroNotFound = false;
    bool eveningTimePlayed = false;
    bool morningTimePlayed = false;

    void PlayMusicTrack(const bool playIntro) {
        PlayMusicTrack();
        introPassed = false;
        if (playIntro) {
            trackStartTime = CurrentTime();
            ChangeState(RadioState::PlayMusicTrack);
        }
    }

    void PlayTime(SoundPlayer*& player) {
        std::string thisTime = "";
        std::string timeFile = "";

        if (isEveningTime()) {
            thisTime = "EVENING" + std::to_string(Utils::GameDays()) + ";";
            timeFile = Utils::tryFindRandomFileWithContainedName(KEYWORD_EVENING, time_path);
        }

        if (isMorningTime()) {
            thisTime = "MORNING" + std::to_string(Utils::GameDays()) + ";";
            timeFile = Utils::tryFindRandomFileWithContainedName(KEYWORD_MORNING, time_path);
        }

        timeSpeeches += thisTime;
        if (timeFile == "") {
            outroNotFound = true;
            return;
        }

        player = new SoundPlayer(time_path);
        player->playTrackBASS(timeFile);
        CheckVolumeOnPlay(player);
    }

    void PlayMusicTrack() {
        isMusicTrackNow = true;
        isAdvertNow = false;
        RadioStation::PlayMusicTrack();
        countMusicTracks++;
        countMusicTracksForNews++;
    }

    void PlaySolo() {
        isMusicTrackNow = false;
        isAdvertNow = false;
        ChangeState(RadioState::PlaySolo);
        soloPlayer->playNewTrack();
        CheckVolumeOnPlay(soloPlayer);
    }

    void PlayID() {
        isMusicTrackNow = false;
        isAdvertNow = false;
        idPlayer->playNewTrack();
        CheckVolumeOnPlay(idPlayer);
    }

    void PlayNews() {
        newsPlayed = true;
        isMusicTrackNow = false;
        isAdvertNow = false;
        newsPlayer->playNewTrack();
        CheckVolumeOnPlay(newsPlayer);
    }

protected:

    void ChangeState(RadioState newState) {
        currentState = newState;
        stateChanged = true;
    }

    void ProcessStateChanged() {
        if (currentState == RadioState::PlayMusicTrack) {

            std::this_thread::sleep_for(std::chrono::milliseconds(introFadeStartMs - introFadeLengthMs));

            bool skipIntro = Utils::getRandomBoolWithProbability(INTRO_SKIP_PROBABILITY);

            if (skipIntro) {
                ChangeState(RadioState::TrackMid);
                return;
            }
            else {
                if (isMorningTime() && !morningTimePlayed) {
                    introAndOutroMusicFade->Activate();
                    std::this_thread::sleep_for(std::chrono::milliseconds(introFadeLengthMs));
                    PlayTime(introPlayer);

                    morningTimePlayed = true;
                    eveningTimePlayed = false;
                }
                else if (isEveningTime() && !eveningTimePlayed) {
                    introAndOutroMusicFade->Activate();
                    std::this_thread::sleep_for(std::chrono::milliseconds(introFadeLengthMs));
                    PlayTime(introPlayer);

                    morningTimePlayed = false;
                    eveningTimePlayed = true;
                }
                else {
                    trackname = Utils::tryFindRandomFileWithContainedName(trackname, intro_path);

                    if (trackname == "") {
                        introAndOutroMusicFade->Activate();
                        introPlayer = new SoundPlayer(general_path); // TODO if no general continue
                        std::this_thread::sleep_for(std::chrono::milliseconds(introFadeLengthMs));
                        introPlayer->playNewTrack();
                    }
                    else {
                        introAndOutroMusicFade->Activate();
                        introPlayer = new SoundPlayer(intro_path);
                        std::this_thread::sleep_for(std::chrono::milliseconds(introFadeLengthMs));
                        introPlayer->playTrackBASS(trackname);
                    }
                }

                if (introPlayer->isEmpty) {
                    ChangeState(RadioState::TrackMid);
                    return;
                }

                CheckVolumeOnPlay(introPlayer);
                ChangeState(RadioState::PlayingIntro);
                wholeIntroFadeLength = introPlayer->getTrackLengthMs();
                std::this_thread::sleep_for(std::chrono::milliseconds(wholeIntroFadeLength));
                ChangeState(RadioState::TrackMid);
                return;
            }
        }

        if (currentState == RadioState::TrackMid) {
            if (isMorningTime() && !morningTimePlayed) {
                PlayTime(outroPlayer);
                morningTimePlayed = true;
                eveningTimePlayed = false;
            }
            else if (isEveningTime() && !eveningTimePlayed) {
                PlayTime(outroPlayer);
                morningTimePlayed = false;
                eveningTimePlayed = true;
            }
            else {
                ChangeState(RadioState::WaitingForOutro);

                if (countMusicTracksForNews >= quanTracksBeforeNews) {
                    if (!Utils::DirectoryCheckRelative(to_path)) {
                        ChangeState(RadioState::SkippingOutro);
                        return;
                    }

                    std::string to_newsRandomFile = Utils::tryFindRandomFileWithContainedName(KEYWORD_NEWS, to_path);
                    outroPlayer = new SoundPlayer(to_path);
                    outroPlayer->playTrackBASS(to_newsRandomFile);
                }
                else if (countMusicTracks >= quanTracksBeforeAdverts) {
                    if (!Utils::DirectoryCheckRelative(to_path)) {
                        ChangeState(RadioState::SkippingOutro);
                        return;
                    }

                    std::string to_advertRandomFile = Utils::tryFindRandomFileWithContainedName(KEYWORD_AD, to_path);
                    outroPlayer = new SoundPlayer(to_path);
                    outroPlayer->playTrackBASS(to_advertRandomFile);
                }
                else {
                    bool skipOutro = false;
                    if (!Utils::DirectoryCheckRelative(general_path) || skipOutro) {
                        ChangeState(RadioState::SkippingOutro);
                        return;
                    }
                    outroPlayer = new SoundPlayer(general_path);
                    outroPlayer->playNewTrack();
                }
            }

            outroPlayer->pauseTrack();
            wholeIntroFadeLength = outroPlayer->getTrackLengthMs() + introFadeLengthMs;
            outroPlayTime = musicPlayer->getTrackLengthMs() - MUSIC_FADE_OUT_OFFSET - outroPlayer->getTrackLengthMs();
        }

        if (currentState == RadioState::MusicTrackEnded) {
            if (countMusicTracksForNews >= quanTracksBeforeNews) {
                countMusicTracksForNews = 0;
                newsPlayed = false;
                ChangeState(RadioState::PlayNews);
                return;
            }
            else {
                if (countMusicTracks >= quanTracksBeforeAdverts) {
                    ChangeState(RadioState::PlayAdvert);
                    return;
                }
                else {
                    bool playSolo = Utils::getRandomBoolWithProbability(SOLO_PLAY_PROBABILITY);
                    if (playSolo) {
                        isMusicTrackNow = false;
                        isAdvertNow = false;
                        ChangeState(RadioState::PlaySolo);
                        soloPlayer->playNewTrack();
                        CheckVolumeOnPlay(soloPlayer);
                        return;
                    }
                    else {
                        PlayMusicTrack(true);
                        return;
                    }
                }
            }
        }

        if (currentState == RadioState::PlayAdvert) {
            outro_to_adverts = false;

            if (musicPlayer->isPlayingOrActive()) {
                musicPlayer->stopTrack();
                musicPlayer->eraseChannel();
            }

            isMusicTrackNow = false;
            isAdvertNow = true;
            advertsPlayer->playNewTrack();
            CheckVolumeOnPlay(advertsPlayer);
            countAdverts++;
            return;
        }

        if (currentState == RadioState::AdvertEnded) {
            if (countAdverts <= quanAdverts) {
                ChangeState(RadioState::PlayAdvert);
                return;
            }
            else {
                countMusicTracks = 0;
                quanTracksBeforeAdverts = Utils::getRandomIntWithDifferentProbabilities(quanTracksBeforeAdvertsProbabilities);
                UpdateQuanAdverts();
                ChangeState(RadioState::PlayID);
                return;
            }
        }

        if (currentState == RadioState::PlayID) {
            isMusicTrackNow = false;
            isAdvertNow = false;
            idPlayer->playNewTrack();
            CheckVolumeOnPlay(idPlayer);
            return;
        }
    }

    void ProcessState(int trackPosMs) {

        if (currentState == RadioState::PlayNews && !newsPlayer->isPlayingOrActive()) {
            if (!newsPlayed) {
                PlayNews();
                std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_AFTER_NEWS_MS));
            }
            else {
                ChangeState(RadioState::PlayID);
            }
        }

        if (trackPosMs > (outroPlayTime - introFadeLengthMs) && currentState == RadioState::WaitingForOutro) {
            ChangeState(RadioState::PlayingOutro);
            introAndOutroMusicFade->Activate();
            std::this_thread::sleep_for(std::chrono::milliseconds(introFadeLengthMs));
            outroPlayer->playContinueTrack();
            CheckVolumeOnPlay(outroPlayer);
        }

        if (!musicPlayer->isPlayingOrActive() && isMusicTrackNow) {
            ChangeState(RadioState::MusicTrackEnded);
            isMusicTrackNow = false;
            return;
        }

        if (currentState == RadioState::PlayAdvert && !advertsPlayer->isPlayingOrActive()) {
            musicPlayer->eraseChannel();
            ChangeState(RadioState::AdvertEnded);
            return;
        }

        if (currentState == RadioState::PlayID && !idPlayer->isPlayingOrActive()) {
            musicPlayer->eraseChannel();
            ChangeState(RadioState::MusicTrackEnded);
            return;
        }

        if (currentState == RadioState::PlaySolo && !soloPlayer->isPlayingOrActive()) {
            musicPlayer->eraseChannel();
            ChangeState(RadioState::MusicTrackEnded);
            return;
        }
    }
};