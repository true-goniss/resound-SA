#pragma once
#include <game_sa/CSprite2d.h>
#include "../../Sound/Radio/RadioStation.h"

struct RadioWheelItem {
    enum class Type { Standard, Custom, Off };

    Type type;
    CSprite2d* icon = nullptr;
    std::string name;
    std::string artist;
    std::string title;
    RadioStation* customStation = nullptr; // only for custom stations
    int stationId = -1;
};

class IRadioWheelItemFactory {
public:
    virtual ~IRadioWheelItemFactory() = default;
    virtual std::vector<RadioWheelItem> createItems() = 0;
};