#pragma once

#include <map>
#include <string>

enum class profileRecordModes {
	None,
	RegularU,
	RegularV,
	IncAngle,
	Speed,
	OrtSpeed,
	TanSpeed,
	NUMITEMS
};

std::map<profileRecordModes, std::pair<std::string, std::string>> profileRecordModeDescriptions = { //mode, long description, short description
	{profileRecordModes::None, {"None","None"}},
	{profileRecordModes::RegularU, {"Pressure/imp/density (\201)","along \201"}},
	{profileRecordModes::RegularV, {"Pressure/imp/density (\202)","along \202"}},
	{profileRecordModes::IncAngle, {"Incident angle","Inc. angle"}},
	{profileRecordModes::Speed, {"Speed distribution","Speed"}},
	{profileRecordModes::OrtSpeed,{"Orthogonal velocity","Ort.velocity"}},
	{profileRecordModes::TanSpeed,{"Tangential velocity","Tan.velocity"}}
};

enum class profileDisplayModes {
	Raw,
	Pressure,
	ImpRate,
	Density,
	Speed,
	Angle,
	NormalizeTo1,
	NUMITEMS
};

std::map<profileDisplayModes, std::string> profileDisplayModeDescriptions = { //mode, description
	{profileDisplayModes::Raw, "Raw"},
	{profileDisplayModes::Pressure, "Pressure (mbar)"},
	{profileDisplayModes::ImpRate, "Impingement rate (1/m\262/sec)"},
	{profileDisplayModes::Density, "Density (1/m3)"},
	{profileDisplayModes::Speed, "Speed (m/s)"},
	{profileDisplayModes::Angle,"Angle (deg)"},
	{profileDisplayModes::NormalizeTo1,"Normalize to 1"}
};
