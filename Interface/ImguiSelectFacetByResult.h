#pragma once
#include <string>
#include <ImguiWindowBase.h>

class ImSelectFacetByResult : public ImWindow {
public:
	enum states : int {
		noMinHits, noMaxHits,
		noMinAbs, noMaxAbs,
		noMinDes, noMaxDes,
		btnSelect, btnAdd, btlRmv
	};
	void Draw();
	double minHits, maxHits;
	double minAbs, maxAbs;
	double minDes, maxDes;
protected:
	std::string minHitsInput, maxHitsInput;
	std::string minAbsInput, maxAbsInput;
	std::string minDesInput, maxDesInput;
	void Preprocess();
};