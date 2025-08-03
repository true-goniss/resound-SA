#pragma once
#include "CAEAudioHardware.h"
#include "CAERadioTrackManager.h"
#include "CAudioEngine.h";
#include "plugin.h"
//#include "CHud.h"

#include "CassettePlayer.h"
#include "Sound/Radio/RadioStation.h"
#include "Sound/Radio/RadioStation_IIIVC.h"
#include "Sound/Radio/RadioStation_V.h"
#include "Sound/Radio/RadioStation_V_Talk.h"
#include "Sound/Radio/RadioStation_ConsistentlyPlayed.h"


#include "Settings/Settings.h"
#include "Sound/Radio/Utils/MusicTracksNames_SA.h"

#include "Components/RadioWheel/RadioWheel.h"


namespace fs = std::filesystem;

using namespace plugin;

static class RadioSystem
{
    static inline RadioWheel* radioWheel = new RadioWheel();;

    public:

    static inline void Initialize() {

        if (Initialised) return;

        Initialised = true;

        MusicTracksNames_SA::Init();

        patch::RedirectJump(0x4EB660, RadioSystem::Event_SA_RetuneRadio); // CAERadioTrackManager::CheckForStationRetune()
        patch::RedirectJump(0x507030, RadioSystem::Disable_SA_RadioName); // CAudioEngine::DisplayRadioStationName

        Events::gameProcessEvent += [] {
            ManagePlayback();
        };

        //Events::

        std::thread soundFadeProcessThread(&SoundFadeProcess_CustomStations);
        soundFadeProcessThread.detach();

        InitializeRadioStations();
    }

    static int GetCurrentRadioId() {
        return CurrentRadioId;
    }

    protected:

    static inline std::vector<RadioStation*> Custom_Radio_Stations;

    static inline CAERadioTrackManager* manager = &AERadioTrackManager;

    static inline int CurrentRadioId = 1;
    static inline int PreviousRadioId = 1;

    static inline int RetuneId = 1;

    static inline unsigned int getTrackNameTime = CurrentTime();
    static inline unsigned int retuneTime = CurrentTime();

    static inline bool Initialised = false;
    static inline bool isRadioStationChanged = false;
    static inline bool isReenteredVehicle = true;
    static inline bool isStationsLaunched = false;
    static inline bool randomizedOnPlayerDeadOrBusted = false;
    static inline bool customStationAfterReenteringVehicleBeenSet = false;
    static inline bool vehicleChanged = false;
    static inline bool hideSATrackNames = true;

    static inline std::vector<std::string> SA_Radio_Stations = { "Playback FM", "K Rose", "K DST", "Bounce FM", "SF UR", "Radio Los Santos", "Radio X", "CSR 103.9", "K Jah West", "Master Sounds 98.3", "WCTR", "User Tracks", "Radio Off" };

