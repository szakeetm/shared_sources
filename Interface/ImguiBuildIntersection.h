#pragma once

#include "ImguiWindowBase.h"

class ImBuildIntersect : public ImWindow {
public:
	void Draw();
protected:
	void BuildButtonPress();
	void UndoButtonPress();
	std::string message = "";
};