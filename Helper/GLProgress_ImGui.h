#pragma once
#include <GLApp/GLProgress_GUI.hpp>

class MyProgress : public GLProgress_Abstract {
public:
	void Draw();
	void Show();
	void Hide();
	void Toggle();
	void SetProgress(const double prg);
	void SetTitle(std::string title);
	void SetVisible(bool value);
protected:
	std::string title;
	bool drawn;
};
