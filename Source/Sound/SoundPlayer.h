/*
    by gon_iss (c) 2024
*/

#pragma once
#include "bass.h"
#include <string>
#include <vector>
#include <random>

#include <game_sa/RenderWare.h>
#include "Utils/Utils.h"
#include "../Visual/TrackInfoVisual.h"
#include "SoundStreamFuncs.h"

using namespace std;

class SoundPlayer
{

    std::function<void(const std::string&)> onNewTrackPlayedCallback = nullptr;

    bool initializedBASS = false;
    bool looped = false;
    bool wasActiveBeforePause = false;
    bool randomizeTrackPosition = false;

    unsigned int timeLastRewind = CurrentTime();

    float freqInitVal = 44100.0f;
    float* channelFrequency = &freqInitVal;
    float currentVolume = 0.0f;

    HSTREAM stream = 0;

public:

    bool isEmpty = false;
    bool isRewindNow = false;
    bool trackIsPlaying = false;
    

    float rewindSpeed = 1.0f;
    float playbackSpeed = 1.0f;

    std::string folder = "";
    std::vector<string> playedTracksNames;
    std::string lastplayedTrack = "";

    int indexOfPlayingTrack = 999999;

    // Usual soundplayer constructor, not looped by default
    SoundPlayer(std::string folder) : SoundPlayer(folder, false) {}

    SoundPlayer(std::string folder, bool looped) {
        if (folder == "" || !Utils::DirectoryCheckRelative(folder)) {
            isEmpty = true;
            return;
        }

        this->folder = Utils::GetCurrentDirectory() + "\\" + folder;
        this->looped = looped;
    }


