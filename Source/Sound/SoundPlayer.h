/*
    by gon_iss (c) 2024
*/

#pragma once
#include "bass.h"
#include <string>
#include <vector>
#include <random>
#include <map>
#include <sstream>
#include <functional>

#include <game_sa/RenderWare.h>
#include "../Utils/Utils.h"
#include "../Utils/pragmascope/pragmascope_client.h"
#include "../Visual/TrackInfoVisual.h"
#include "SoundStreamFuncs.h"

using namespace std;

// #define ENABLE_LOGGING
// #define DISABLE_TRACK_CACHE

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
#ifdef ENABLE_LOGGING
    PragmaScope* debugScope1;
#endif
    static bool bassInitialized;

#ifndef DISABLE_TRACK_CACHE
    std::map<std::string, uint32_t> trackLengthCache;
#endif

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

    SoundPlayer(std::string folder) : SoundPlayer(folder, false) {}

    SoundPlayer(std::string folder, bool looped) {
        SetFolder(folder);
        this->looped = looped;
#ifdef ENABLE_LOGGING
        debugScope1 = new PragmaScope("SoundPlayer", "RadioLogic1");
        LOG("Constructor", "SoundPlayer");
#endif
    }

    ~SoundPlayer() {
        if (stream) {
            BASS_ChannelStop(stream);
            BASS_StreamFree(stream);
        }
#ifdef ENABLE_LOGGING
        delete debugScope1;
#endif
    }

    static void FreeBASSGlobal() {
        BASS_Free();
        bassInitialized = false;
    }

    void SetFolder(std::string folder) {
        if (folder == "" || !Utils::DirectoryCheckRelative(folder)) {
            isEmpty = true;
            return;
        }
        this->folder = Utils::GetCurrentDirectory() + "\\" + folder;
    }

    std::string GetDebugInfo() const {
        std::stringstream ss;
        ss << "SoundPlayer@" << static_cast<const void*>(this)
            << " [stream:" << stream
            << ", folder:" << folder << "]";
        return ss.str();
    }

#ifdef ENABLE_LOGGING
    void Log(string key, string value) {
        std::ostringstream jsonPayload;
        jsonPayload << "{";
        jsonPayload << "\"value\":\"" << debugScope1->escapeJson(value) << "\"";
        jsonPayload << "}";
        if (debugScope1) {
            debugScope1->info(key, jsonPayload.str());
        }
    }
