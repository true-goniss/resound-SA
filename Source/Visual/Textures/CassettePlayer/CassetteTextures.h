#include <extensions/Screen.h>
#include <game_sa/CSprite2d.h>
#include <game_sa/CCamera.h>

#include "Utils/ShapeHelper.h"
#include "Utils/Utils.h"
#include "../Textures.h"


static class CassetteTextures
{

    public:

    static inline bool initialized = false;

    static inline bool disableDrawingCassette = false;

    static inline const int MAX_CIRCLE_SIDES = 60;
    static inline const float CIRCLE_STEP = 10.0f;
    static inline float mSinTable[360];
    static inline float mCosTable[360];

    static inline float maxDiameter = 76.31f;
    static inline float minDiameter = 42.0f;
    static inline CSprite2d* cassette_roller;
    static inline CSprite2d* cassette_tape;
    static inline CSprite2d* cassette;
    static inline CSprite2d* rollerAxisSprite;
    static inline CSprite2d* blicky;

    static inline RwRaster* cassette_roller_raster = nullptr;
    static inline RwRaster* cassette_raster = nullptr;

    static const inline Textures textures;

    static const inline float cassetteSizeX = 375.0f;
    static const inline float cassetteSizeY = 246.0f;

    //old x  w/o interface
    //static const inline float cassetteCoordX = 440.0f;

    //coords without interface
    //static const inline float cassetteCoordX = 440.0f;
    //static const inline float cassetteCoordY = 300.0f;

    static const inline float cassetteCoordX = 468.0f;
    static inline float cassetteCoordY = 350.0f;

    static const inline float RollerOne_X = cassetteCoordX - 112.5f;
    static const inline float RollerTwo_X = cassetteCoordX - 267.0f;
    static const inline float rollersOffsetY = 111.9f;

    static inline float Rollers_Y = cassetteCoordY - rollersOffsetY;
    static const inline float RollersDiameter = 85; //  30

    static inline const std::string skinsFolderFullPath = Utils::GetCurrentDirectory() + "\\" + "resound" + "\\" + "textures" + "\\" + "cassette" + "\\" + "skins";
    static inline bool reiniting = true;

    static inline void Initialize()
    {
        if (initialized) return;

        ShapeHelper::InitSinCosTable();

        for (unsigned int i = 0; i < 360; i++) {
            mSinTable[i] = sinf(static_cast<float>(i) * 0.017453292f);
            mCosTable[i] = cosf(static_cast<float>(i) * 0.017453292f);
        }

    }

    static inline void setCoordY(float coordY) {
        cassetteCoordY = coordY;
        Rollers_Y = coordY - rollersOffsetY;
    }

    static inline void DrawCenteredTexturedRotatedRectangle(float centerX, float centerY, float targetWidth, float targetHeight, CSprite2d* sprite, RwRaster* raster, float angle, float opacity)
    {
        if (!sprite->m_pTexture) {
            return;
        }

        float x = centerX - targetWidth * 0.5f;
        float y = centerY - targetHeight * 0.5f;

        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);

        float x1 = SCREEN_COORD_RIGHT(x);
        float y1 = SCREEN_COORD_BOTTOM(y);
        float x2 = SCREEN_COORD_RIGHT(x + targetWidth);
        float y2 = SCREEN_COORD_BOTTOM(y + targetHeight);

        CRect rect(x1, y1, x2, y2);

        float u1 = 0.0f;
        float v1 = 0.0f;
        float u2 = 1.0f;
        float v2 = 0.0f;
        float u3 = 0.0f;
        float v3 = 1.0f;
        float u4 = 1.0f;
        float v4 = 1.0f;

        CRGBA color = CRGBA(255, 255, 255, opacity);
        CSprite2d::SetVertices(rect, color, color, color, color, u1, v1, u2, v2, u3, v3, u4, v4);
        ShapeHelper::RotateVertices(sprite->maVertices, 4, SCREEN_COORD_RIGHT(x + targetWidth * 0.5f), SCREEN_COORD_BOTTOM(y + targetHeight * 0.5f), angle);

        RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, sprite->maVertices, 4);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
    }


    static void Delete() {
        cassette_tape->Delete();
        cassette_roller->Delete();
        cassette->Delete();
        rollerAxisSprite->Delete();
        blicky->Delete();
    }

    static void Draw(float percentPlayback, float playbackSpeed, bool isRewind, bool trackIsPlaying)
    {
        //if (!(percentPlayback > 0)) {
        //    return;
        //}

        if (!(reiniting || disableDrawingCassette)) {



            if (!cassette_roller) {
                cassette_roller = new CSprite2d();
                cassette_roller->m_pTexture = textures.LoadPNGTexture("cassette", "cassette_roller");
            }

            if (!cassette_tape) {
                cassette_tape = new CSprite2d();
                cassette_tape->m_pTexture = textures.LoadPNGTexture("cassette", "tape");
            }

            if (!cassette) {
                cassette = new CSprite2d();
                cassette->m_pTexture = textures.LoadPNGTexture("cassette", "cassette");
            }

            if (!blicky) {
                blicky = new CSprite2d();
                blicky->m_pTexture = textures.LoadPNGTexture("cassette", "blicky");
            }

            DrawTapes(percentPlayback);//
            DrawRollers(percentPlayback, playbackSpeed, isRewind, trackIsPlaying);
            DrawCassette();
        }

        DrawRollerAxis(RollerOne_X, Rollers_Y, 33);
        DrawRollerAxis(RollerTwo_X, Rollers_Y, 33);
    }



    static void ReInitTextures() {
        reiniting = true;

        const std::string skinFolder = Utils::tryFindRandomFolderName(skinsFolderFullPath);

        textures.RasterFromPngFile(( Utils::GetCurrentDirectory() + "\\resound\\textures\\cassette\\skins\\" + skinFolder + "\\cassette_roller.png").c_str(), cassette_roller_raster);
        textures.RasterFromPngFile(( Utils::GetCurrentDirectory() + "\\resound\\textures\\cassette\\skins\\" + skinFolder + "\\cassette.png").c_str(), cassette_raster);

        reiniting = false;
    }


    float degreesToRadians(float degrees) {
        return degrees * (3.141592653589793238 / 180.0);
    }

    void RotateVertices(RwIm2DVertex* verts, unsigned int num, float center_x, float center_y, float angle) {
        float l_angle = fmodf(angle, 360.0f);
        if (l_angle < 0.0f)
            l_angle += 360.0;
        l_angle = 360.0f - l_angle;
        float fCos = mCosTable[static_cast<unsigned int>(l_angle) % 360];
        float fSin = mSinTable[static_cast<unsigned int>(l_angle) % 360];
        for (unsigned int i = 0; i < num; i++) {
            float xold = verts[i].x;
            float yold = verts[i].y;
            verts[i].x = center_x + (xold - center_x) * fCos + (yold - center_y) * fSin;
            verts[i].y = center_y - (xold - center_x) * fSin + (yold - center_y) * fCos;
        }
    }

    static inline void DrawCassette() {
        CRect rect(SCREEN_COORD_RIGHT(cassetteCoordX), SCREEN_COORD_BOTTOM(cassetteCoordY),
            SCREEN_COORD_RIGHT(cassetteCoordX - cassetteSizeX), SCREEN_COORD_BOTTOM(cassetteCoordY - cassetteSizeY));

        if (cassette_raster) {
            float u1 = 0.0f;
            float v1 = 0.0f;
            float u2 = 1.0f;
            float v2 = 0.0f;
            float u3 = 0.0f;
            float v3 = 1.0f;
            float u4 = 1.0f;
            float v4 = 1.0f;

            CRGBA color = CRGBA(255, 255, 255, 255);
            CSprite2d::SetVertices(rect, color, color, color, color, u1, v1, u2, v2, u3, v3, u4, v4);

            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, cassette_raster);

            RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, cassette->maVertices, 4);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
        }

        //if (cassette) {

        //    if (cassette->m_pTexture) {
        //        cassette->Draw(rect, CRGBA(255, 255, 255, 255));
        //    }
        //}
    }

    static inline void DrawTapes(float percentPlayback) {
        //DrawTapePrimitive(RollerOne_X, Rollers_Y, percentPlayback);
        //DrawTapePrimitive(RollerTwo_X, Rollers_Y, 100 - percentPlayback);
        DrawTapeTextured(RollerOne_X, Rollers_Y, percentPlayback);
        DrawTapeTextured(RollerTwo_X, Rollers_Y, 100 - percentPlayback);
        DrawBlickyTextured(RollerOne_X, Rollers_Y, percentPlayback, AdjustAngleBasedOnCamera(TheCamera.m_aCams[TheCamera.m_nActiveCam].m_fVerticalAngle, TheCamera.m_aCams[TheCamera.m_nActiveCam].m_fHorizontalAngle));
        DrawBlickyTextured(RollerTwo_X, Rollers_Y, 100 - percentPlayback, AdjustAngleBasedOnCamera(TheCamera.m_aCams[TheCamera.m_nActiveCam].m_fVerticalAngle, TheCamera.m_aCams[TheCamera.m_nActiveCam].m_fHorizontalAngle));

        float verticalAngle = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_fVerticalAngle;
    }




    static inline float AdjustAngleBasedOnCamera(float verticalAngle, float horizontalAngle) {
        // angle based on camera orientation
        float adjustedAngle = 0.0f;

        // Damping factor to control rotation speed
        float dampingFactor = 0.1f;

        // vertical orientation with damping impacting rotation speed only
        float verticalRotationSpeed = Utils::MapRange(min(0.0f, verticalAngle), -0.2f, 0.0f, 0.0f, 1.0f);
        float verticalSpeedAdjusted = Utils::Lerp(0.0f, verticalRotationSpeed, dampingFactor);

        float horizontalRotation = Utils::MapRange(horizontalAngle, -1.7f, 1.5f, 180.0f, 90.0f);

        // Combine vertical and horizontal rotations
        adjustedAngle = verticalSpeedAdjusted + horizontalRotation;

        return adjustedAngle;
    }

    static inline float rollerAngle = 0;

    static inline void DrawRollerAxis(float coordRight_X1, float coordBottom_Y1, float diameter) {

        if (!rollerAxisSprite) {

            rollerAxisSprite = new CSprite2d();
            rollerAxisSprite->m_pTexture = textures.LoadPNGTexture("cassette_player", "rolleraxis");
        }

        float x = coordRight_X1 - diameter * 0.5f;
        float y = coordBottom_Y1 - diameter * 0.5f;

        float x1 = SCREEN_COORD_RIGHT(x);
        float y1 = SCREEN_COORD_BOTTOM(y);
        float x2 = SCREEN_COORD_RIGHT(x + diameter);
        float y2 = SCREEN_COORD_BOTTOM(y + diameter);

        CRect rect(x1, y1, x2, y2);

        rollerAxisSprite->Draw(rect, CRGBA(255, 255, 255, 255));
    }

    static inline void DrawRollers(float percentPlayback, float playbackSpeed, bool isRewind, bool trackIsPlaying) {

        //float l_angle = fmodf(rollerAngle, 360.0f);
        //if (l_angle < 0.0f)
        //    l_angle += 360.0;

        //l_angle = 360.0f + l_angle;

        if (!cassette_roller_raster) return;

        if (isRewind) {
            rollerAngle -= (5.05f * playbackSpeed);
        }
        else if (trackIsPlaying) {
            rollerAngle += (5.05f * playbackSpeed);
        }

        DrawCenteredTexturedRotatedRectangle(RollerOne_X, Rollers_Y, RollersDiameter, RollersDiameter, cassette_roller, cassette_roller_raster, rollerAngle, 255);
        DrawCenteredTexturedRotatedRectangle(RollerTwo_X, Rollers_Y, RollersDiameter, RollersDiameter, cassette_roller, cassette_roller_raster, rollerAngle, 255);
    }

    static inline void DrawTapePrimitive(float centerX, float centerY, float percentage) {

        float interpolatedDiameter = maxDiameter + (minDiameter - maxDiameter) * (percentage / 100.0f);
        float x = centerX - interpolatedDiameter * 0.5f;
        float y = centerY - interpolatedDiameter * 0.5f;

        ShapeHelper::DrawCircleSectorTextured(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, SCREEN_COORD_RIGHT(centerX), SCREEN_COORD_BOTTOM(centerY), interpolatedDiameter, interpolatedDiameter, 110.0f, 120.0f + 200.0f * 100, 25, 1, 1, 255);
    }

    static inline void DrawTapeTextured(float centerX, float centerY, float percentage) {
        float interpolatedDiameter = (maxDiameter * 2) + ((minDiameter * 2) - (maxDiameter * 2)) * (percentage / 100.0f);

        if (cassette_tape) {
            float x = centerX - interpolatedDiameter * 0.5f;
            float y = centerY - interpolatedDiameter * 0.5f;
            CRect rect(SCREEN_COORD_RIGHT(x), SCREEN_COORD_BOTTOM(y), SCREEN_COORD_RIGHT(x + interpolatedDiameter), SCREEN_COORD_BOTTOM(y + interpolatedDiameter));
            cassette_tape->Draw(rect, CRGBA(255, 255, 255, 255));
        }

    }

    static inline void DrawBlickyTextured(float centerX, float centerY, float percentage, float angle) {
        float interpolatedDiameter = (maxDiameter * 2) + ((minDiameter * 2) - (maxDiameter * 2)) * (percentage / 100.0f);

        float angle2 = 90.0f;
        float opacity2 = 200;

        if (blicky) {

            // second glare properties based on the angle of the first glare

            if (angle > 90.0f && angle < 180.0f) {
                // angle and opacity for the second glare in the specified range
                float angleDifference = angle - 90.0f;
                float angleDifferencePercent = angleDifference / 90.0f * 100;

                if (angleDifferencePercent < 50) {
                    opacity2 = std::round(angleDifferencePercent / 50 * 220.0f);
                }
                else {
                    opacity2 = std::round((100 - angleDifferencePercent) / 50 * 220.0f);
                }


                angle2 = 90.0f + angleDifference * 0.52f; //0.55f; // Increase angle
                DrawCenteredTexturedRotatedRectangle(centerX, centerY, interpolatedDiameter, interpolatedDiameter, blicky, blicky->m_pTexture->raster, angle2, opacity2);
            }
            else { //  180 / 220, 220 / 40 ; 40 / 90

                float normalizedAngle = angle;
                if (angle < 90) normalizedAngle = 180 + angle;

                if (normalizedAngle > 180 && normalizedAngle < 270) {
                    float angleDifference = 90 - (270 - normalizedAngle);
                    float angleDifferencePercent = angleDifference / 90.0f * 100;
                    if (angleDifferencePercent < 50) {
                        opacity2 = std::round(angleDifferencePercent / 50 * 190.0f);
                    }
                    else {
                        opacity2 = std::round((100 - angleDifferencePercent) / 50 * 190.0f);
                    }

                    angle2 = 180 + angleDifference * 0.52f;
                }

                DrawCenteredTexturedRotatedRectangle(centerX, centerY, interpolatedDiameter, interpolatedDiameter, blicky, blicky->m_pTexture->raster, angle2, opacity2);
            }

            DrawCenteredTexturedRotatedRectangle(centerX, centerY, interpolatedDiameter, interpolatedDiameter, blicky, blicky->m_pTexture->raster, angle, 255 - opacity2);
        }
    }
};

