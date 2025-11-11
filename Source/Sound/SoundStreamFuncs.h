/*
    by gon_iss (c) 2025

    functions for working with existing BASS stream
*/

#pragma once
#include "bass.h"

static class SoundStreamFuncs
{

public:

    static bool CheckChannel(HSTREAM stream) {
        return stream != 0 
            && stream != NULL;
    }

    static bool Pause(HSTREAM stream) {
        if (!CheckChannel(stream)) 
            return false;

        BASS_ChannelPause(stream);

        return true;
    }

    static bool Stop(HSTREAM stream) {
        if (!(CheckChannel(stream) && stream)) 
            return false;

        BASS_ChannelStop(stream);

        return true;
    }

    static bool PlayContinue(HSTREAM stream) {
        if (!CheckChannel(stream))
            return false;

        BASS_ChannelPlay(stream, false);

        return true;
    }

    static void Rewind(HSTREAM stream, float seconds) {
        float newPosSecs = GetPositionSeconds(stream) - seconds;

        if (newPosSecs < 0)
            newPosSecs = 0;

        QWORD newPosition = BASS_ChannelSeconds2Bytes(stream, newPosSecs);
        BASS_ChannelSetPosition(stream, newPosition, BASS_POS_BYTE);
    }

    static bool SetSpeed(HSTREAM stream, float* channelFrequency, float playbackSpeed) {
        if (!CheckChannel(stream))
            return false;

        BASS_ChannelSetAttribute(stream, BASS_ATTRIB_FREQ, playbackSpeed * (*channelFrequency));
    }

    static bool SetVolume(HSTREAM stream, float volume) {
        if (!CheckChannel(stream)) 
            return false;

        BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, volume);

        return false;
    }

    static float GetVolume(HSTREAM stream) {
        if (!CheckChannel(stream)) 
            return 0;

        float volume;
        BASS_ChannelGetAttribute(stream, BASS_ATTRIB_VOL, &volume);

        return volume;
    }

    static int GetLengthMs(HSTREAM stream) {
        if (!CheckChannel(stream))
            return 0;

        QWORD totalLengthBytes = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
        QWORD totalLengthMS = BASS_ChannelBytes2Seconds(stream, totalLengthBytes) * 1000;

        return totalLengthMS;
    }

    static float GetPositionSeconds(HSTREAM stream) {
        if (!CheckChannel(stream)) 
            return 0;

        QWORD currentPosition = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
        double currentPositionSeconds = BASS_ChannelBytes2Seconds(stream, currentPosition);
        return currentPositionSeconds;
    }

    static float GetPositionPercent(HSTREAM stream) {
        if (!CheckChannel(stream))
            return 0;

        QWORD pos = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
        QWORD length = BASS_ChannelGetLength(stream, BASS_POS_BYTE);

        return (float)pos / (float)length * 100.0;
        
    }

    static bool SetPositionPercent(HSTREAM stream, float& positionPercent) {
        if (!CheckChannel(stream))
            return false;

        QWORD totalLength = BASS_ChannelGetLength(stream, BASS_POS_BYTE);

        // Convert the percentage to bytes
        QWORD newPosition = (QWORD)(positionPercent / 100.0f * totalLength);
        // Set the position of the channel in terms of bytes
        BASS_ChannelSetPosition(stream, newPosition, BASS_POS_BYTE);

        return true;
    }

    static int GetState(HSTREAM stream) {
        if (!CheckChannel(stream)) 
            return 3;

        return BASS_ChannelIsActive(stream);
    }

};