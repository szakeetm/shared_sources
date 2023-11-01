#pragma once

class Interface;

class ImWindow {
public:
	virtual void Draw() = 0;
	void Init(Interface* mApp_);
	void Show();
	void Hide();
	void Toggle();
	const bool IsVisible();
	void SetVisible(bool value);
protected:
	bool drawn = false;
	float txtW=0, txtH=0;
	Interface* mApp;
};