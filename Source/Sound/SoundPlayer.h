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

using namespace std;

class SoundPlayer
{
    bool initializedBASS = false;
    bool looped = false;
    bool wasActiveBeforePause = false;
    DWORD bassChannel = NULL;
    HSTREAM stream = 0;
    bool randomizeTrackPosition = false;
    unsigned int timeLastRewind = CTimer::m_snTimeInMilliseconds;
    float freqInitVal = 44100.0f;
    float* channelFrequency = &freqInitVal;

    float currentVolume = 0.0f;

    bool CheckChannel() {
        if (isEmpty) return false;

        return stream != 0 && bassChannel != NULL;
    }

public:

    bool isEmpty = false;
    std::string folder = "";
    bool showTrackInfoOnNewTrack = false;
    bool isRewindNow = false;
    float rewindSpeed = 1.0f;

    SoundPlayer(std::string folder) {
        if (folder == "" || !Utils::DirectoryCheckRelative(folder)) { isEmpty = true; return; }

        this->folder = Utils::GetCurrentDirectory() + "\\" + folder;
    }

    SoundPlayer(std::string folder, bool looped) {
        if (folder == "" || !Utils::DirectoryCheckRelative(folder)) { isEmpty = true; return; }

        this->folder = Utils::GetCurrentDirectory() + "\\" + folder;
        this->looped = looped;
    }

    std::vector<string> playedTracksNames;
    int indexOfPlayingTrack = 999999;

    float playbackSpeed = 1.0f;
    bool trackIsPlaying = false;
    std::string lastplayedTrack = "";

    void playTrackBASS(const std::string& musicfile) {

        if (musicfile == "" || isEmpty) return;

        if (!initializedBASS) {
            initializedBASS = true;
            bassChannel = NULL;
            BASS_Init(-1, 44100, 0, RsGlobal.ps->window, nullptr);
        }

        std::string filePath = folder + "//" + musicfile;

        if (!Utils::FileCheck(filePath.c_str())) {
            stream = 0;
            return;
        }

        BASS_ChannelStop(bassChannel);

        if (looped) {
            stream = BASS_StreamCreateFile(FALSE, (filePath).c_str(), 0, 0, BASS_SAMPLE_FLOAT | BASS_SAMPLE_LOOP);
        }
        else {
            stream = BASS_StreamCreateFile(FALSE, (filePath).c_str(), 0, 0, 0);
        }

        lastplayedTrack = musicfile;

        if (stream != 0) {

            std::string rawFilename = Utils::remove_music_extension(musicfile);
            std::pair artistAndName = Utils::GetTrackArtistAndName(rawFilename);
            if (showTrackInfoOnNewTrack) TrackInfoVisual::ShowWithAnimation(artistAndName.first, artistAndName.second);

            BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, currentVolume);

            bassChannel = stream;
            BASS_ChannelPlay(bassChannel, FALSE);
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

    float percentPlayback() {
        try
        {
            QWORD pos = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
            QWORD length = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
            double seconds = BASS_ChannelBytes2Seconds(stream, pos);

            if (!hasPlayedAnyTrack() || playingStateIsStopped()) return 0;

            return (float)pos / (float)length * 100.0;
        }
        catch (const std::exception& ex)
        {
            return 0;
        }
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
        if (!CheckChannel()) return;

        if (isActive()) {
            wasActiveBeforePause = true;
        }

        BASS_ChannelPause(bassChannel);
        trackIsPlaying = false;
    }

    void stopTrack() {
        if (!CheckChannel()) return;

        if (bassChannel) BASS_ChannelStop(bassChannel);
        trackIsPlaying = false;
    }

    void eraseChannel() {
        bassChannel = NULL;
    }

    void playContinueTrack() {
        if (!CheckChannel() || !wasActiveBeforePause) return;


        isRewindNow = false;
        wasActiveBeforePause = false;

        playbackSpeed = 1.0f;
        BASS_ChannelPlay(bassChannel, false);
        trackIsPlaying = true;
    }

    int getTrackState() {
        if (!CheckChannel()) return 3;

        return BASS_ChannelIsActive(bassChannel);
    }

    int getTrackPositionMs() {
        if (!CheckChannel()) return 0;

        double seconds = getCurrentPositionSeconds();
        int positionMs = static_cast<int>(seconds * 1000);

        return positionMs;
    }

    int getTrackLengthMs() {
        if (!CheckChannel()) return 0;

        QWORD totalLengthBytes = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
        QWORD totalLengthMS = BASS_ChannelBytes2Seconds(stream, totalLengthBytes) * 1000;

        return totalLengthMS;
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
        if (!CheckChannel()) return;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 100 - 1);
        float positionPercent = (float)dis(gen);

        setPositionPercent(positionPercent);
    }

