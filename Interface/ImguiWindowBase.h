#pragma once

class ImWindow {
public:
	virtual void Draw() = 0;
	void Show();
	void Hide();
	void Toggle();
	const bool IsVisible();
	void SetVisible(bool value);
protected:
	bool drawn = false;
};