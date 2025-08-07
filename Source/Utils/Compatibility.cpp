#include "Compatibility.h"
#include "plugin.h"
#include "CFont.h"
#include "common.h"

/* originally by dk22pac */

using namespace plugin;

float offsetX = 0;
//float offsetY = 0;

void Compatibility::SetPrintOffsetX(bool enabled, float value) {
    offsetX = enabled ? value : 0;
}

void Compatibility::InstallCompatibilityPatches() {
    patch::RedirectCall(0x58AE5D, PrintAreaName);
    patch::RedirectCall(0x58B156, PrintVehicleName);
}

void Compatibility::PrintAreaName(float x, float y, char* text) {
    if (FindPlayerVehicle(-1, false)) {
        x += SCREEN_COORD(offsetX);
        //y += SCREEN_COORD(offsetY);

        CFont::PrintStringFromBottom(x, y, text);
    }
    else
        CFont::PrintStringFromBottom(x, y, text);
}

void Compatibility::PrintVehicleName(float x, float y, char* text) {
    if (FindPlayerVehicle(-1, false)) {
        x += SCREEN_COORD(offsetX);
        //y += SCREEN_COORD(offsetY);

        CFont::PrintString(x, y, text);
    }
    else
        CFont::PrintString(x, y, text);
}