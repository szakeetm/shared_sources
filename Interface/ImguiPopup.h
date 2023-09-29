#pragma once
#include "imgui.h"
#include <string>
#include <vector>
#include "GLApp.h"

enum PopupCode { // most common responses and buttons
	notDrawn		= -3, // the popup is inactive
	drawnNoResponse = -2, // the popup is visible but no user input is registered
	popupError		= -1, // values higher than this represent responses
	buttonCancel    =  1,
	buttonOk		=  2,
};

class MyPopup {
public:
	MyPopup();
	// pair of values that contains text for a button and a return code to be returned if that button is pressed
	typedef struct {
		std::string name;
		int retVal; // PopupCode enum contains reserved values
	} button;

	void OpenImMsgBox(std::string title, std::string message, std::vector<button> buttons); // main popup function to be called by others, should toggle a popup, set it's message, define buttons and if available return the button pressed
	void Draw(); // call this every ImGui Render
	bool WasResponse();
	int GetResponse(); // returns the recorded value from a button press
protected:
	int returnValue;
	std::string message;
	std::string title;
	bool wasClicked;
	bool drawn;
	std::vector<button> buttons;
};