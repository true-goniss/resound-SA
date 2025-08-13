
#pragma once
#include "CPed.h"
#include "plugin.h"
#include <game_sa/CTimer.h>
#include "extensions/ScriptCommands.h"

#include "Utils/Utils.h"
#include "Utils/GamePausedWatcher.h"

#include "Sound/SoundPlayer.h"
#include "Sound/InterfaceSounds.h"
#include "Visual/Textures/CassettePlayer/CassetteInterfaceTextures.h"

#include <string>
#include <thread>
#include <chrono>

using namespace plugin;
using namespace std;

class CassettePlayer_Music
{

    float basicVolume = 0.23f;
    float fastfwdVolume = 0.1f;
    
    bool pausedByGame = false;

public:

    SoundPlayer* soundPlayer = new SoundPlayer("resound//music");
    unsigned int timeLastKeypress = CurrentTime();

    CassettePlayer_Music() {
        soundPlayer->setVolume(basicVolume);

        std::thread playerMusicThread(&CassettePlayer_Music::ManagePlayback, this);
        playerMusicThread.detach();

        Events::onResumeAllSounds += [this] {
            AdjustVolumeValueFromSettings();
            soundPlayer->setVolume(basicVolume);
        };

        GamePausedWatcher::AddHandler([this](GamePausedWatcher::EventType eventType) {
            switch (eventType)
            {
                case GamePausedWatcher::EventType::Paused:

                    if (soundPlayer->isPlayingOrActive()) {
                        PauseTrack(false);
                        pausedByGame = true;
                    }

                    break;

                case GamePausedWatcher::EventType::Unpaused:

                    if (pausedByGame) {
                        pausedByGame = false;
                        PlayContinueTrack(false);
                    }
                        
                    break;
                }
        });
    }

    void PauseTrack(bool playSoundClick) {
        CassetteInterfaceTextures::SetTexture("pause");
        soundPlayer->pauseTrack();
        pausedByPlayer = true;


        StopSoundTrackNoise();
        if(playSoundClick) PlaySoundClick();
    }

    void PlayContinueTrack(bool playClick = true) {
        PlaySoundTrackNoise(playClick);
        CassetteInterfaceTextures::SetTexture("play");
        soundPlayer->playContinueTrack();
    }

protected:

    CVehicle* vehicle;
    bool newVehicle = false;
    
    const string interfaceSoundPath = "resound//sounds//cassette//";
    const string clickSoundPathFull = (Utils::GetCurrentDirectory() + "//resound//sounds//cassette//click");
    const string clickSoundPath = "resound//sounds//cassette//click//";
    bool initialized = false;

    bool pausedByPlayer = false;
    bool playInitiated = false;
    bool fastFwdPressed = false;
    bool rewindPressed = false;

    float maxVolume = 0.23f;

    void ManagePlayback() {
        
        while (true) {
            this_thread::sleep_for(chrono::milliseconds(80));

            if (KeyPressed(VK_INSERT) && Utils::timePassedEnough(timeLastKeypress, 300)) {
                timeLastKeypress = CurrentTime();
                PlayPreviousTrack();
            }

            if ((KeyPressed(VK_END) && Utils::timePassedEnough(timeLastKeypress, 1200))) {
                timeLastKeypress = CurrentTime();

                if (!soundPlayer->hasPlayedAnyTrack()) {
                    PlayNewTrack();
                    continue;
                }

                if (soundPlayer->isPlayingOrActive()) {
                    PauseTrack(true);
                }
                else {
                    if (soundPlayer->playingStateIsPaused() || soundPlayer->playingStateIsStopped()) {
                        PlayContinueTrack();
                    }
                }
            }

            if (KeyPressed(VK_NEXT) && Utils::timePassedEnough(timeLastKeypress, 1200)) {
    
                SetTrackSpeedFastFwd();
                fastFwdPressed = true;
            }
            else {
                if (fastFwdPressed) { 
                    PlaySoundClick();
                    SetTrackSpeedNormal(); 
                    CassetteInterfaceTextures::textureCurrent = "play";
                }
                fastFwdPressed = false;
            }

            if (KeyPressed(VK_DELETE) && CurrentTime() > (timeLastKeypress + 300)) {

                CassetteInterfaceTextures::textureCurrent = "rewind";
                
                if (soundPlayer->getCurrentPositionSeconds() < 1) {
                    soundPlayer->speedNormal();

                    StopSoundTrackNoise();
                    StopSoundFastFwd();
                }
                else {
                    SetTrackSpeedRewind();
                }

                rewindPressed = true;
            }
            else {
                if (rewindPressed) {
                    PlaySoundClick();
                    SetTrackSpeedNormal();
                    if (soundPlayer->hasPlayedAnyTrack()) { 
                        PlayContinueTrack(); 
                        CassetteInterfaceTextures::textureCurrent = "play";
                    }
                    else {
                        CassetteInterfaceTextures::textureCurrent = "stop";
                        PlaySoundClick();
                    }
                    rewindPressed = false;
                }
            }

            if (KeyPressed(VK_PRIOR) && CurrentTime() > (timeLastKeypress + 300)) {
                timeLastKeypress = CurrentTime();
                PauseTrack(false);
                PlayNewTrack();
            }

            if (soundPlayer->playingStateIsStopped()) {
                PlayNewTrack();
            }

            //if (CassetteInterfaceTextures::textureCurrent == "fastfwd" && !fastFwdPressed) {
            //    SetTrackSpeedNormal();
            //}

            if (CassetteInterfaceTextures::texturePrevious.empty() || (CassetteInterfaceTextures::texturePrevious != CassetteInterfaceTextures::textureCurrent)) {
                CassetteInterfaceTextures::texturePrevious = CassetteInterfaceTextures::textureCurrent;
                //if (CassetteInterfaceTextures::textureCurrent == "pause") {
                //    string randomfile = Utils::pick_random_music_file(pauseSoundPathFull);
                //    InterfaceSounds::Play(pauseSoundPath + randomfile, "pause", false, true);
                //    InterfaceSounds::Stop("motorloop");
                //    InterfaceSounds::Stop("tapenoiseloop");
                //}
            }
        }
    }

