
#pragma once

/*
    by gon_iss (c) 2026
*/

#include "RadioStation.h"

#include "../AudioSelector.h"
#include "../AudioResourceManager.h"

#include "../../Utils/pragmascope/pragmascope_client.h"
#include "../../Utils/Keys.h"

#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <algorithm>

using namespace std;

// --- States ---

enum class BTStatus { SUCCESS, FAILURE, RUNNING };
enum class SegmentType { NONE, MUSIC, ADVERT, NEWS, STATION_ID, SOLO };


struct RadioSegment {
    SegmentType type = SegmentType::NONE;
    string fileName;

    int durationMs = 0;
    int startTimeMs = 0;


    string introFile;
    int introDurationMs = 0;
    int introOffsetMs = 0;

    string outroFile;
    int outroDurationMs = 0;

    bool hasIntro = false;
    bool hasOutro = false;
    bool isReady = false;


    std::atomic<bool> isStartedInBass{ false };
    bool introTriggered = false;
    bool outroTriggered = false;

    AudioTrackTicket musicTicket;
    AudioTrackTicket introTicket;
    AudioTrackTicket outroTicket;

    bool IsFinished(int virtualTimeMs) const {
        if (!isReady) return false;
        return virtualTimeMs >= (startTimeMs + durationMs);
    }
};

/**
 * @brief Blackboard / state storage
 */
struct RadioBlackboard {
    int virtualTimeMs = 0;
    shared_ptr<RadioSegment> currentSegment = nullptr;

    int tracksSinceLastAd = 0;
    int tracksSinceLastNews = 0;
    int targetTracksUntilAd = 0;
    int adsToPlayInRow = 1;
    int currentAdInRowCounter = 0;

    float currentVolume = 1.0f; // Ducking (0.25 - 1.0)
    float targetVolume = 1.0f;
};

class RadioStation_V;

// --- Behavior Tree base ---

class BTNode {
public:
    virtual ~BTNode() = default;
    virtual BTStatus Tick(RadioStation_V* agent) = 0;
};

/**
 * @brief Selector runs children until one returns SUCCESS or RUNNING.
 */
class BTSelector : public BTNode {
    vector<unique_ptr<BTNode>> children;
public:
    void AddChild(unique_ptr<BTNode> c) { children.push_back(move(c)); }
    BTStatus Tick(RadioStation_V* a) override {
        for (auto& child : children) {
            BTStatus s = child->Tick(a);
            if (s != BTStatus::FAILURE) return s;
        }
        return BTStatus::FAILURE;
    }
};

/**
 * @brief Sequence: runs children until all SUCCESS.
 */
class BTSequence : public BTNode {
    vector<unique_ptr<BTNode>> children;
public:
    void AddChild(unique_ptr<BTNode> c) { children.push_back(move(c)); }
    BTStatus Tick(RadioStation_V* a) override {
        for (auto& child : children) {
            BTStatus s = child->Tick(a);
            if (s != BTStatus::SUCCESS) return s;
        }
        return BTStatus::SUCCESS;
    }
};

/**
 * @brief Planning node
 */
class ActionPlanner : public BTNode {
public:
    BTStatus Tick(RadioStation_V* a) override;
};

/**
 * @brief Execution node
 */
class ActionExecutor : public BTNode {
public:
    BTStatus Tick(RadioStation_V* a) override;
};

class RadioStation_V : public RadioStation {
    friend class ActionPlanner;
    friend class ActionExecutor;

public:
    RadioBlackboard bb;
    vector<double> adProbabilities = { 0.2, 0.1, 0.3, 0.2, 0.2 };

private:
    unique_ptr<BTNode> rootNode;

    AudioSelector musicSelector{ 15 };
    AudioSelector voiceSelector{ 20 };
    AudioSelector adSelector{ 10 };
    AudioSelector newsSelector{ 5 };
    AudioSelector idSelector{ 5 };

    string intro_path, general_path, ad_path, news_path, id_path, solo_path, time_path;
    SoundPlayer* voicePlayer = nullptr;

    const int MAX_CATCH_UP_STEPS = 5;
    const float SOLO_PROBABILITY = 0.15f;
    const float INTRO_PROBABILITY = 0.60f;
    const int NEWS_THRESHOLD = 8;

    PragmaScope* debugScope = nullptr;

public:
    RadioStation_V(string folder, SettingsRadioStation* settings) : RadioStation(folder, settings) {
        debugScope = new PragmaScope("Radio_V_BT_" + folder, "AudioLogic");

        intro_path = path + "\\intro\\";
        general_path = path + "\\general\\";
        id_path = path + "\\id\\";
        solo_path = path + "\\solo\\";
        time_path = path + "\\time\\";
        ad_path = "resound\\radio_adverts\\adverts_v\\";
        news_path = "resound\\radio_news\\news_v\\";

        InitPlayer(musicPlayer, path, false);
        InitPlayer(voicePlayer, solo_path, false);

        bb.targetTracksUntilAd = Utils::getRandomIntWithDifferentProbabilities(adProbabilities);
        bb.adsToPlayInRow = Utils::getRandomInt(1, 2);

        SetupBT();
    }

