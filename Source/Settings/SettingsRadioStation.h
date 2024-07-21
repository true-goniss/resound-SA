#pragma once
#include <string>

#include "INIReader.h"
#include "Utils/Utils.h"

using namespace std;

class SettingsRadioStation
{
	public:

	std::string radio_type;

	bool adverts_enable = true;
	std::string adverts_path = "resound\\radio_adverts\\adverts_v\\";

	bool news_enable = true;
	std::string news_path = "resound\\radio_news\\news_v\\";

	bool time_enable_morning = true;
	bool time_enable_evening = true;
	bool time_enable_night = false;

	SettingsRadioStation(const std::string& stationPath) {

		this->stationPath = stationPath;
		this->radio_type = Utils::isRadioStationAGTAIIIVCType(this->stationPath) ? "IIIVC" : "V";

		std::string path = Utils::GetCurrentDirectory() + "\\resound\\radio\\" + stationPath + "\\settings\\radio_station_settings.ini";

		INIReader reader(path);

		if (reader.ParseError() < 0) {
			return;
		}

		Load(reader);
	}

	protected:

	std::string stationPath;

	void Load(INIReader reader) {

		if (reader.HasSection("type") && reader.HasValue("type", "custom_radio_type")) {

			const std::string val = reader.GetString("type", "custom_radio_type", radio_type);

			this->radio_type = (Utils::ContainsCaseInsensitive(val, "III") || Utils::ContainsCaseInsensitive(val, "VC")) ? "IIIVC" : val;
		}

		if (reader.HasSection("adverts")) {

			if (reader.HasValue("adverts", "enable")) 
			{
				this->adverts_enable = reader.GetBoolean("adverts", "enable", this->adverts_enable);
			}

			if (reader.HasValue("adverts", "custom_adverts_path")) 
			{
				this->adverts_path = reader.GetString("adverts", "custom_adverts_path", this->adverts_path);
			}
		}

		if (reader.HasSection("news") && reader.HasValue("news", "enable")) {
			this->news_enable = reader.GetBoolean("news", "enable", this->news_enable);

			if (reader.HasValue("news", "custom_news_path"))
			{
				this->news_path = reader.GetString("news", "custom_news_path", this->news_path);
			}
		}

		if (reader.HasSection("time")) {
			this->time_enable_morning = reader.GetBoolean("time", "enable_morning", this->time_enable_morning);
			this->time_enable_evening = reader.GetBoolean("time", "enable_evening", this->time_enable_evening);
			this->time_enable_night = reader.GetBoolean("time", "enable_night", this->time_enable_night);
		}
	}
};

