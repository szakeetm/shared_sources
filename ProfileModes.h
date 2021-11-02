#pragma once

#include <map>
#include <string>

enum class ProfileRecordModes {
	None,
	RegularU,
	RegularV,
	IncAngle,
	Speed,
	OrtSpeed,
	TanSpeed,
	NUMITEMS
};

enum class ProfileDisplayModes {
	Raw,
	Pressure,
	ImpRate,
	Density,
	Speed,
	Angle,
	NormalizeTo1,
	NUMITEMS
};

extern std::map<ProfileRecordModes, std::pair<std::string, std::string>> profileRecordModeDescriptions;
extern std::map<ProfileDisplayModes, std::string> profileDisplayModeDescriptions;