#define LOG(key, value) Log(key, value)
#else
#define LOG(key, value) ((void)0)
#endif

    void playTrackBASS(const std::string& musicfile, bool useAbsolutePath) {
        LOG("playTrackBASS", folder);

        std::string filePath = useAbsolutePath ? musicfile : folder + "//" + musicfile;
        LOG("playTrackBASS", filePath);
        LOG("playTrackBASS player isempty", std::to_string(isEmpty));

        if (filePath == "" || isEmpty) return;

        if (BASS_GetDevice() == -1) {
            if (!BASS_Init(-1, 44100, 0, RsGlobal.ps->window, nullptr)) {
                int initError = BASS_ErrorGetCode();
                LOG("BASS_Init_Error", "Failed to initialize BASS: " + std::to_string(initError));
                return;
            }
            initializedBASS = true;
        }

        if (!Utils::FileCheck(filePath.c_str())) {
            LOG("FileCheck_Failed", "File not found: " + filePath);
            stream = 0;
            return;
        }

        if (stream) {
            BASS_ChannelStop(stream);
            BASS_StreamFree(stream);
            stream = 0;
        }

        if (looped) {
            stream = BASS_StreamCreateFile(FALSE, filePath.c_str(), 0, 0,
                BASS_SAMPLE_FLOAT | BASS_SAMPLE_LOOP | BASS_ASYNCFILE);
        }
        else {
            stream = BASS_StreamCreateFile(FALSE, filePath.c_str(), 0, 0, BASS_ASYNCFILE);
        }

        if (stream == 0) {
            int streamError = BASS_ErrorGetCode();
            LOG("BASS_Stream_Error", "Failed to create stream for: " + filePath + " Error: " + std::to_string(streamError));
            return;
        }

        lastplayedTrack = Utils::GetFileNameFromPath(filePath);

        BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, currentVolume);

        if (!BASS_ChannelPlay(stream, FALSE)) {
            int playError = BASS_ErrorGetCode();
            LOG("BASS_Play_Error", "Failed to play stream: " + std::to_string(playError));
            return;
        }

        trackIsPlaying = true;

        if (randomizeTrackPosition) {
            randomizeTrackPosition = false;
            randomizePosition();
        }

        BASS_ChannelGetAttribute(stream, BASS_ATTRIB_FREQ, channelFrequency);

        LOG("BASS_Success", "Stream created and playing: " + filePath + " | Stream: " + std::to_string((DWORD)stream));
    }

    void playTrackBASS(const std::string& musicfile) {
        playTrackBASS(musicfile, false);
    }

    void setOnNewTrackPlayed(std::function<void(const std::string&)> callback) {
        onNewTrackPlayedCallback = callback;
    }

    float percentPlayback() {
        if (playingStateIsStopped()) return 0;
        return SoundStreamFuncs::GetPositionPercent(stream);
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
        return !playedTracksNames.empty();
    }

    void playNextTrack() {
        if (isEmpty) return;

        playbackSpeed = 1;

        if (playedTracksNames.empty()) {
            indexOfPlayingTrack = 0;
            playNewTrack();
            return;
        }


        // track not last in playlist, playing next track
        if (indexOfPlayingTrack < (playedTracksNames.size() - 1)) {
            indexOfPlayingTrack++;
            string musicfile = playedTracksNames[indexOfPlayingTrack];
            playTrackBASS(musicfile);
        }
        else {
            playNewTrack();
        }
    }

    void pauseTrack() {
        if (!SoundStreamFuncs::Pause(stream)) return;

        if (isActive()) {
            wasActiveBeforePause = true;
        }
        trackIsPlaying = false;
    }

    void stopTrack() {
        if (!SoundStreamFuncs::Stop(stream)) return;
        trackIsPlaying = false;
    }

    void eraseChannel() {
        stream = NULL;
    }

    void playContinueTrack() {
        if (!SoundStreamFuncs::PlayContinue(stream) || !wasActiveBeforePause) return;

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
        return static_cast<int>(seconds * 1000);
    }

    void setTrackPositionMs(uint32_t ms) {
        double seconds = ms / 1000.0;
        QWORD pos = BASS_ChannelSeconds2Bytes(stream, seconds);
        BASS_ChannelSetPosition(stream, pos, BASS_POS_BYTE);
    }

    uint32_t getTrackLengthMs(string path) {
#ifndef DISABLE_TRACK_CACHE
        auto it = trackLengthCache.find(path);
        if (it != trackLengthCache.end()) {
            return it->second;
        }
#endif

        HSTREAM temp = BASS_StreamCreateFile(FALSE, path.c_str(), 0, 0, BASS_STREAM_DECODE);
        if (!temp) return 0;

        double secs = BASS_ChannelBytes2Seconds(temp, BASS_ChannelGetLength(temp, BASS_POS_BYTE));
        BASS_StreamFree(temp);

        uint32_t lengthMs = static_cast<uint32_t>(secs * 1000);

#ifndef DISABLE_TRACK_CACHE
        trackLengthCache[path] = lengthMs;
#endif

        return lengthMs;
    }

    int getTrackLengthMs() {
        return SoundStreamFuncs::GetLengthMs(stream);
    }

    std::string playNewTrack() {
        if (isEmpty) return "";

        string musicfile = Utils::pick_random_music_file(folder);
        bool playedRecently = trackWasPlayedRecently(10, musicfile);

        int countTries = 0;
        while (playedRecently && countTries <= 15) {
            musicfile = Utils::pick_random_music_file(folder);
            playedRecently = trackWasPlayedRecently(10, musicfile);
            countTries++;
        }

        if (playedTracksNames.size() > 10) {
            playedTracksNames.erase(playedTracksNames.begin());
        }

        playedTracksNames.push_back(musicfile);
        indexOfPlayingTrack++;

        playTrackBASS(musicfile);
        return musicfile;
    }

    void randomizePosition() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 99);
        float positionPercent = static_cast<float>(dis(gen));
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
        if (playbackSpeed < 6.5f) playbackSpeed += 0.1f;
        SoundStreamFuncs::SetSpeed(stream, channelFrequency, playbackSpeed);
    }

    void speedNormal() {
        isRewindNow = false;
        rewindSpeed = 1.0f;
        playbackSpeed = 1.0f;
        SoundStreamFuncs::SetSpeed(stream, channelFrequency, playbackSpeed);
    }

    void setVolume(float volume) {
        if (!SoundStreamFuncs::SetVolume(stream, volume)) return;
        currentVolume = volume;
    }

    void setVolumeMute() {
        float volume = SoundStreamFuncs::GetVolume(stream);
        if (volume != 0) {
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
        if (rewindSpeed < 6.5f) rewindSpeed += 0.1f;
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
        if (isEmpty) return false;
        for (const auto& name : playedTracksNames) {
            if (name == filename) return true;
        }
        return false;
    }

    void playPreviousTrack(bool goToTheLastAfterFirst) {
        if (playedTracksNames.empty()) return;

        indexOfPlayingTrack--;

        if (goToTheLastAfterFirst) {
            if (indexOfPlayingTrack < 0)
                indexOfPlayingTrack = playedTracksNames.size() - 1;
        }
        else {
            if (indexOfPlayingTrack < 0)
                indexOfPlayingTrack = 0;
        }

        string musicfile = playedTracksNames[indexOfPlayingTrack];
        playTrackBASS(musicfile);
    }
};