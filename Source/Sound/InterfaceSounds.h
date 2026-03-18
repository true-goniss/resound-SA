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

    static inline const std::string radioInterfaceSoundsPath = "resound\\sounds\\radiosystem\\";

    static HSTREAM Play(const std::string& path, const std::string& soundName, bool looped, bool playIfAlreadyPlaying) {
        auto it = BASS_Streams.find(soundName);
        bool exists = (it != BASS_Streams.end());

        // sound is playing, no need to restart
        if (exists && BASS_ChannelIsActive(it->second) == BASS_ACTIVE_PLAYING && !playIfAlreadyPlaying) {
            return it->second;
        }

        // sound is in the memory, free the stream
        if (exists && it->second != 0) {
            BASS_ChannelStop(it->second);
            BASS_StreamFree(it->second);
            BASS_Streams[soundName] = 0; // in case we load a new one
        }

        // create the new one
        DWORD flags = (looped) ? (BASS_SAMPLE_FLOAT | BASS_SAMPLE_LOOP) : 0;
        HSTREAM newStream = BASS_StreamCreateFile(FALSE, path.c_str(), 0, 0, flags);

        if (newStream == 0) {
            // file not found or corrupted
            return 0;
        }

        // save and run
        BASS_Streams[soundName] = newStream;
        BASS_ChannelSetAttribute(newStream, BASS_ATTRIB_VOL, basicVolume);
        BASS_ChannelPlay(newStream, FALSE);

        return newStream;
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