    std::pair<std::string, std::string> TryGetArtistTitle() const override {

        if (bb.currentSegment && !bb.currentSegment->fileName.empty()) {
            
            if (bb.currentSegment->type != SegmentType::MUSIC) return { "", "" };

            return Utils::GetTrackArtistAndName(Utils::remove_music_extension(Utils::GetFileNameFromPath(bb.currentSegment->fileName)));
        }

        return { "", "" };
    }

    virtual ~RadioStation_V() {
        if (debugScope) delete debugScope;
        if (voicePlayer) delete voicePlayer;
    }

    void SetupBT() {
        // Sequence root: plan -> execute
        auto root = make_unique<BTSequence>();
        root->AddChild(make_unique<ActionPlanner>());
        root->AddChild(make_unique<ActionExecutor>());
        rootNode = move(root);
    }

    void Launch() override {}

    void PlayMusicTrack() override {}

    void Update(float dt) override {
        if (forcePaused || !rootNode) return;

        bb.virtualTimeMs += (int)dt;

        //if (muted)
        //    return;

        // F7 test key
        if (Keys::GetKeyJustDown(rsF7) && bb.currentSegment && bb.currentSegment->isReady) {
            int elapsed = bb.virtualTimeMs - bb.currentSegment->startTimeMs;
            int remaining = bb.currentSegment->durationMs - elapsed;
            int skipMs = 25000;

            if (remaining < skipMs) skipMs = max(0, remaining - 2000);
            bb.virtualTimeMs += skipMs;

            int newElapsed = bb.virtualTimeMs - bb.currentSegment->startTimeMs;

            // async track pos
            if (bb.currentSegment->type == SegmentType::MUSIC && musicPlayer) {

                AudioResourceManager::Enqueue([this, newElapsed]() {
                    if (this->musicPlayer) {
                        this->musicPlayer->setTrackPositionMs(newElapsed);
                    }
                    });
            }
            else if (voicePlayer) {
                AudioResourceManager::Enqueue([this, newElapsed]() {
                    if (this->voicePlayer) {
                        this->voicePlayer->setTrackPositionMs(newElapsed);
                    }
                    });
            }

            if (debugScope) debugScope->info("BT_Radio", "Test Skip: " + to_string(skipMs) + "ms");
        }

        //int steps = 0;
        //while (steps < MAX_CATCH_UP_STEPS) {
            //if (rootNode->Tick(this) != BTStatus::SUCCESS) 
                //break;
        //    steps++;
        //}

        rootNode->Tick(this);

        if (Keys::GetKeyJustDown(rsF8)) LogStatus();
    }


    void Pause() override {
        if (this->forcePaused) return;
        this->forcePaused = true;

        if (bb.currentSegment) {

            if (bb.currentSegment->type == SegmentType::MUSIC) {
                if (AudioResourceManager::GetTrackPositionMs(bb.currentSegment->musicTicket) != -1) {
                    musicPlayer->pauseTrack();
                }

                bool voiceWasActive = (
                    AudioResourceManager::GetTrackPositionMs(bb.currentSegment->introTicket) != -1 || 
                    AudioResourceManager::GetTrackPositionMs(bb.currentSegment->outroTicket) != -1
                );

                if (voiceWasActive)
                    voicePlayer->pauseTrack();
            }
            else {
                voicePlayer->pauseTrack();
            }
        }
    }

    void Unpause() override {
        if (!this->forcePaused) return;
        this->forcePaused = false;

        if (bb.currentSegment) {

            if (bb.currentSegment->type == SegmentType::MUSIC) {
                if (AudioResourceManager::GetTrackPositionMs(bb.currentSegment->musicTicket) != -1)
                    musicPlayer->playContinueTrack();

                bool voiceWasActive = (AudioResourceManager::GetTrackPositionMs(bb.currentSegment->introTicket) != -1 ||
                    AudioResourceManager::GetTrackPositionMs(bb.currentSegment->outroTicket) != -1);

                if (voiceWasActive)
                    voicePlayer->playContinueTrack();
            }
            else {
                voicePlayer->playContinueTrack();
            }
        }
    }

    void Randomize() override {
        bb.currentSegment = nullptr;
        bb.virtualTimeMs = Utils::getRandomInt(0, 1000 * 60 * 120);
        musicPlayer->stopTrack();
        voicePlayer->stopTrack();
    }