    void playTrackBASS(const std::string& musicfile) {

        if (musicfile == "" || isEmpty) return;

        if (!initializedBASS) {
            initializedBASS = true;
            stream = NULL;
            BASS_Init(-1, 44100, 0, RsGlobal.ps->window, nullptr);
        }

        std::string filePath = folder + "//" + musicfile;

        if (isEmpty || !Utils::FileCheck(filePath.c_str())) {
            stream = 0;
            return;
        }

        BASS_ChannelStop(stream);

        if (looped) {
            stream = BASS_StreamCreateFile(FALSE, (filePath).c_str(), 0, 0, BASS_SAMPLE_FLOAT | BASS_SAMPLE_LOOP);
        }
        else {
            stream = BASS_StreamCreateFile(FALSE, (filePath).c_str(), 0, 0, 0);
        }

        lastplayedTrack = musicfile;

        if (stream != 0) {

            if(onNewTrackPlayedCallback)
                onNewTrackPlayedCallback(musicfile);

            BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, currentVolume);
            BASS_ChannelPlay(stream, FALSE);
            
            trackIsPlaying = true;

            if (randomizeTrackPosition) {
                randomizeTrackPosition = false;
                randomizePosition();
            }

            BASS_ChannelGetAttribute(stream, BASS_ATTRIB_FREQ, channelFrequency);
        }
        else {
            // BASS_Free();
            // std::to_string( BASS_ErrorGetCode() )
        }
    }

    void setOnNewTrackPlayed(std::function<void(const std::string&)> callback) {
        onNewTrackPlayedCallback = callback;
    }

    float percentPlayback() {
        
        if ( playingStateIsStopped() ) return 0; //!hasPlayedAnyTrack() ||

        float perc = SoundStreamFuncs::GetPositionPercent(stream);

        return perc;
    }

    bool playingStateIsActive() {
        return getTrackState() == 1;
    }

    bool playingStateIsPaused() {
        int state = getTrackState();
        return state == 2 || state == 3;
    }

    bool playingStateIsStopped() {
        return getTrackState() == 0;
    }

    bool playingStateIsPlaying() {
        return getTrackState() == 4;
    }

    bool isPlayingOrActive() {
        return playingStateIsActive() || playingStateIsPlaying();
    }

    bool isActive() {
        if (isEmpty) return false;

        return playingStateIsActive() || playingStateIsPaused() || playingStateIsPlaying();
    }

    bool hasPlayedAnyTrack() {
        return playedTracksNames.size() != 0;
    }

    void playNextTrack() {
        if (isEmpty) return;

        playbackSpeed = 1;

        if (playedTracksNames.size() == 0)
        {
            indexOfPlayingTrack = 0;
            playNewTrack(); // new track into playlist
            return;
        }


        // track not last in playlist, playing next track
        if (indexOfPlayingTrack < (playedTracksNames.size() - 1)) {
            indexOfPlayingTrack++;
            string musicfile = playedTracksNames[indexOfPlayingTrack];
            playTrackBASS(musicfile);
        }
        else {
            playNewTrack(); // new track into playlist
        }
    }

    void pauseTrack() {
        if (!SoundStreamFuncs::Pause(stream)) 
            return;

        if (isActive()) {
            wasActiveBeforePause = true;
        }

        trackIsPlaying = false;
    }

    void stopTrack() {
        if (!SoundStreamFuncs::Stop(stream))
            return;

        trackIsPlaying = false;
    }

    void eraseChannel() {
        stream = NULL;
    }

    void playContinueTrack() {
        if (!SoundStreamFuncs::PlayContinue(stream) || !wasActiveBeforePause) 
            return;

        isRewindNow = false;
        wasActiveBeforePause = false;

        playbackSpeed = 1.0f;
        trackIsPlaying = true;
    }

    int getTrackState() {
        return SoundStreamFuncs::GetState(stream);
    }

    int getTrackPositionMs() {
        double seconds = getCurrentPositionSeconds();
        int positionMs = static_cast<int>(seconds * 1000);

        return positionMs;
    }

    int getTrackLengthMs() {
        return SoundStreamFuncs::GetLengthMs(stream);
    }

    std::string playNewTrack() {
        if (isEmpty) return "";

        string musicfile = "";
        musicfile = Utils::pick_random_music_file(folder);

        bool playedRecently = trackWasPlayedRecently(10, musicfile);

        int countTries = 0;

        while (playedRecently)
        {
            musicfile = Utils::pick_random_music_file(folder);
            playedRecently = trackWasPlayedRecently(10, musicfile);
            countTries++;
            if (countTries > 15) break;
        }

        if (playedTracksNames.size() > 10) playedTracksNames[0].erase();

        playedTracksNames.push_back(musicfile);
        indexOfPlayingTrack++;

        std::string playedRecentlyStr = (playedRecently) ? "true" : "false";

        playTrackBASS(musicfile);

        return musicfile;
    }

    void randomizePosition() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 100 - 1);
        float positionPercent = (float)dis(gen);

        setPositionPercent(positionPercent);
    }

    void setPositionPercent(float positionPercent) {
        SoundStreamFuncs::SetPositionPercent(stream, positionPercent);
    }

    float getCurrentPositionSeconds() {
        return SoundStreamFuncs::GetPositionSeconds(stream);
    }

    void speedFastForward() {

        isRewindNow = false;

        if (playbackSpeed < 6.5) playbackSpeed += 0.1f;

        SoundStreamFuncs::SetSpeed(stream, channelFrequency, playbackSpeed);
    }

    void speedNormal() {

        isRewindNow = false;
        rewindSpeed = 1.0f;
        playbackSpeed = 1.0f;

        SoundStreamFuncs::SetSpeed(stream, channelFrequency, playbackSpeed);
    }

    void setVolume(float volume) {
        if (!SoundStreamFuncs::SetVolume(stream, volume))
            return;

        this->currentVolume = volume;
    }

    void setVolumeMute() {

        float volume = SoundStreamFuncs::GetVolume(stream);

        if (volume != 0) {
            //volumeBeforeMute = volume;
            setVolume(0);
        }
    }

    void setVolumeUnmute(float basicVolume) {

        float volume = SoundStreamFuncs::GetVolume(stream);

        if (volume == 0) {
            setVolume(basicVolume);
        }
    }

    void speedRewind() {
        if(rewindSpeed < 6.5f) rewindSpeed += 0.1;

        if (CurrentTime() < timeLastRewind + 300) return;
        
        timeLastRewind = CurrentTime();

        pauseTrack();
        rewindByASecond();
    }

    void rewindByASecond() {
        isRewindNow = true;
        SoundStreamFuncs::Rewind(stream, 1);
    }

    bool trackWasPlayedRecently(int lastTracks, string filename) {
        bool containsTarget = false;
        if (isEmpty) return containsTarget;

        for (int i = 0; i < playedTracksNames.size(); ++i) {
            if (playedTracksNames[i] == filename) {
                containsTarget = true;
                break;
            }
        }

        return containsTarget;
    }

    void playPreviousTrack(bool goToTheLastAfterFirst) {
        if (playedTracksNames.size() == 0)
        {
            return;
        }

        indexOfPlayingTrack--;

        if (goToTheLastAfterFirst) {
            if (indexOfPlayingTrack < 0) indexOfPlayingTrack = playedTracksNames.size() - 1;
        }
        else {
            if (indexOfPlayingTrack < 0) indexOfPlayingTrack = 0;
        }

        string musicfile = playedTracksNames[indexOfPlayingTrack];
        playTrackBASS(musicfile);
    }
};