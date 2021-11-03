#pragma once

#include "ProfileModes.h"

std::map<ProfileRecordModes, std::pair<std::string, std::string>> profileRecordModeDescriptions = { //mode, long description, short description
	{ProfileRecordModes::None,     {"None",                        "None"}},
	{ProfileRecordModes::RegularU, {"Pressure/imp/density (\201)", "along \201"}},
	{ProfileRecordModes::RegularV, {"Pressure/imp/density (\202)", "along \202"}},
	{ProfileRecordModes::IncAngle, {"Incident angle",              "Inc. angle"}},
	{ProfileRecordModes::Speed,    {"Speed distribution",          "Speed"}},
	{ProfileRecordModes::OrtSpeed, {"Orthogonal velocity",         "Ort.velocity"}},
	{ProfileRecordModes::TanSpeed, {"Tangential velocity",         "Tan.velocity"}}
};

std::map<ProfileDisplayModes, std::string> profileDisplayModeDescriptions = { //mode, description
	{ProfileDisplayModes::Raw,          "Raw"},
	{ProfileDisplayModes::Pressure,     "Pressure (mbar)"},
	{ProfileDisplayModes::ImpRate,      "Impingement rate (1/m\262/sec)"},
	{ProfileDisplayModes::Density,      "Density (1/m3)"},
	{ProfileDisplayModes::Speed,        "Speed (m/s)"},
	{ProfileDisplayModes::Angle,        "Angle (deg)"},
	{ProfileDisplayModes::NormalizeTo1, "Normalize to 1"}
};