    void UpdateVolume(bool isMissionTalkingNow) override {
        if (muted) return;

        auto s = bb.currentSegment;

        bool djVoiceActive = false;

        if (s->introTriggered && AudioResourceManager::GetTrackPositionMs(s->introTicket) != -1) djVoiceActive = true;
        if (s->outroTriggered && AudioResourceManager::GetTrackPositionMs(s->outroTicket) != -1) djVoiceActive = true;

        bb.targetVolume = djVoiceActive || isMissionTalkingNow ? VOL_MULT_DUCKED : VOL_MULT_NORMAL;

        if (abs(bb.currentVolume - bb.targetVolume) > 0.01f) {
            float step = FADE_SPEED * 16.0f; // 16ms approx frame
            if (bb.currentVolume < bb.targetVolume)
                bb.currentVolume = min(bb.currentVolume + step, bb.targetVolume);
            else
                bb.currentVolume = max(bb.currentVolume - step, bb.targetVolume);

            if (musicPlayer && s->type == SegmentType::MUSIC) {
                musicPlayer->setVolume(bb.currentVolume * basicVolume);
            }

            if (voicePlayer && isMissionTalkingNow) {
                voicePlayer->setVolume(bb.currentVolume * basicVolume);
            }
            else if (voicePlayer) {
                voicePlayer->setVolume(basicVolume);
            }
        }
    }

    void LogStatus() {
        string segName = bb.currentSegment ? Utils::GetFileNameFromPath(bb.currentSegment->fileName) : "None";
        string info = "Time: " + to_string(bb.virtualTimeMs) + "ms | Seg: " + segName + " | Type: " + to_string((int)bb.currentSegment->type);
        if (debugScope) debugScope->info("BT_Radio", info);
    }

    /**
     * @brief Fulfilles an empty segment
     */
    void ConfigureSegment(shared_ptr<RadioSegment> s, string file, SegmentType type) {
        s->type = type;
        s->fileName = file;

        // time measuring
        SoundPlayer* measurePlayer = (type == SegmentType::MUSIC) ? musicPlayer : voicePlayer;
        s->durationMs = measurePlayer ? measurePlayer->getTrackLengthMs(file) : 1000;

        s->startTimeMs = bb.currentSegment ? (bb.currentSegment->startTimeMs + bb.currentSegment->durationMs) : bb.virtualTimeMs;

        if (type == SegmentType::MUSIC) {
            s->hasIntro = Utils::getRandomBoolWithProbability(INTRO_PROBABILITY);
            s->hasOutro = Utils::getRandomBoolWithProbability(0.5f);
            s->introOffsetMs = Utils::getRandomInt(4000, 12000);

            if (s->hasIntro) {
                int hour = Utils::GameHours();
                string timeKey = (hour >= 5 && hour <= 10) ? "MORNING" : (hour >= 18 && hour <= 23) ? "EVENING" : "";

                if (!timeKey.empty()) s->introFile = voiceSelector.SelectByKeyword(time_path, timeKey);
                if (s->introFile.empty()) {
                    string trackKey = Utils::remove_music_extension(Utils::GetFileNameFromPath(file));
                    s->introFile = voiceSelector.SelectByKeyword(intro_path, trackKey);
                }

                if (!s->introFile.empty() && voicePlayer)
                    s->introDurationMs = voicePlayer->getTrackLengthMs(s->introFile);
                else
                    s->hasIntro = false;
            }

            if (s->hasOutro) {
                s->outroFile = voiceSelector.SelectRandom(general_path);

                if (!s->outroFile.empty() && voicePlayer)
                    s->outroDurationMs = voicePlayer->getTrackLengthMs(s->outroFile);
                else
                    s->hasOutro = false;
            }
        }
        s->isReady = true;
    }
};

// --- Nodes ---

