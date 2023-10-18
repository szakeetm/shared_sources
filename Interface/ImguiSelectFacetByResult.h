#pragma once
#include <string>

class ImSelectFacetByResult {
public:
	enum states : int {
		noMinHits, noMaxHits,
		noMinAbs, noMaxAbs,
		noMinDes, noMaxDes,
		btnSelect, btnAdd, btlRmv
	};
	void Draw();
	void Show();
	void Preprocess();
	double minHits, maxHits;
	double minAbs, maxAbs;
	double minDes, maxDes;
protected:
	std::string minHitsInput, maxHitsInput;
	std::string minAbsInput, maxAbsInput;
	std::string minDesInput, maxDesInput;
	bool drawn = false;
};