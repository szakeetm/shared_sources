#pragma once
#include "imgui.h"
#include <string>
#include <vector>
#include "GLApp.h"

enum PopupCode: int { // most common responses and buttons
	notDrawn		= -3, // the popup is inactive
	drawnNoResponse = -2, // the popup is visible but no user input is registered
	popupError		= -1, // values higher than this represent responses
	buttonCancel    =  1,
	buttonOk		=  2,
	buttonYes		=  3,
	buttonNo		=  4,
	buttonFunction  =  100
};

class MyButton {
public:
	std::string name;
	int retVal; // PopupCode enum contains reserved values
	virtual void DoCall() = 0;
};

class MyButtonFunc : public MyButton {
public:
	MyButtonFunc(std::string name, void (*func)());
	void DoCall() override;
protected:
	void (*function)();
};

class MyButtonFuncStr : public MyButton {
public:
	MyButtonFuncStr(std::string name, void (*func)(std::string), std::string arg);
	void DoCall() override;
protected:
	void (*function)(std::string);
	std::string argument;
};

class MyButtonInt : public MyButton {
public:
	MyButtonInt(std::string name, int retVal);
	void DoCall() {};
};

class MyPopup {
public:
	MyPopup();

	void OpenImMsgBox(std::string title, std::string message, std::vector<std::shared_ptr< MyButton >> buttons); // main popup function to be called by others, should toggle a popup, set it's message, define buttons and if available return the button pressed
	void Draw(); // call this every ImGui Render
	bool WasResponse();
	int GetResponse(); // returns the recorded value from a button press
protected:
	int returnValue;
	std::string message;
	std::string title;
	bool wasClicked;
	bool drawn;
	std::vector<std::shared_ptr< MyButton >> buttons;
};