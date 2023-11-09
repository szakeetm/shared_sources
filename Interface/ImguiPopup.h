#pragma once
#include "imgui.h"
#include <string>
#include <vector>
enum code { // most common responses and buttons
	NotDrawn =				1 << 0,
	DrawnNoResponse =		1 << 1,
	popupError =			1 << 2,
	buttonCancel =			1 << 3,
	buttonOk =				1 << 4,
};

class MyPopup {
public:
	typedef struct {
		std::string name;
		int retVal;
	} button;
	int ImMsgBox(std::string title, std::string message, std::vector<button> buttons); // main popup function to be called by others, should toggle a popup, set it's message, define buttons and if available return the button pressed
	void Draw();
	MyPopup();
	int GetResponse();
protected:
	int returnValue;
	std::string message;
	std::string title;
	std::vector<button> buttons;
	bool drawn;
};