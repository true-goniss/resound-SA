#pragma once
#include "Animations/ShowFromDownAnimation.h"
#include "extensions/ScriptCommands.h"
#include <plugin.h>

#include "Utils/Compatibility.h"
#include "Utils/Keys.h"
#include "Visual/Fonts/Fonts.h"
#include "Visual/Textures/CassettePlayer/CassetteTextures.h"
#include "Visual/Textures/CassettePlayer/CassetteInterfaceTextures.h"
#include "Visual/Textures/CassettePlayer/CassettePlayerTextures.h"
#include "Visual/RadioInfoVisual.h"

#include "Sound/Cassette/CassettePlayer_Music.h"

using namespace plugin;

static class CassettePlayer
{
    static inline bool initialized = false;
    static inline ShowFromDownAnimation* showFromDownAnimation = new ShowFromDownAnimation();

    static inline float showFromDownAnimTime = 350.0f;

    static inline unsigned int last_show_keypressed = CurrentTime();
    static inline unsigned int last_show_event_time = CurrentTime();
    static inline bool show_direction = false;

    static void ProcessShowState() {

        if (Keys::GetKeyJustDown(67)) {

            if (CurrentTime() - last_show_event_time < 800) return;

            if (CurrentTime() - last_show_keypressed < 700) {
                show_direction = !show_direction;
                last_show_event_time = CurrentTime();

                showFromDownAnimation->Activate(show_direction);

                switch (show_direction) {
                case true:
                    Compatibility::EnablePrintOffset();
                    break;
                case false:
                    Compatibility::DisablePrintOffset();
                    break;
                }

            }
            else
            {
                last_show_keypressed = CurrentTime();
            }
        }

        CPed* playa = FindPlayerPed();

        if (Command<Commands::IS_CHAR_DEAD>(playa) || Command<Commands::HAS_CHAR_BEEN_ARRESTED>(playa)) 
        {
            showFromDownAnimTime = 6500;
            Hide();
        }
        else {
            showFromDownAnimTime = 350.0f;
        }
    }

    static void DrawTextures(float percentPlayback, float playbackSpeed, bool isRewind, bool trackIsPlaying, float showFromDownPercent) {

        if (CassetteInterfaceTextures::textureCurrent.empty()) CassetteInterfaceTextures::textureCurrent = "stop";

        float playerYposition = Utils::Lerp(-380, 12, showFromDownPercent / 100);

        float cassetteYposition = playerYposition + 338;

        float interfaceAndFrameWidth = 400.45f;
        float interfaceAndFramePosX = 480.45f;
        float frameWidth = 253;


        float interfaceHeight = 95.1f;
        float interfacePosY = interfaceHeight + playerYposition;

        float frameHeight = 326.1;
        float framePosY = frameHeight + interfacePosY;

        CassetteTextures::Initialize();
        CassettePlayerTextures::DrawBackground(interfaceAndFramePosX, interfacePosY + frameWidth, interfaceAndFramePosX - interfaceAndFrameWidth, framePosY - frameHeight);
        CassetteTextures::Draw(percentPlayback, playbackSpeed, isRewind, trackIsPlaying);
        CassetteTextures::setCoordY(cassetteYposition);
        CassettePlayerTextures::DrawFrame(interfaceAndFramePosX, interfacePosY + frameWidth, interfaceAndFramePosX - interfaceAndFrameWidth, framePosY - frameHeight);


        CassetteInterfaceTextures::Draw(interfaceAndFramePosX, interfacePosY, interfaceAndFramePosX - interfaceAndFrameWidth, interfacePosY - interfaceHeight);
    }

    static void DeleteTextures() {
        CassetteTextures::Delete();
        CassetteInterfaceTextures::Delete();
        CassettePlayerTextures::Delete();
    }

    static inline CassettePlayer_Music* cassPlayerMus = new CassettePlayer_Music();

    public:

    static void Initialize() {
        if (initialized) return;
        initialized = true;

        cassPlayerMus->soundPlayer->showTrackInfoOnNewTrack = true;

        Events::processScriptsEvent += [] {

        };

        Events::drawingEvent += [] {

            
            bool isRewindNow = cassPlayerMus->soundPlayer->isRewindNow;

            float playbackSpeed = isRewindNow ? cassPlayerMus->soundPlayer->rewindSpeed : cassPlayerMus->soundPlayer->playbackSpeed;

            DrawTextures(cassPlayerMus->soundPlayer->percentPlayback(), playbackSpeed, isRewindNow, IsNowActive(), showFromDownAnimation->GetValue(showFromDownAnimTime));

            if (TrackInfoVisual::opacityAnimationText->isAnimationNow)
            {
                TrackInfoVisual::DrawTrackInfo(Fonts::fArtist, Fonts::fTrack);
            }
            else if
               (RadioInfoVisual::opacityAnimationText->isAnimationNow)
            {
                RadioInfoVisual::DrawRadioInfo(Fonts::fArtist, Fonts::fTrack, false);
            }
        };

        Events::processScriptsEvent += [] {
            ProcessShowState();
        };

        Events::shutdownRwEvent += [] {
            DeleteTextures();
            TrackInfoVisual::Delete();
        };
    }

    static void Hide() {
        if (!showFromDownAnimation->isAnimationNow) {

            showFromDownAnimation->Activate(false);
            PauseMusic();
        }
    }

    static bool IsNowActive() {
        if (!Utils::timePassedEnough(cassPlayerMus->timeLastKeypress, 1500)) return true;

        bool isActive = cassPlayerMus->soundPlayer->isRewindNow || cassPlayerMus->soundPlayer->getTrackState() == 1;
        return isActive;
    }

    static void PauseMusic() {
        
        cassPlayerMus->PauseTrack(false);
    }
};