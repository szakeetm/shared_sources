#pragma once
#include <string>
#include "ImguiWindowBase.h"
#include "Interface.h"

class ImSelectTextureType : public ImWindow{
public:
	enum modes : size_t {
		exactly,
		between,
		none,
		btnSelect,
		addSelect,
		rmvSelect
	};
	void Draw();
protected:
	size_t squareTextrueCheck = 2;
	int mode = none;
	size_t desorbtionCheck = 2;
	size_t absorbtionCheck = 2;
	size_t reflectionCheck = 2;
	size_t transpPassCheck = 2;
	size_t directionCheck = 2;
	double exactlyValue;
	double minValue;
	double maxValue;
	bool exactlyCheck = false;
	bool betweenCheck = false;
	std::string exactlyInput = "";
	std::string minInput = "";
	std::string maxInput = "";
	void Preprocess();
	void Select(int src);
};