    void setPositionPercent(float positionPercent) {
        QWORD totalLength = BASS_ChannelGetLength(stream, BASS_POS_BYTE);

        // Convert the percentage to bytes
        QWORD newPosition = (QWORD)(positionPercent / 100.0f * totalLength);
        // Set the position of the channel in terms of bytes
        BASS_ChannelSetPosition(stream, newPosition, BASS_POS_BYTE);
    }

    float getCurrentPositionSeconds() {
        if (!CheckChannel()) return 0;

        QWORD currentPosition = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
        double currentPositionSeconds = BASS_ChannelBytes2Seconds(stream, currentPosition);
        return currentPositionSeconds;
    }

    float getPositionPercent() {
        double currentPositionSeconds = getCurrentPositionSeconds();

        QWORD totalLengthBytes = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
        double totalLengthSeconds = BASS_ChannelBytes2Seconds(stream, totalLengthBytes);

        return (float)currentPositionSeconds / (float)totalLengthSeconds * 100.0f;
    }

    int GetChannelDurationMS(HSTREAM stream) {
        if (isEmpty) return 0;

        QWORD totalLengthBytes = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
        double totalLengthSeconds = BASS_ChannelBytes2Seconds(stream, totalLengthBytes);
        int durationMS = (int)(totalLengthSeconds * 1000);

        return durationMS;
    }

    void speedFastForward() {
        if (!CheckChannel()) return;

        isRewindNow = false;

        if (playbackSpeed < 6.5) playbackSpeed += 0.1f;

        if (stream != 0) BASS_ChannelSetAttribute(stream, BASS_ATTRIB_FREQ, playbackSpeed * (*channelFrequency));
    }

    void speedNormal() {
        if (!CheckChannel()) return;

        isRewindNow = false;
        rewindSpeed = 1.0f;
        playbackSpeed = 1.0f;
        if (stream != 0) BASS_ChannelSetAttribute(stream, BASS_ATTRIB_FREQ, playbackSpeed * (*channelFrequency));
    }

    void setVolume(float volume) {
        if (!CheckChannel()) return;

        this->currentVolume = volume;

        BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, volume);
    }

    void setVolumeMute() {
        if (!CheckChannel()) return;

        float volume;
        BASS_ChannelGetAttribute(stream, BASS_ATTRIB_VOL, &volume);

        if (volume != 0) {
            //volumeBeforeMute = volume;
            setVolume(0);
        }
    }

    void setVolumeUnmute(float basicVolume) {
        if (!CheckChannel()) return;

        float volume;
        BASS_ChannelGetAttribute(stream, BASS_ATTRIB_VOL, &volume);

        if (volume == 0) {
            setVolume(basicVolume);
        }
    }



    void speedRewind() {
        if(rewindSpeed < 6.5f) rewindSpeed += 0.1;

        if (CTimer::m_snTimeInMilliseconds < timeLastRewind + 300) return;
        
        timeLastRewind = CTimer::m_snTimeInMilliseconds; 

        pauseTrack();
        rewindByASecond();
    }

    void rewindByASecond() {
        isRewindNow = true;
        QWORD newPosition = BASS_ChannelSeconds2Bytes(stream, getCurrentPositionSeconds() - 1);
        BASS_ChannelSetPosition(stream, newPosition, BASS_POS_BYTE);
    }

    //void speedRewind() {
    //    if (!CheckChannel()) return;

    //    float currentPositionPercent = getPositionPercent();

    //    float newPositionPercent = currentPositionPercent - 0.5; 
    //    if (newPositionPercent < 0) {
    //        newPositionPercent = 0;
    //    }

    //    setPositionPercent(newPositionPercent);
    //}

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