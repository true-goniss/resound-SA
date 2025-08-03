#pragma once

/*
         
    gon_iss  (c) 2024 - 2025

    source repository : https://github.com/true-goniss/resound-SA

*/

#include <plugin.h>
#include "extensions/ScriptCommands.h"

#include "Visual/Fonts/DXFont.h"
#include "Utils/Compatibility.h";
#include "Visual/Fonts/Fonts.h"
#include "CassettePlayer.h"
#include "RadioSystem.h"

using namespace plugin;

class resound_SA {

    bool init = false;

public:

    resound_SA() {

        DXFont* fontArtist = new DXFont();
        DXFont* fontTrack = new DXFont();

        InitFonts(fontArtist, fontTrack);

        Events::initRwEvent += [] {
            Compatibility::InstallCompatibilityPatches();
        };

        Events::processScriptsEvent += [this, fontArtist, fontTrack] {
            if (init) return;

            if (plugin::Command<Commands::IS_PLAYER_PLAYING>(0))
            {
                init = true;

                Fonts::fArtist = fontArtist;
                Fonts::fTrack = fontTrack;
                CassettePlayer::Initialize();
                ResoundAudioEngine::Initialize();
                RadioSystem::Initialize();
            }


        };

        Events::drawHudEvent += [] {
            //Async::ProcessMainThreadEvents();
        };

        
    }

    static void InitFonts(DXFont* fontArtist, DXFont* fontTrack) {

        const char* fontPath = "Bank_Gothic_Medium.ttf";
        const char* fontName = "Bank Gothic(RUS BY LYAJKA)";

        Events::initRwEvent.Add([fontArtist, fontPath, fontName]() { fontArtist->InitFont(fontPath, fontName, 25); });
        Events::d3dResetEvent.Add([fontArtist, fontPath, fontName]() { fontArtist->InitFont(fontPath, fontName, 25); });

        Events::initRwEvent.Add([fontTrack, fontPath, fontName]() { fontTrack->InitFont(fontPath, fontName, 22); });
        Events::d3dResetEvent.Add([fontTrack, fontPath, fontName]() { fontTrack->InitFont(fontPath, fontName, 22); });
    }

} _resound_SA;
