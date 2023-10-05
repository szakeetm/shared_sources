#pragma once
#include <string>
class ImSelectTextureType {
public:
	enum mode: size_t {
		exactly,
		between
	};
	ImSelectTextureType();
	void Show();
	void Draw();
	bool drawn;
	bool squareTextrueCheck;
	int mode;
	bool desorbtionCheck;
	bool absorbtionCheck;
	bool reflectionCheck;
	bool transpPassCheck;
	bool directionCheck;
	double exactlyValue;
	double minValue;
	double maxValue;
protected:
	std::string exactlyInput;
	std::string minInput;
	std::string maxInput;
	void (*select)();
	void (*addSelect)();
	void (*rmvSelect)();
	void Preprocess();
};