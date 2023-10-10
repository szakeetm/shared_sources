#pragma once
#include "imgui.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace WrappersIO {

	void InfoPopup(std::string title, std::string msg);

	enum PopupCode : int { // most common responses and buttons
		notDrawn = -3, // the popup is inactive
		drawnNoResponse = -2, // the popup is visible but no user input is registered
		popupError = -1, // values higher than this represent responses
		buttonCancel = 1,
		buttonOk = 2,
		buttonYes = 3,
		buttonNo = 4,
		buttonFunction = 100, // use this to indicate the button is not meant to return a value but execute a function call instead
		closeMe = -100
	};

	class MyButton {
	public:
		std::string name;
		int retVal;
		int key;
		int key2;
		virtual void DoCall() = 0;
	};

	class MyButtonFunc : public MyButton {
	public:
		MyButtonFunc(std::string name, std::function<void()> func, int key = -1, int key2 = -1);
		void DoCall() override;
		int retVal = buttonFunction;
	protected:
		std::function<void()> function;
	};

	class MyButtonFuncStr : public MyButton {
	public:
		MyButtonFuncStr(std::string name, std::function<void(std::string)> func, std::string arg, int key = -1, int key2 = -1);
		void DoCall() override;
		int retVal = buttonFunction;
	protected:
		std::function<void(std::string)> function;
		std::string argument;
	};

	class MyButtonFuncInt : public MyButton {
	public:
		MyButtonFuncInt(std::string name, std::function<void(int)> func, int arg, int key = -1, int key2 = -1);
		void DoCall() override;
		int retVal = buttonFunction;
	protected:
		std::function<void(int)> function;
		int argument;
	};

	class MyButtonInt : public MyButton {
	public:
		MyButtonInt(std::string name, int retVal=0, int key = -1, int key2 = -1);
		void DoCall() {};
	};

	class MyPopup {
	public:
		void Close();
		void Open(std::string title, std::string message, std::vector<std::shared_ptr< MyButton >> buttons); // main popup function to be called by others, should toggle a popup, set it's message, define buttons and if available return the button pressed
		void Draw(); // call this every ImGui Render
		bool WasResponse();
		int GetResponse(); // returns the recorded value from a button press
	protected:
		int returnValue = notDrawn;
		std::string message = "";
		std::string title = "";
		bool drawn = false;
		std::vector<std::shared_ptr< MyButton >> buttons;
	};

	class MyInput : public MyPopup {
	public:
		void Open(std::string title, std::string message, void (*func)(std::string), std::string deafultArg = "");
		void Draw();
	protected:
		void (*function)(std::string);
		std::string value;
	};
}