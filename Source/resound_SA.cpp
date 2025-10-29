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

        InitFonts();

        Events::initRwEvent += [] {
            Compatibility::InstallCompatibilityPatches();
            HudController::Init();
        };

        Events::processScriptsEvent += [this] {
            if (init) return;

            if (plugin::Command<Commands::IS_PLAYER_PLAYING>(0))
            {
                init = true;

                CassettePlayer::Initialize();
                ResoundAudioEngine::Initialize();
                GamePausedWatcher::Initialize();
                RadioSystem::Initialize();
            }


        };

        Events::drawHudEvent += [] {
            //Async::ProcessMainThreadEvents();
        };

        
    }

    static void InitFonts() {

        const char* fontPath = "Bank_Gothic_Medium.ttf";
        const char* fontName = "Bank Gothic(RUS BY LYAJKA)";

        Fonts::Initialize(
            fontPath,
            fontName,
            25,
            22
        );
    }

} _resound_SA;
