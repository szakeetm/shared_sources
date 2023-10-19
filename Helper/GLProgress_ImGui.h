#pragma once
#include <GLApp/GLProgress_GUI.hpp>
#include "Interface/ImguiWindowBase.h"

class ImProgress : public GLProgress_Abstract, public ImWindow {
public:
	void Draw();
	void SetProgress(const double prg);
	void SetTitle(std::string title);
	void SetVisible(bool value);
protected:
	std::string title;
	bool isNew = true;
};
