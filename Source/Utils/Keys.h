#pragma once

#include "CPad.h";
#include <iostream>
#include <chrono>
#include <unordered_map>

/* originally by gennariarmando ( _AG ) */

static std::unordered_map<int, std::chrono::time_point<std::chrono::high_resolution_clock>> keyDownTimeMap;

static class Keys
{
#define MOUSE_CUSTOM_OFFSET (2000)

public:

    static bool GetKeyDown(int key) {
        if (key < 255) {
            if (CPad::NewKeyState.standardKeys[key])
                return true;
        }
        else if (key > MOUSE_CUSTOM_OFFSET) {
            key -= MOUSE_CUSTOM_OFFSET;

            switch (key) {
            case rsMOUSELEFTBUTTON:
                if (CPad::NewMouseControllerState.lmb)
                    return true;
                break;
            case rsMOUSEMIDDLEBUTTON:
                if (CPad::NewMouseControllerState.mmb)
                    return true;
                break;
            case rsMOUSERIGHTBUTTON:
                if (CPad::NewMouseControllerState.rmb)
                    return true;
                break;
            case rsMOUSEWHEELUPBUTTON:
                if (CPad::NewMouseControllerState.wheelUp)
                    return true;
                break;
            case rsMOUSEWHEELDOWNBUTTON:
                if (CPad::NewMouseControllerState.wheelDown)
                    return true;
                break;
            case rsMOUSEX1BUTTON:
                if (CPad::NewMouseControllerState.bmx1)
                    return true;
                break;
            case rsMOUSEX2BUTTON:
                if (CPad::NewMouseControllerState.bmx2)
                    return true;
                break;
            default:
                break;
            }
        }

        for (int i = 0; i < 12; i++) {
            if (i + rsF1 == key) {
                if (CPad::NewKeyState.FKeys[i])
                    return true;
            }
        }

        switch (key) {
        case rsESC:
            if (CPad::NewKeyState.esc)
                return true;
            break;
        case rsINS:
            if (CPad::NewKeyState.insert)
                return true;
            break;
        case rsDEL:
            if (CPad::NewKeyState.del)
                return true;
            break;
        case rsHOME:
            if (CPad::NewKeyState.home)
                return true;
            break;
        case rsEND:
            if (CPad::NewKeyState.end)
                return true;
            break;
        case rsPGUP:
            if (CPad::NewKeyState.pgup)
                return true;
            break;
        case rsPGDN:
            if (CPad::NewKeyState.pgdn)
                return true;
            break;
        case rsUP:
            if (CPad::NewKeyState.up)
                return true;
            break;
        case rsDOWN:
            if (CPad::NewKeyState.down)
                return true;
            break;
        case rsLEFT:
            if (CPad::NewKeyState.left)
                return true;
            break;
        case rsRIGHT:
            if (CPad::NewKeyState.right)
                return true;
            break;
        case rsSCROLL:
            if (CPad::NewKeyState.scroll)
                return true;
            break;
        case rsPAUSE:
            if (CPad::NewKeyState.pause)
                return true;
            break;
        case rsNUMLOCK:
            if (CPad::NewKeyState.numlock)
                return true;
            break;
        case rsDIVIDE:
            if (CPad::NewKeyState.div)
                return true;
            break;
        case rsTIMES:
            if (CPad::NewKeyState.mul)
                return true;
            break;
        case rsMINUS:
            if (CPad::NewKeyState.sub)
                return true;
            break;
        case rsPLUS:
            if (CPad::NewKeyState.add)
                return true;
            break;
        case rsPADENTER:
            if (CPad::NewKeyState.enter)
                return true;
            break;
        case rsPADDEL:
            if (CPad::NewKeyState.decimal)
                return true;
            break;
        case rsPADEND:
            if (CPad::NewKeyState.num1)
                return true;
            break;
        case rsPADDOWN:
            if (CPad::NewKeyState.num2)
                return true;
            break;
        case rsPADPGDN:
            if (CPad::NewKeyState.num3)
                return true;
            break;
        case rsPADLEFT:
            if (CPad::NewKeyState.num4)
                return true;
            break;
        case rsPAD5:
            if (CPad::NewKeyState.num5)
                return true;
            break;
        case rsPADRIGHT:
            if (CPad::NewKeyState.num6)
                return true;
            break;
        case rsPADHOME:
            if (CPad::NewKeyState.num7)
                return true;
            break;
        case rsPADUP:
            if (CPad::NewKeyState.num8)
                return true;
            break;
        case rsPADPGUP:
            if (CPad::NewKeyState.num9)
                return true;
            break;
        case rsPADINS:
            if (CPad::NewKeyState.num0)
                return true;
            break;
        case rsBACKSP:
            if (CPad::NewKeyState.back)
                return true;
            break;
        case rsTAB:
            if (CPad::NewKeyState.tab)
                return true;
            break;
        case rsCAPSLK:
            if (CPad::NewKeyState.capslock)
                return true;
            break;
        case rsENTER:
            if (CPad::NewKeyState.extenter)
                return true;
            break;
        case rsLSHIFT:
            if (CPad::NewKeyState.lshift)
                return true;
            break;
        case rsSHIFT:
            if (CPad::NewKeyState.shift)
                return true;
            break;
        case rsRSHIFT:
            if (CPad::NewKeyState.rshift)
                return true;
            break;
        case rsLCTRL:
            if (CPad::NewKeyState.lctrl)
                return true;
            break;
        case rsRCTRL:
            if (CPad::NewKeyState.rctrl)
                return true;
            break;
        case rsLALT:
            if (CPad::NewKeyState.lmenu)
                return true;
            break;
        case rsRALT:
            if (CPad::NewKeyState.rmenu)
                return true;
            break;
        case rsLWIN:
            if (CPad::NewKeyState.lwin)
                return true;
            break;
        case rsRWIN:
            if (CPad::NewKeyState.rwin)
                return true;
            break;
        case rsAPPS:
            if (CPad::NewKeyState.apps)
                return true;
            break;
        default:
            break;
        }

        return false;
    }

