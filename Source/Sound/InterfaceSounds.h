#pragma once
#include <map>
#include <bass.h>
#include <string>

static class InterfaceSounds
{
    static bool soundWasPlayed(std::string name) {
        auto it = BASS_Streams.find(name);
        return it != BASS_Streams.end();
    }

public:

    static inline float basicVolume = 0.23f;

    static inline std::map<std::string, HSTREAM> BASS_Streams;

    static HSTREAM Play(const std::string& path, const std::string& soundName, bool looped, bool playIfAlreadyPlaying) {

        if (soundWasPlayed(soundName) && BASS_ChannelIsActive(BASS_Streams[soundName]) && !playIfAlreadyPlaying) {
            return 0;
        }

        if (looped) {
            BASS_Streams[soundName] = BASS_StreamCreateFile(FALSE, path.c_str(), 0, 0, BASS_SAMPLE_FLOAT | BASS_SAMPLE_LOOP);
        }
        else {
            BASS_Streams[soundName] = BASS_StreamCreateFile(FALSE, path.c_str(), 0, 0, 0);
        }

        if (BASS_Streams[soundName] == 0) {
            BASS_Free();
            return 0;
        }

        BASS_ChannelPlay(BASS_Streams[soundName], FALSE);
        SetVolume(soundName, basicVolume);

        return BASS_Streams[soundName];
    }

    //static void randomize(const std::string& soundName) {
    //    if(BASS_Streams[soundName]) 
    //}

    static void SetVolume(const std::string& soundName, const float volume) {
        if (BASS_Streams[soundName]) BASS_ChannelSetAttribute(BASS_Streams[soundName], BASS_ATTRIB_VOL, volume);
    }


    static bool Stop(const std::string& soundName) {

        auto it = BASS_Streams.find(soundName);
        if (it != BASS_Streams.end()) {
            BASS_ChannelStop(it->second);
            return true;
        }

        return false;
    }
};

