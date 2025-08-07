#pragma once

/*
    gon_iss (c) 2025
*/

#include "plugin.h"
using namespace plugin;

class HudController {


    static inline BYTE* hudDrawAddr1 = nullptr;
    static inline BYTE* hudDrawAddr2 = nullptr;

public:
    static void Init() {
        // Saving original draw functions bytes
        memcpy(originalDrawRadarMap, (void*)0x586880, 5);
        memcpy(originalDrawBlips, (void*)0x588050, 5);
        memcpy(originalDraw3dMarkers, (void*)0x585BF0, 5);

        // Saving the original circle draw call 
        memcpy(originalRadarCircleCall, (void*)0x58AA25, 5);

        // Another circle draws 
        memcpy(drawCircleNopArea1, (void*)0x58A818, 16);
        memcpy(drawCircleNopArea2, (void*)0x58A8C2, 16);
        memcpy(drawCircleNopArea3, (void*)0x58A96C, 16);

        hudDrawAddr1 = (BYTE*)0xBA6769;
        hudDrawAddr2 = (BYTE*)0xBA676A;
    }

    static void ShowRadar() {
        if (radarHidden) {
            // Restore og
            patch::SetRaw(0x586880, originalDrawRadarMap, 5);
            patch::SetRaw(0x588050, originalDrawBlips, 5);
            patch::SetRaw(0x585BF0, originalDraw3dMarkers, 5);

            patch::SetRaw(0x58A818, drawCircleNopArea1, 16);
            patch::SetRaw(0x58A8C2, drawCircleNopArea2, 16);
            patch::SetRaw(0x58A96C, drawCircleNopArea3, 16);

            patch::SetRaw(0x58AA25, originalRadarCircleCall, 5);

            *hudDrawAddr1 = 1;
            *hudDrawAddr2 = 1;

            radarHidden = false;
        }
    }

    static void HideRadar() {
        if (!radarHidden) {
            // Disable Radar
            patch::SetUChar(0x586880, 0xC3);
            patch::SetRaw(0x586881, (uint8_t*)"\x90\x90\x90\x90", 4);
            patch::SetUChar(0x588050, 0xC3);
            patch::SetRaw(0x588051, (uint8_t*)"\x90\x90\x90\x90", 4);
            patch::SetUChar(0x585BF0, 0xC3);
            patch::SetRaw(0x585BF1, (uint8_t*)"\x90\x90\x90\x90", 4);

            // Disable circle
            patch::RedirectCall(0x58AA25, EmptyRadarCircleFunction);
            
            // Disable what's left of it
            patch::Nop(0x58A818, 16);
            patch::Nop(0x58A8C2, 16);
            patch::Nop(0x58A96C, 16);


            *hudDrawAddr1 = 0;
            *hudDrawAddr2 = 0;

            radarHidden = true;
        }
    }

    static void __fastcall EmptyRadarCircleFunction(
        CSprite2d* sprite,
        void* _EDX,
        CRect const& rect,
        CRGBA const& color
    ) { // it's literally empty 
    }

    static void SetHudShown(bool state) {
        if (state) {
            ShowRadar();
        }
        else {
            HideRadar();
        }
    }

private:
    static inline bool radarHidden = false;
    static inline uint8_t originalDrawRadarMap[5];
    static inline uint8_t originalDrawBlips[5];
    static inline uint8_t originalDraw3dMarkers[5];
    static inline uint8_t originalRadarCircleCall[5];


    static inline uint8_t drawCircleNopArea1[16];
    static inline uint8_t drawCircleNopArea2[16];
    static inline uint8_t drawCircleNopArea3[16];
};