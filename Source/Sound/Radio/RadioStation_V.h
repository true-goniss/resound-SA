#pragma once

/*
    by gon_iss (c) 2024
*/

#include "CAudioEngine.h"
#include "CHud.h"

#include "RadioStation.h"
#include "../SoundPlayer.h"
#include "../../Animations/SoundFade.h"
#include "../../Utils/Keys.h"

using namespace std;

class RadioStation_V : public RadioStation
{

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

    std::string STATE = "none";

    int countAdverts = 0;
    int countMusicTracks = 0;
    int countMusicTracksForNews = 0;
    int introFadeStartMs = 10000;
    int introFadeStartMaxMs = 20000;

    int wholeOutroFadeLength = 5000;
    int quanTracksBeforeNews = 10;
    int quanAdverts = 1;

    unsigned int outroPlayTime = 0;
    unsigned int trackStartTime = CTimer::m_snTimeInMilliseconds;

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

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

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

                    CHud::SetHelpMessage(STATE.c_str(), false, false, false);
                }


                if (Keys::GetKeyJustDown(rsF7)) {
                    if (muted) continue;

                    musicPlayer->setPositionPercent(93);
                }
            }

            if (forcePaused) {
                continue;
            }

            int trackPosMs = musicPlayer->getTrackPositionMs();

            if (STATE_changed) {
                STATE_changed = false;
                //CHud::SetHelpMessage((folder + " s: " + STATE).c_str(), false, false, false);

                

                if (STATE == "playMusicTrack") {

                    std::this_thread::sleep_for(std::chrono::milliseconds(introFadeStartMs - introFadeLengthMs));

                    bool skipIntro = Utils::getRandomBoolWithProbability(0.37f);

                    if (skipIntro) {
                        //std::this_thread::sleep_for(std::chrono::milliseconds((introFadeStartMaxMs - trackPosMs) + 2000));
                        ChangeSTATE("trackMid");
                        continue;
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

                            ChangeSTATE("trackMid");
                            continue;
                        }

                        CheckVolumeOnPlay(introPlayer);

                        ChangeSTATE("playingIntro");

                        wholeIntroFadeLength = introPlayer->getTrackLengthMs();

                        std::this_thread::sleep_for(std::chrono::milliseconds(wholeIntroFadeLength));

                        ChangeSTATE("trackMid");
                        continue;
                    }
                }

                if (STATE == "trackMid") {

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

                        ChangeSTATE("waitingForOutro");

                        if (countMusicTracksForNews >= quanTracksBeforeNews) {

                            if (!Utils::DirectoryCheckRelative(to_path)) {
                                ChangeSTATE("skippingOutro");
                                CHud::SetHelpMessage(("skippingOutro1"), false, false, false);
                                continue;
                            }

                            std::string to_newsRandomFile = Utils::tryFindRandomFileWithContainedName("NEWS", to_path);

                            //std::string pathExists = Utils::DirectoryCheckRelative(to_path) ? "true" : "false";
                            CHud::SetHelpMessage(("to_newsRandomFile:"+to_newsRandomFile).c_str(), false, false, false);

                            outroPlayer = new SoundPlayer(to_path);
                            outroPlayer->playTrackBASS(to_newsRandomFile);
                        }
                        else if (countMusicTracks >= quanTracksBeforeAdverts) {

                            if (!Utils::DirectoryCheckRelative(to_path)) {
                                ChangeSTATE("skippingOutro");
                                CHud::SetHelpMessage(("skippingOutro2"), false, false, false);
                                continue;
                            }

                            std::string to_advertRandomFile = Utils::tryFindRandomFileWithContainedName("AD", to_path);

                            CHud::SetHelpMessage(("to_advertRandomFile:"+to_advertRandomFile).c_str(), false, false, false);

                            outroPlayer = new SoundPlayer(to_path);
                            outroPlayer->playTrackBASS(to_advertRandomFile);
                        }
                        else {

                            bool skipOutro = false;// Utils::getRandomBoolWithProbability(0.27f);

                            std::string generalpathExists = Utils::DirectoryCheckRelative(to_path) ? "true" : "false";

                            CHud::SetHelpMessage(("generalpathExists" + generalpathExists).c_str(), false, false, false);

                            if (!Utils::DirectoryCheckRelative(general_path) || skipOutro) {
                                ChangeSTATE("skippingOutro");
                                //CHud::SetHelpMessage(("skippingOutro3"), false, false, false);
                                continue;
                            }
                            outroPlayer = new SoundPlayer(general_path);
                            outroPlayer->playNewTrack();
                            CHud::SetHelpMessage(("playing general file:"), false, false, false);
                        }
                    }

                    outroPlayer->pauseTrack();
                    wholeIntroFadeLength = outroPlayer->getTrackLengthMs() + introFadeLengthMs;
                    outroPlayTime = musicPlayer->getTrackLengthMs() - 4000 - outroPlayer->getTrackLengthMs();


                }

                if (STATE == "musicTrackEnded") {

                    if (countMusicTracksForNews >= quanTracksBeforeNews) {
                        countMusicTracksForNews = 0;
                        newsPlayed = false;
                        ChangeSTATE("playNews");
                        continue;
                    }
                    else {
                        if (countMusicTracks >= quanTracksBeforeAdverts) {
                            ChangeSTATE("playAdvert");
                            continue;
                        }
                        else {
                            bool playSolo = Utils::getRandomBoolWithProbability(0.2f);

                            if (playSolo)
                            {
                                isMusicTrackNow = false;
                                isAdvertNow = false;
                                ChangeSTATE("playSolo");
                                soloPlayer->playNewTrack();
                                CheckVolumeOnPlay(soloPlayer);
                                continue;
                            }
                            else {
                                PlayMusicTrack(true);
                                continue;
                            }
                        }
                    }
                }

                if (STATE == "playAdvert") {
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
                    continue;
                }

                if (STATE == "advertEnded") {
                    if (countAdverts <= quanAdverts) {
                        ChangeSTATE("playAdvert");
                        continue;
                    }
                    else {
                        countMusicTracks = 0;
                        quanTracksBeforeAdverts = Utils::getRandomIntWithDifferentProbabilities(quanTracksBeforeAdvertsProbabilities);
                        UpdateQuanAdverts();           
                        ChangeSTATE("playID");
                        continue;
                    }
                }

                if (STATE == "playID") {
                    isMusicTrackNow = false;
                    isAdvertNow = false;

                    idPlayer->playNewTrack();
                    CheckVolumeOnPlay(idPlayer);
                    continue;
                }

                continue;
            }

            if (STATE == "playNews" && !newsPlayer->isPlayingOrActive()) {

                if (!newsPlayed) {
                    PlayNews();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
                else {
                    ChangeSTATE("playID");
                }

            }

            if (trackPosMs > (outroPlayTime - introFadeLengthMs) && STATE == "waitingForOutro") {

                ChangeSTATE("playingOutro");

                introAndOutroMusicFade->Activate();

                std::this_thread::sleep_for(std::chrono::milliseconds(introFadeLengthMs));

                outroPlayer->playContinueTrack();
                CheckVolumeOnPlay(outroPlayer);
            }

            //if (STATE == "playingOutro" && trackPosMs > (outroPlayTime + outroPlayer->getTrackLengthMs())) {
            //    ChangeSTATE("outroEnded");
            //}


            if (!musicPlayer->isPlayingOrActive() && isMusicTrackNow) {
                ChangeSTATE("musicTrackEnded");
                isMusicTrackNow = false;
                continue;
            }

            if (STATE == "playAdvert" && !advertsPlayer->isPlayingOrActive()) {
                musicPlayer->eraseChannel();
                ChangeSTATE("advertEnded");
                continue;
            }

            if (STATE == "playID" && !idPlayer->isPlayingOrActive()) {
                musicPlayer->eraseChannel();
                ChangeSTATE("musicTrackEnded");
                continue;
            }

            if (STATE == "playSolo" && !soloPlayer->isPlayingOrActive()) {
                musicPlayer->eraseChannel();
                ChangeSTATE("musicTrackEnded");
                continue;
            }
        }
    }

    void Pause() override {
        RadioStation::Pause();
    }

    void Unpause() override {
        RadioStation::Unpause();
    }

    void Randomize() override {
        timeState = "none";
        previoustimeState = "";
        timePlayed = false;
        timeNeedsToBePlayed = false;

        //TODO reset timespeeches every day

        timeSpeeches = "";

        Stop();
        // TODO randomize between advert or track
        PlayMusicTrack();
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

    int quanTracksBeforeAdverts = Utils::getRandomIntWithDifferentProbabilities(quanTracksBeforeAdvertsProbabilities);

    void UpdateQuanAdverts() {
        quanAdverts = Utils::getRandomInt(1, 2);
    }

    bool isEveningTime() {
        return (Utils::GameHours() > 19 && Utils::GameHours() < 23);
    }

    bool isMorningTime() {
        return (Utils::GameHours() > 5 && Utils::GameHours() < 9);
    }

    bool outroNotFound = false;



    bool STATE_changed = false;
    bool eveningTimePlayed = false;
    bool morningTimePlayed = false;

    void ChangeSTATE(const std::string& state) {
        STATE = state;
        STATE_changed = true;
    }

    void PlayMusicTrack(const bool playIntro) {

        PlayMusicTrack();

        introPassed = false;

        if (playIntro) {
            trackStartTime = CTimer::m_snTimeInMilliseconds;
            ChangeSTATE("playMusicTrack");
        }
    }

    void PlayTime(SoundPlayer*& player) {

        std::string thisTime = "";
        std::string timeFile = "";

        if (isEveningTime()) {
            thisTime = "EVENING" + std::to_string(Utils::GameDays()) + ";";
            timeFile = Utils::tryFindRandomFileWithContainedName("EVENING", time_path);
        }

        if (isMorningTime()) {
            thisTime = "MORNING" + std::to_string(Utils::GameDays()) + ";";
            timeFile = Utils::tryFindRandomFileWithContainedName("MORNING", time_path);
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
        ChangeSTATE("playSolo");
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
};