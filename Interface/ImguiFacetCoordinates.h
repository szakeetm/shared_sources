#pragma once
#include "ImguiWindowBase.h"

class ImFacetCoordinates : public ImWindow {
public:
	void Draw();
protected:
	void DrawTable();
	void ApplyButtonPress();
	void Apply();
	void Insert(int pos=-1);
	std::string insertIdInput = "0";
	size_t insertID = 0;
};