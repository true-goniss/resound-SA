#pragma once

/*
    gon_iss (c) 2025
*/

#include "RadioWheelItem.h"
#include "../../Visual/RadioIcons.h"

int getStationIdForStandard(const std::string& name) {
    // Names with SA id
    static std::unordered_map<std::string, int> mapping = {
        { "Playback FM", 1}, 
        { "K Rose", 2},  
        {"K DST", 3}, 
        {"Bounce FM", 4}, 
        {"SF UR", 5},
        {"Radio Los Santos", 6}, 
        {"Radio X", 7},
        {"CSR 103.9", 8},
        {"K Jah West", 9},  
        {"Master Sounds 98.3", 10},
        {"WCTR", 11}, 
        { "User Tracks", 12}, 
        { "Radio Off", 13 }
    };

    return mapping.count(name) ? mapping[name] : -1;
}

class StandardWheelFactory : public IRadioWheelItemFactory {
public:
    std::vector<RadioWheelItem> createItems() override {
        std::vector<RadioWheelItem> items;

        items.push_back({
            RadioWheelItem::Type::Off,
            RadioIcons::getIconRadioOff(),
            "Radio Off", "", "", nullptr, 13
            });

        // Standard
        for (const auto& [name, icon] : RadioIcons::icons_round) {
            if (name == "Radio Off") continue;

            items.push_back({
                RadioWheelItem::Type::Standard,
                icon,
                name, "", "", nullptr,
                getStationIdForStandard(name)
            });
        }

        return items;
    }

};

class CustomWheelFactory : public IRadioWheelItemFactory {
public:
    CustomWheelFactory(std::vector<RadioStation*> stations)
        : stations_(std::move(stations)) {}

    std::vector<RadioWheelItem> createItems() override {
        std::vector<RadioWheelItem> items;

        // "Radio Off"
        items.push_back({
            RadioWheelItem::Type::Off,
            RadioIcons::getIconRadioOff(),
            "Radio Off", "", "", nullptr, 13
            });

        // Custom stations
        for (size_t i = 0; i < stations_.size(); ++i) {
            auto* station = stations_[i];
            auto artistTitle = station->TryGetArtistTitle();
            items.push_back({
                RadioWheelItem::Type::Custom,
                station->icon_round,
                station->name,
                artistTitle.first,
                artistTitle.second,
                station,
                static_cast<int>(i + 14) // ID from 14
                });
        }
        return items;

    }

private:
    std::vector<RadioStation*> stations_;
};

class CombinedWheelFactory : public IRadioWheelItemFactory {
public:
    CombinedWheelFactory(std::vector<RadioStation*> customStations)
        : customStations_(std::move(customStations)) {}

    std::vector<RadioWheelItem> createItems() override {
        std::vector<RadioWheelItem> items;

        // "Radio Off"
        items.push_back({
            RadioWheelItem::Type::Off,
            RadioIcons::getIconRadioOff(),
            "Radio Off", "", "", nullptr, 13
            });

        // Standard stations
        for (const auto& [name, icon] : RadioIcons::icons_round) {
            if (name == "Radio Off") continue;

            items.push_back({
                RadioWheelItem::Type::Standard,
                icon,
                name, "", "",
                nullptr,
                getStationIdForStandard(name)
                });
        }

        for (size_t i = 0; i < customStations_.size(); ++i) {
            auto* station = customStations_[i];
            auto artistTitle = station->TryGetArtistTitle();
            items.push_back({
                RadioWheelItem::Type::Custom,
                station->icon_round,
                station->name,
                artistTitle.first,
                artistTitle.second,
                station,
                static_cast<int>(i + 14) // ID from 14
                });
        }
        return items;
    }

private:
    std::vector<RadioStation*> customStations_;
};