    static inline std::vector<std::pair<std::string, std::string>> SA_Radio_Current_ArtistTracks = { std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", ""), std::make_pair("", "") };

    static inline std::string STATE = "none";

    static inline CVehicle* vehicle = NULL;

    static inline CAudioEngine* eng = &AudioEngine;
    static inline int lastTalkTime = CurrentTime();
    static inline int talkFadeOutTime = CurrentTime();
    static inline float volumeDecreaseOnTalkMult = 0.2f;

    static inline void Disable_SA_RadioName() {}

    static void InitializeRadioStations() {

        fs::path directoryPath = "resound\\radio";

        for (const auto& entry : fs::directory_iterator(directoryPath)) {

            if (fs::is_directory(entry.path())) {

                std::string folderName = entry.path().filename().string();

                RadioStation* station;

                SettingsRadioStation* settings = new SettingsRadioStation(folderName);

                if (settings->radio_type == "V_TALK") {
                    station = new RadioStation_V_Talk(folderName, settings);
                }

                if (settings->radio_type == "CONSISTENTLY_PLAYED") {
                    station = new RadioStation_ConsistentlyPlayed(folderName, settings);
                }
                else {
                    if (settings->radio_type == "IIIVC") {
                        station = new RadioStation_IIIVC(folderName, settings);
                    }
                    else {
                        station = new RadioStation_V(folderName, settings);
                    }
                }

                Custom_Radio_Stations.push_back(station);
            }
        }

        for (const std::string& station_name : SA_Radio_Stations) {
            RadioStation* station;


        }

        RadioIcons::Initialize(SA_Radio_Stations, Custom_Radio_Stations);

        std::unique_ptr<IRadioWheelItemFactory> factory;

        switch (RadioWheel::WheelType::Custom) {
        case RadioWheel::WheelType::Standard:
            factory = std::make_unique<StandardWheelFactory>();
            break;
        case RadioWheel::WheelType::Custom:
            factory = std::make_unique<CustomWheelFactory>(Custom_Radio_Stations);
            break;
        case RadioWheel::WheelType::Combined:
            factory = std::make_unique<CombinedWheelFactory>(Custom_Radio_Stations);
            break;
        }

        radioWheel->Initialize(std::move(factory));
        radioWheel->SyncWithSystem(CurrentRadioId);

        radioWheel->OnSelectionChanged = RadioWheelSelectionChanged;
        radioWheel->OnSelectionSelected = RadioWheelSelectionSelected;

        //RadioWheel::Initialize(Custom_Radio_Stations);
        //RadioWheel::OnSelectionChanged = RadioWheelSelectionChanged;
        //RadioWheel::OnSelectionSelected = RadioWheelSelectionSelected;
    }




    static void RadioWheelSelectionChanged(const RadioWheelItem& item) {
        DisableRadio();
        InterfaceSounds::Stop("retuneloop");
        InterfaceSounds::Play(InterfaceSounds::radioInterfaceSoundsPath + "retuneloop.mp3", "retuneloop", true, false);
        InterfaceSounds::Play(InterfaceSounds::radioInterfaceSoundsPath + "radioselect.mp3", "radioselect", false, true);

        RadioInfoVisual::SetInfo(
            item.name,
            item.artist,
            item.title,
            item.name,
            true
        );
    };

    static void RadioWheelSelectionSelected(const RadioWheelItem& item) {
        InterfaceSounds::Stop("retuneloop");
        RetuneRadio(item.stationId);
    };

    static void ManagePlayback() {

        if (!isStationsLaunched)
        {
            isStationsLaunched = true;
            MuteCustomStations();
            LaunchCustomStations();
        }

        CPed* playa = FindPlayerPed();

        if (!(Command<Commands::IS_PLAYER_PLAYING>(0) && CanRetuneRadioStation(playa, FindPlayerVehicle(-1, false))))
        {
            MuteCustomStations();
        }

        if (Command<Commands::IS_CHAR_DEAD>(playa) || Command<Commands::HAS_CHAR_BEEN_ARRESTED>(playa) || playa->m_fHealth <= 0.0f)
        {
            MuteCustomStations();

            if (!randomizedOnPlayerDeadOrBusted) {
                randomizedOnPlayerDeadOrBusted = true;
                
                RandomizeCustomStations();
            }

            radioWheel->Hide();
        }
        else {
            randomizedOnPlayerDeadOrBusted = false;
        }

        if (CassettePlayer::IsNowActive()) {
            DisableRadio();
            radioWheel->Hide();
        }

        // Custom stations pause theirselves in their class
        //static inline void PauseCustomStations() {
        //    for (RadioStation* station : Custom_Radio_Stations) {
        //        station->Pause();
        //    }
        //}

        //static inline void UnpauseCustomStations() {
        //    for (RadioStation* station : Custom_Radio_Stations) {
        //        station->Unpause();
        //    }
        //}

        //if (CTimer::m_UserPause) {
        //    PauseCustomStations();
        //}
        //else {
        //    UnpauseCustomStations();
        //}
    }

    static inline void Event_SA_RetuneRadio() {

        CVehicle* foundVehicle = FindPlayerVehicle(-1, false);
        CPed* playa = FindPlayerPed();

        bool playaIsDriving = Command<Commands::IS_CHAR_IN_ANY_CAR>(playa);

        if (!playaIsDriving)
        {
            customStationAfterReenteringVehicleBeenSet = false;
            isReenteredVehicle = true;
        }

        if (Keys::GetKeyJustDown(rsF8)) {

            if (Settings::isDebuggingMode) {
                CAEAudioHardware* hardw = &AEAudioHardware;
                int trackId = plugin::CallMethodAndReturn<int, 0x4D8F80, CAEAudioHardware*>(hardw);
            }
        }

        int MaxRadioID = SA_Radio_Stations.size() + Custom_Radio_Stations.size();


        bool currentRadioIsCustom = CurrentRadioId > SA_Radio_Stations.size();

        bool showWheelRadioHudKeyDown = Keys::GetKeyDown(rsTAB);

        if (foundVehicle && CanRetuneRadioStation(playa, foundVehicle)) {

            if (showWheelRadioHudKeyDown && !CassettePlayer::IsNowActive()) {
                radioWheel->Show(CurrentRadioId);
            }
            else {
                radioWheel->Hide();
            }

            int CurrentRadioIndex = CurrentRadioId;

            if (showWheelRadioHudKeyDown && !currentRadioIsCustom) {
                CurrentRadioIndex = CurrentRadioId - 1;
            }
            else if (showWheelRadioHudKeyDown && currentRadioIsCustom) {
                CurrentRadioIndex = CurrentRadioId - 14;
            }

            bool mouseWheelDown = Keys::GetKeyJustDown(rsMOUSEWHEELDOWNBUTTON + MOUSE_CUSTOM_OFFSET);
            bool mouseWheelUp = Keys::GetKeyJustDown(rsMOUSEWHEELUPBUTTON + MOUSE_CUSTOM_OFFSET);

            //if (showWheelRadioHudKeyDown && mouseWheelDown) {
            //    RadioInfoVisual::wheelType = RadioInfoVisual::WheelType::Custom;
            //}
            //else if (showWheelRadioHudKeyDown && mouseWheelUp) {
            //    RadioInfoVisual::wheelType = RadioInfoVisual::WheelType::Standard;
            //}

            if (!showWheelRadioHudKeyDown && mouseWheelDown)
            {
                CurrentRadioId--;
                if (CurrentRadioId < 1) CurrentRadioId = MaxRadioID;
                isRadioStationChanged = true;
            }
            else if (!showWheelRadioHudKeyDown && mouseWheelUp)
            {
                CurrentRadioId++;
                if (CurrentRadioId > MaxRadioID) CurrentRadioId = 1;
                isRadioStationChanged = true;
            }
            

            if (isRadioStationChanged) {

                if (CassettePlayer::IsNowActive()) {
                    CassettePlayer::Hide();
                    
                }

                isRadioStationChanged = false;

                StartRetuneRadio(CurrentRadioId);
            }
        }
        else {
            radioWheel->Hide();
        }

        if (STATE == "retuneRadio" && (!playaIsDriving || CurrentTime() > retuneTime)) {
            InterfaceSounds::Stop("retuneloop");
            STATE = "none";

            if (CanRetuneRadioStation(playa, foundVehicle)) RetuneRadio(RetuneId);
        }

        if (CassettePlayer::IsNowActive()) return;

        if (foundVehicle && vehicle != foundVehicle) {
            vehicle = foundVehicle;
            vehicleChanged = true;

            StartRetuneRadio(CurrentRadioId);
        }
        else {
            vehicleChanged = false;
        }

        bool SA_radio_active = AERadioTrackManager.IsVehicleRadioActive();

        if (currentRadioIsCustom && AudioEngine.GetCurrentRadioStationID() <= 13 && !customStationAfterReenteringVehicleBeenSet) {
            if (SA_radio_active) {
                customStationAfterReenteringVehicleBeenSet = true;
                StartRetuneRadio(CurrentRadioId);
            }
        }

        if (!currentRadioIsCustom && isReenteredVehicle && SA_radio_active && CanRetuneRadioStation(playa, foundVehicle)) {
            isReenteredVehicle = false;
            RadioInfoVisual::ShowWithAnimation(SA_Radio_Stations[CurrentRadioId - 1], "", "", SA_Radio_Stations[CurrentRadioId - 1], false);
        }
    };

    static void DisableRadio() {
        AudioEngine.RetuneRadio(13);
        AERadioTrackManager.StartRadio(13, AERadioTrackManager.m_Settings.m_nBassSet, LOWORD(AERadioTrackManager.m_Settings.m_fBassGain), 0);

        AERadioTrackManager.StopRadio(NULL, 0);
        MuteCustomStations();
    }

    static void RetuneRadio(int stationId) {

        AERadioTrackManager.StopRadio(NULL, 0);
        MuteCustomStations();

        if (radioWheel) {
            radioWheel->SyncWithSystem(stationId);
        }

        CurrentRadioId = stationId;

        if (stationId == 13) { 
            return;
        }

        if (stationId <= 12) { 
            AERadioTrackManager.StartRadio(stationId,
                AERadioTrackManager.m_Settings.m_nBassSet,
                LOWORD(AERadioTrackManager.m_Settings.m_fBassGain), 0);
            AudioEngine.RetuneRadio(stationId);
        }
        else {
            int customIndex = stationId - 14;
            if (customIndex >= 0 && customIndex < Custom_Radio_Stations.size()) {
                for (auto* station : Custom_Radio_Stations) {
                    station->isTunedNow = false;
                }
                Custom_Radio_Stations[customIndex]->isTunedNow = true;
                Custom_Radio_Stations[customIndex]->Unmute();
            }
        }  
    }

    static void StartRetuneRadio(char id) {

        AERadioTrackManager.StopRadio(NULL, 0);
        MuteCustomStations();

        InterfaceSounds::Play(InterfaceSounds::radioInterfaceSoundsPath + "radioselect.mp3", "radioselect", false, true);
        InterfaceSounds::Play(InterfaceSounds::radioInterfaceSoundsPath + "retuneloop.mp3", "retuneloop", true, false);

        CVehicle* vehicle = FindPlayerVehicle(-1, false);
        
        bool canShowInfo = CanRetuneRadioStation(FindPlayerPed(), vehicle);

        if (radioWheel) {
            radioWheel->SyncWithSystem(id);
        }

        if (id > 13) {

            int indexCustom = id - 14;
            std::pair artist_title = Custom_Radio_Stations[indexCustom]->TryGetArtistTitle();

            if(canShowInfo) RadioInfoVisual::ShowWithAnimation(Custom_Radio_Stations[indexCustom]->name, artist_title.first, artist_title.second, Custom_Radio_Stations[indexCustom]->name, true);
        }
        else {

            int index = id - 1;

            const std::string artist = SA_Radio_Current_ArtistTracks[id].first;
            const std::string track = SA_Radio_Current_ArtistTracks[id].second;

            if(canShowInfo) RadioInfoVisual::ShowWithAnimation(SA_Radio_Stations[index], artist, track, SA_Radio_Stations[index], false);
        }

        STATE = "retuneRadio";
        retuneTime = CurrentTime() + 1000;
        RetuneId = id;
    }

    static inline void LaunchCustomStations() {
        for (RadioStation* station : Custom_Radio_Stations) {
            station->Launch();
        }
    }

    static inline void RandomizeCustomStations() {
        for (RadioStation* station : Custom_Radio_Stations) {
            station->Randomize();
        }
    }

    static inline void MuteCustomStations() {
        for (RadioStation* station : Custom_Radio_Stations) {
            station->Mute();
        }
    }

    static inline void SoundFadeProcess_CustomStations() {

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            for (RadioStation* station : Custom_Radio_Stations) {

                if (station->muted) continue;

                bool talking = !plugin::CallMethodAndReturn<bool, 0x5072C0, CAudioEngine*>(eng, 0);

                float decreasedVolume = station->basicVolume * volumeDecreaseOnTalkMult;

                if (talking) {
                    lastTalkTime = CurrentTime();
                    station->SetVolumeAllPlayers(decreasedVolume);
                    continue;
                }
                else {
                    int talkTimeDiff = CurrentTime() - lastTalkTime;

                    if (talkTimeDiff < 300) {
                        station->SetVolumeAllPlayers(decreasedVolume);
                        talkFadeOutTime = CurrentTime();
                        continue;
                    }
                    else if (talkTimeDiff >= 300 && talkTimeDiff < 1200) {
                        station->SetVolumeAllPlayers((decreasedVolume + ((station->basicVolume - decreasedVolume) * Utils::getElapsedTimePercentage(300, talkFadeOutTime) / 100)));
                        continue;
                    }
                }

                if (station->muted) {
                    station->musicPlayer->setVolume(0);
                    continue;
                }

                if (station->introAndOutroMusicFade->isActive) {
                    float volumeFadedValue = station->introAndOutroMusicFade->GetValue(station->basicVolume * station->introFadeMinVolumeMult, station->basicVolume, station->introFadeLengthMs, station->wholeIntroFadeLength);
                    station->musicPlayer->setVolume(volumeFadedValue);
                }
                else {
                    station->musicPlayer->setVolume(station->basicVolume);
                }
            }
        }
    }

    static inline void UnmuteCustomStation(int index) {
        Custom_Radio_Stations[index]->Unmute();
    }

    static bool CanRetuneRadioStation(CPed* playa, CVehicle* vehicle) {
        if (Command<Commands::IS_CHAR_DEAD>(playa) || 
            Command<Commands::HAS_CHAR_BEEN_ARRESTED>(playa) || 
            !Command<Commands::IS_CHAR_IN_ANY_CAR>(playa) ||
            (playa->m_fHealth <= 0.0 && playa->m_fArmour <= 0.0)
            ) return false;
        return (!(vehicle && vehicle->m_vehicleAudio.m_settings.m_nRadioType != RADIO_CIVILIAN));
    }
};