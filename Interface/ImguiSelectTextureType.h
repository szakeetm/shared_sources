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
	size_t squareTextrueCheck;
	int mode;
	size_t desorbtionCheck;
	size_t absorbtionCheck;
	size_t reflectionCheck;
	size_t transpPassCheck;
	size_t directionCheck;
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