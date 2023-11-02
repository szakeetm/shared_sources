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
// explicidly defined values to save many lines of ImGui code
enum class ProfileDisplayModes {
	Raw = 0,
	Pressure = 1,
	ImpRate = 2,
	Density = 3,
	Speed = 4,
	Angle = 5,
	NormalizeTo1 = 6,
	NUMITEMS = 7
};

extern std::map<ProfileRecordModes, std::pair<std::string, std::string>> profileRecordModeDescriptions;
extern std::map<ProfileDisplayModes, std::string> profileDisplayModeDescriptions;
