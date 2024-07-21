#include "Compatibility.h"
#include "plugin.h"
#include "CFont.h"
#include "common.h"

/* originally by dk22pac */

using namespace plugin;

float offset = 0;

void Compatibility::PrintAreaName(float x, float y, char* text) {
    if (FindPlayerVehicle(-1, false)) {
        x -= SCREEN_COORD(offset);
        CFont::PrintStringFromBottom(x, y, text);
    }
    else
        CFont::PrintStringFromBottom(x, y, text);
}

void Compatibility::PrintVehicleName(float x, float y, char* text) {
    if (FindPlayerVehicle(-1, false)) {
        x -= SCREEN_COORD(offset);
        CFont::PrintString(x, y, text);
    }
    else
        CFont::PrintString(x, y, text);
}

void Compatibility::DisablePrintOffset() {
    offset = 0;
}

void Compatibility::EnablePrintOffset() {
    offset = 500;
}

void Compatibility::InstallCompatibilityPatches() {
    patch::RedirectCall(0x58AE5D, PrintAreaName);
    patch::RedirectCall(0x58B156, PrintVehicleName);
}