    static bool GetKeyUp(int key) {
        return !GetKeyDown(key) && !GetKeyJustDown(key);
    }

    static bool GetKeyJustDown(int key) {

        if (key < 255) {
            if (CPad::NewKeyState.standardKeys[key] && !CPad::OldKeyState.standardKeys[key])
                return true;
        }
        else if (key > MOUSE_CUSTOM_OFFSET) {
            key -= MOUSE_CUSTOM_OFFSET;

            switch (key) {
            case rsMOUSELEFTBUTTON:
                if (CPad::NewMouseControllerState.lmb && !CPad::OldMouseControllerState.lmb)
                    return true;
                break;
            case rsMOUSEMIDDLEBUTTON:
                if (CPad::NewMouseControllerState.mmb && !CPad::OldMouseControllerState.mmb)
                    return true;
                break;
            case rsMOUSERIGHTBUTTON:
                if (CPad::NewMouseControllerState.rmb && !CPad::OldMouseControllerState.rmb)
                    return true;
                break;
            case rsMOUSEWHEELUPBUTTON:
                if (CPad::NewMouseControllerState.wheelUp && !CPad::OldMouseControllerState.wheelUp)
                    return true;
                break;
            case rsMOUSEWHEELDOWNBUTTON:
                if (CPad::NewMouseControllerState.wheelDown && !CPad::OldMouseControllerState.wheelDown)
                    return true;
                break;
            case rsMOUSEX1BUTTON:
                if (CPad::NewMouseControllerState.bmx1 && !CPad::OldMouseControllerState.bmx1)
                    return true;
                break;
            case rsMOUSEX2BUTTON:
                if (CPad::NewMouseControllerState.bmx2 && !CPad::OldMouseControllerState.bmx2)
                    return true;
                break;
            default:
                break;
            }
        }

        for (int i = 0; i < 12; i++) {
            if (i + rsF1 == key) {
                if (CPad::NewKeyState.FKeys[i] && !CPad::OldKeyState.FKeys[i])
                    return true;
            }
        }

        switch (key) {
        case rsESC:
            if (CPad::NewKeyState.esc && !CPad::OldKeyState.esc)
                return true;
            break;
        case rsINS:
            if (CPad::NewKeyState.insert && !CPad::OldKeyState.insert)
                return true;
            break;
        case rsDEL:
            if (CPad::NewKeyState.del && !CPad::OldKeyState.del)
                return true;
            break;
        case rsHOME:
            if (CPad::NewKeyState.home && !CPad::OldKeyState.home)
                return true;
            break;
        case rsEND:
            if (CPad::NewKeyState.end && !CPad::OldKeyState.end)
                return true;
            break;
        case rsPGUP:
            if (CPad::NewKeyState.pgup && !CPad::OldKeyState.pgup)
                return true;
            break;
        case rsPGDN:
            if (CPad::NewKeyState.pgdn && !CPad::OldKeyState.pgdn)
                return true;
            break;
        case rsUP:
            if (CPad::NewKeyState.up && !CPad::OldKeyState.up)
                return true;
            break;
        case rsDOWN:
            if (CPad::NewKeyState.down && !CPad::OldKeyState.down)
                return true;
            break;
        case rsLEFT:
            if (CPad::NewKeyState.left && !CPad::OldKeyState.left)
                return true;
            break;
        case rsRIGHT:
            if (CPad::NewKeyState.right && !CPad::OldKeyState.right)
                return true;
            break;
        case rsSCROLL:
            if (CPad::NewKeyState.scroll && !CPad::OldKeyState.scroll)
                return true;
            break;
        case rsPAUSE:
            if (CPad::NewKeyState.pause && !CPad::OldKeyState.pause)
                return true;
            break;
        case rsNUMLOCK:
            if (CPad::NewKeyState.numlock && !CPad::OldKeyState.numlock)
                return true;
            break;
        case rsDIVIDE:
            if (CPad::NewKeyState.div && !CPad::OldKeyState.div)
                return true;
            break;
        case rsTIMES:
            if (CPad::NewKeyState.mul && !CPad::OldKeyState.mul)
                return true;
            break;
        case rsMINUS:
            if (CPad::NewKeyState.sub && !CPad::OldKeyState.sub)
                return true;
            break;
        case rsPLUS:
            if (CPad::NewKeyState.add && !CPad::OldKeyState.add)
                return true;
            break;
        case rsPADENTER:
            if (CPad::NewKeyState.enter && !CPad::OldKeyState.enter)
                return true;
            break;
        case rsPADDEL:
            if (CPad::NewKeyState.decimal && !CPad::OldKeyState.decimal)
                return true;
            break;
        case rsPADEND:
            if (CPad::NewKeyState.num1 && !CPad::OldKeyState.num1)
                return true;
            break;
        case rsPADDOWN:
            if (CPad::NewKeyState.num2 && !CPad::OldKeyState.num2)
                return true;
            break;
        case rsPADPGDN:
            if (CPad::NewKeyState.num3 && !CPad::OldKeyState.num3)
                return true;
            break;
        case rsPADLEFT:
            if (CPad::NewKeyState.num4 && !CPad::OldKeyState.num4)
                return true;
            break;
        case rsPAD5:
            if (CPad::NewKeyState.num5 && !CPad::OldKeyState.num5)
                return true;
            break;
        case rsPADRIGHT:
            if (CPad::NewKeyState.num6 && !CPad::OldKeyState.num6)
                return true;
            break;
        case rsPADHOME:
            if (CPad::NewKeyState.num7 && !CPad::OldKeyState.num7)
                return true;
            break;
        case rsPADUP:
            if (CPad::NewKeyState.num8 && !CPad::OldKeyState.num8)
                return true;
            break;
        case rsPADPGUP:
            if (CPad::NewKeyState.num9 && !CPad::OldKeyState.num9)
                return true;
            break;
        case rsPADINS:
            if (CPad::NewKeyState.num0 && !CPad::OldKeyState.num0)
                return true;
            break;
        case rsBACKSP:
            if (CPad::NewKeyState.back && !CPad::OldKeyState.back)
                return true;
            break;
        case rsTAB:
            if (CPad::NewKeyState.tab && !CPad::OldKeyState.tab)
                return true;
            break;
        case rsCAPSLK:
            if (CPad::NewKeyState.capslock && !CPad::OldKeyState.capslock)
                return true;
            break;
        case rsENTER:
            if (CPad::NewKeyState.extenter && !CPad::OldKeyState.extenter)
                return true;
            break;
        case rsLSHIFT:
            if (CPad::NewKeyState.lshift && !CPad::OldKeyState.lshift)
                return true;
            break;
        case rsSHIFT:
            if (CPad::NewKeyState.shift && !CPad::OldKeyState.shift)
                return true;
            break;
        case rsRSHIFT:
            if (CPad::NewKeyState.rshift && !CPad::OldKeyState.rshift)
                return true;
            break;
        case rsLCTRL:
            if (CPad::NewKeyState.lctrl && !CPad::OldKeyState.lctrl)
                return true;
            break;
        case rsRCTRL:
            if (CPad::NewKeyState.rctrl && !CPad::OldKeyState.rctrl)
                return true;
            break;
        case rsLALT:
            if (CPad::NewKeyState.lmenu && !CPad::OldKeyState.lmenu)
                return true;
            break;
        case rsRALT:
            if (CPad::NewKeyState.rmenu && !CPad::OldKeyState.rmenu)
                return true;
            break;
        case rsLWIN:
            if (CPad::NewKeyState.lwin && !CPad::OldKeyState.lwin)
                return true;
            break;
        case rsRWIN:
            if (CPad::NewKeyState.rwin && !CPad::OldKeyState.rwin)
                return true;
            break;
        case rsAPPS:
            if (CPad::NewKeyState.apps && !CPad::OldKeyState.apps)
                return true;
            break;
        default:
            break;
        }

        return false;
    }

    static bool GetKeyWasDownForMs(int key, double milliseconds) {

        bool keyDown = GetKeyDown(key);
        bool keyUp = GetKeyUp(key);
        bool keyWasDown = (keyDownTimeMap.find(key) != keyDownTimeMap.end());


        if (keyUp && keyWasDown) {
            keyDownTimeMap.erase(key);
            return false;
        }

        if (keyDown && !keyWasDown) {
            keyDownTimeMap[key] = std::chrono::high_resolution_clock::now();
            return false;
        }

        if (keyDown && keyWasDown) {
            auto keyDownTime = keyDownTimeMap[key];
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - keyDownTime).count();

            if (elapsedTime >= milliseconds) {
                return true;
            }
        }

        return false;
    }
};