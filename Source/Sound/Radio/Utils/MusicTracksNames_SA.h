#pragma once
#include <unordered_map>
#include <string>
#include "Utils/Utils.h"

/*
	by gon_iss (c) 2024
*/

static class MusicTracksNames_SA
{
	static inline std::unordered_map<std::string, std::string> tracksIdNames;

public:

	static void Init() {
		tracksIdNames["1118"] = "2Pac feat. Pogo — I Don’t Give a Fuck"; // 1123
		tracksIdNames["1169"] = "Above the Law — Murder Rap";
		tracksIdNames["1132"] = "Compton’s Most Wanted — Hood Took Me Under";
		tracksIdNames["1164"] = "Cypress Hill — How I Could Just Kill a Man";
		tracksIdNames["1183"] = "Da Lench Mob — Guerillas in tha Mist";
		tracksIdNames["1202"] = "Dr. Dre feat. Snoop Dogg — Deep Cover";
		tracksIdNames["1111"] = "Dr. Dre feat. Snoop Dogg + Jewell + RBX — Fuck wit Dre Day (And Everybody’s Celebratin')";
		tracksIdNames["1125"] = "Dr. Dre feat. Snoop Dogg — Nuthin' But A 'G' Thang";
		tracksIdNames["1176"] = "Eazy-E — Eazy-Er Said Than Dunn";
		tracksIdNames["1195"] = "Ice Cube feat. Das EFX — Check Yo Self";
		tracksIdNames["1190"] = "Ice Cube — It Was a Good Day";
		tracksIdNames["1158"] = "Kid Frost — La Raza";
		tracksIdNames["1146"] = "N.W.A — Alwayz Into Somethin'";
		tracksIdNames["1152"] = "N.W.A — Express Yourself";
		tracksIdNames["1139"] = "The D.O.C. — It’s Funky Enough";
		tracksIdNames["1208"] = "Too Short — The Ghetto";
	}

	static inline std::pair<std::string, std::string> GetArtistAndNameByID(int trackID) {
		std::string id_str = std::to_string(trackID);

		for (const auto& pair : tracksIdNames)
		{
			if (Utils::ContainsCaseInsensitive(pair.first, id_str)) {
				std::pair artist_name = Utils::GetTrackArtistAndName(pair.second);
				return artist_name;
			}
		}

		return std::make_pair("", "");
	}
};