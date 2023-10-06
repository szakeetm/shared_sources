#pragma once
#include <string>
class ImSelectTextureType {
public:
	enum modes : size_t {
		exactly,
		between,
		none,
		btnSelect,
		addSelect,
		rmvSelect
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
	bool exactlyCheck;
	bool betweenCheck;
protected:
	std::string exactlyInput;
	std::string minInput;
	std::string maxInput;
	void (*select)(int src);
	void Preprocess();
};