    void AdjustVolumeValueFromSettings() {
        unsigned char* radioVolume = reinterpret_cast<unsigned char*>(0xBA6798);

        if (radioVolume != nullptr) {
            unsigned int intValue = (*radioVolume) & 0x7F;
            float volumeValue = Utils::Lerp(0.0f, maxVolume, (float)intValue / 64.0f);
            basicVolume = volumeValue;
        }
    }

    void SetTrackSpeedRewind() {
        CassetteInterfaceTextures::textureCurrent = "rewind";

        if (!rewindPressed) {
            PlaySoundFastFwd();
            if (!soundPlayer->hasPlayedAnyTrack()) return;
        }

        if (!soundPlayer->hasPlayedAnyTrack()) return;

        soundPlayer->speedRewind();

        
    }

    void SetTrackSpeedNormal() {
        soundPlayer->setVolume(basicVolume);
        StopSoundFastFwd();
        soundPlayer->speedNormal();
    }

    void SetTrackSpeedFastFwd() {
        CassetteInterfaceTextures::textureCurrent = "fastfwd";

        soundPlayer->setVolume(fastfwdVolume);
        if (!fastFwdPressed) {
            PlaySoundFastFwd();
            if (!soundPlayer->hasPlayedAnyTrack()) {
                PlayNewTrackOnFastFwd();
            }
            else {
                soundPlayer->playContinueTrack();
            }
            
        }

        if (soundPlayer->getCurrentPositionSeconds() < 1) return;

        soundPlayer->speedFastForward();
    }

    void PlayPreviousTrack() {

        if (!soundPlayer->hasPlayedAnyTrack()) {
            //PlaySoundClick();
            CassetteInterfaceTextures::SetTexture("eject");
            PlaySoundEject();
            this_thread::sleep_for(chrono::milliseconds(100));
            CassetteInterfaceTextures::SetTexture("stop");
            PlaySoundClick();
            return;
        }

        PlaySoundNewCassette();

        CassetteInterfaceTextures::SetTexture("play");

        PlaySoundTrackNoise();

        soundPlayer->playPreviousTrack(true);
        soundPlayer->setVolume(basicVolume);
    }

    void PlayNewTrack() {
        
        CassetteTextures::disableDrawingCassette = true;
        PlaySoundNewCassette();
        CassetteInterfaceTextures::SetTexture("play");
        
        

        this_thread::sleep_for(chrono::milliseconds(700));

        CassetteTextures::ReInitTextures();

        this_thread::sleep_for(chrono::milliseconds(700));

        CassetteTextures::disableDrawingCassette = false;
        PlaySoundTrackNoise();
        
        soundPlayer->playNextTrack();
        soundPlayer->setVolume(basicVolume);
    }

    void PlayNewTrackOnFastFwd() {
        CassetteInterfaceTextures::textureCurrent = "fastfwd";

        PlaySoundTrackNoise();

        soundPlayer->playNextTrack();
        soundPlayer->setVolume(basicVolume);
    }

    void PlaySoundEject() {
        InterfaceSounds::Play(interfaceSoundPath + "eject.mp3", "eject", false, true);
    }

    void PlaySoundNewCassette() {
        InterfaceSounds::Play(interfaceSoundPath + "eject.mp3", "eject", false, true);
        this_thread::sleep_for(chrono::milliseconds(500));
        InterfaceSounds::Play(interfaceSoundPath + "openeject.mp3", "openeject", false, true);
    }

    void PlaySoundTrackNoise(bool playClick = true) {
        if(playClick)
            InterfaceSounds::Play(interfaceSoundPath + "firstplay.mp3", "firstplay", false, true);

        InterfaceSounds::Play(interfaceSoundPath + "tapenoiseloop.mp3", "tapenoiseloop", true, false);
        InterfaceSounds::Play(interfaceSoundPath + "motorloop.mp3", "motorloop", true, false);
    }

    void StopSoundTrackNoise() {
        InterfaceSounds::Stop("motorloop");
        InterfaceSounds::Stop("tapenoiseloop");
    }

    void PlaySoundClick() {
        string randomfile = Utils::pick_random_music_file(clickSoundPathFull);
        InterfaceSounds::Play(clickSoundPath + randomfile, "click", false, true);
    }

    void PlaySoundFastFwd() {
        InterfaceSounds::Play(interfaceSoundPath + "pressfastfwd.mp3", "pressfastfwd", false, true);
        InterfaceSounds::Play(interfaceSoundPath + "startfastfwd.mp3", "startfastfwd", false, true);
        InterfaceSounds::Play(interfaceSoundPath + "loopfastfwd.mp3", "loopfastfwd", true, false);
    }

    void StopSoundFastFwd() {
        InterfaceSounds::Stop("loopfastfwd");
    }
};