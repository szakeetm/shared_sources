#pragma once
#include <string>
#include <vector>
#include <ImguiWindowBase.h>

class Interface;
// TODO Synrad stuff
class ImSelectFacetByResult : public ImWindow {
public:
	enum state : int {
		noMinHits, noMaxHits,
		noMinAbs, noMaxAbs,
		noMinDes, noMaxDes,
		btnSelect, btnAdd, btnRmv
	};
	void Draw();
	double minHits = 0, maxHits = 0;
	double minAbs = 0, maxAbs = 0;
	double minDes = 0, maxDes = 0;
	std::vector<state> states;
protected:
	std::string minHitsInput = "", maxHitsInput = "";
	std::string minAbsInput = "", maxAbsInput = "";
	std::string minDesInput = "", maxDesInput = "";
	bool Preprocess();
	void DoSelect();
};