BTStatus ActionPlanner::Tick(RadioStation_V* a) {
    // segment still playing
    if (a->bb.currentSegment && !a->bb.currentSegment->IsFinished(a->bb.virtualTimeMs)) {
        return BTStatus::SUCCESS;
    }

    auto next = make_shared<RadioSegment>();
    string file;
    SegmentType type = SegmentType::MUSIC;

    // advert logic
    bool wasAd = a->bb.currentSegment && a->bb.currentSegment->type == SegmentType::ADVERT;
    if (wasAd && a->bb.currentAdInRowCounter < a->bb.adsToPlayInRow - 1) {
        a->bb.currentAdInRowCounter++;
        file = a->adSelector.SelectNextTrack(a->ad_path);
        type = SegmentType::ADVERT;
    }
    else if (wasAd) {
        // advert -> Station ID
        file = a->idSelector.SelectNextTrack(a->id_path);
        type = SegmentType::STATION_ID;
        a->bb.tracksSinceLastAd = 0;
        a->bb.currentAdInRowCounter = 0;
        a->bb.targetTracksUntilAd = Utils::getRandomIntWithDifferentProbabilities(a->adProbabilities);
        a->bb.adsToPlayInRow = Utils::getRandomInt(1, 2);
    }
    // news
    else if (a->bb.tracksSinceLastNews >= a->NEWS_THRESHOLD) {
        file = a->newsSelector.SelectNextTrack(a->news_path);
        type = SegmentType::NEWS;
        a->bb.tracksSinceLastNews = 0;
    }
    // advert
    else if (a->bb.tracksSinceLastAd >= a->bb.targetTracksUntilAd) {
        file = a->adSelector.SelectNextTrack(a->ad_path);
        type = SegmentType::ADVERT;
        a->bb.currentAdInRowCounter = 0;
    }
    // solo
    else if (a->bb.currentSegment && a->bb.currentSegment->type == SegmentType::MUSIC && Utils::getRandomBoolWithProbability(a->SOLO_PROBABILITY)) {
        file = a->voiceSelector.SelectRandom(a->solo_path);
        type = SegmentType::SOLO;
    }
    // music
    else {
        file = a->musicSelector.SelectNextTrack(a->path);
        type = SegmentType::MUSIC;
        a->bb.tracksSinceLastAd++;
        a->bb.tracksSinceLastNews++;
    }

    // file not found
    if (file.empty()) {
        next->durationMs = 500;
        next->startTimeMs = a->bb.currentSegment ? (a->bb.currentSegment->startTimeMs + a->bb.currentSegment->durationMs) : a->bb.virtualTimeMs;
        next->isReady = true;
    }
    else {
        a->ConfigureSegment(next, file, type);
    }

    a->bb.currentSegment = next;
    return BTStatus::SUCCESS;
}

static inline CAudioEngine* eng = &AudioEngine;
static inline int lastTalkTime = CurrentTime();

BTStatus ActionExecutor::Tick(RadioStation_V* a) {
    auto s = a->bb.currentSegment;
    if (!s || !s->isReady || a->muted) return BTStatus::RUNNING;

    int elapsed = a->bb.virtualTimeMs - s->startTimeMs;

    // segment time never came or ran out
    if (elapsed < 0 || elapsed >= s->durationMs) return BTStatus::RUNNING;

    // --- Main ---
    if (!s->isStartedInBass) {
        s->isStartedInBass = true;
        string p = s->fileName;
        SoundPlayer* player = (s->type == SegmentType::MUSIC) ? a->musicPlayer : a->voicePlayer;

        if (player) {
            s->musicTicket = AudioResourceManager::EnqueuePlayback(player, [a, player, p, elapsed]() {
                if (!player) return;
                player->playTrackBASS(p, true);
                player->setTrackPositionMs(elapsed);
                // Важно: устанавливаем начальную громкость
                player->setVolume(a->bb.currentVolume * a->basicVolume);
                });
        }
    }

    // --- Intro ---
    if (s->hasIntro && !s->introFile.empty() && a->voicePlayer) {
        bool inRange = (elapsed >= s->introOffsetMs && elapsed < (s->introOffsetMs + s->introDurationMs));
        if (!s->introTriggered && inRange) {
            s->introTriggered = true;
            string vFile = s->introFile;
            int vPos = elapsed - s->introOffsetMs;
            s->introTicket = AudioResourceManager::EnqueuePlayback(a->voicePlayer, [a, vFile, vPos]() {
                if (!a->voicePlayer) return;
                a->voicePlayer->playTrackBASS(vFile, true);
                a->voicePlayer->setTrackPositionMs(vPos);
                a->voicePlayer->setVolume(a->basicVolume);
                });
        }
    }

    // --- Outro ---
    if (s->hasOutro && !s->outroFile.empty() && a->voicePlayer) {
        int outroStart = max(0, s->durationMs - s->outroDurationMs - (int)a->introFadeLengthMs);
        bool inRange = (elapsed >= outroStart && elapsed < (s->durationMs - (int)a->introFadeLengthMs));
        if (!s->outroTriggered && inRange) {
            s->outroTriggered = true;
            string vFile = s->outroFile;
            int vPos = elapsed - outroStart;
            s->outroTicket = AudioResourceManager::EnqueuePlayback(a->voicePlayer, [a, vFile, vPos]() {
                if (!a->voicePlayer) return;
                a->voicePlayer->playTrackBASS(vFile, true);
                a->voicePlayer->setTrackPositionMs(vPos);
                a->voicePlayer->setVolume(a->basicVolume);
                });
        }
    }

    return BTStatus::RUNNING;
}