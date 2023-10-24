#pragma once
#include <string>
#include <vector>
#include <ImguiWindowBase.h>

class Interface;
// TODO Synrad stuff
class ImSelectFacetByResult : public ImWindow {
public:
	enum states : int {
		noMinHits, noMaxHits,
		noMinAbs, noMaxAbs,
		noMinDes, noMaxDes,
		btnSelect, btnAdd, btnRmv
	};
	void Draw();
	void Init(Interface* mApp_);
	double minHits, maxHits;
	double minAbs, maxAbs;
	double minDes, maxDes;
	std::vector<states> state;
protected:
	std::string minHitsInput, maxHitsInput;
	std::string minAbsInput, maxAbsInput;
	std::string minDesInput, maxDesInput;
	bool Preprocess();
	void DoSelect();
	Interface* mApp;
};