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
	short squareTextrueCheck = 2;
	int mode = none;
	short desorbtionCheck = 2;
	short absorbtionCheck = 2;
	short reflectionCheck = 2;
	short transpPassCheck = 2;
	short directionCheck = 2;
	double exactlyValue;
	double minValue;
	double maxValue;
	std::string exactlyInput = "";
	std::string minInput = "";
	std::string maxInput = "";
	void Preprocess();
	void Select(int src);
};