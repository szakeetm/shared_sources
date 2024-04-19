#pragma once
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace ImIOWrappers {
	bool DoSave();
	void InfoPopup(const std::string& title, const std::string& msg);
	void AskToSaveBeforeDoing(const std::function<void()>& action=0);

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

	class ImButton {
	public:
		std::string name;
		int retVal;
		ImGuiKey key;
		ImGuiKey key2;
		virtual void DoCall() = 0;
	};

	class ImButtonFunc : public ImButton {
	public:
		ImButtonFunc(const std::string& name_, const std::function<void()>& func_, ImGuiKey key_ = ImGuiKey_None, ImGuiKey key2_ = ImGuiKey_None);
		void DoCall() override;
		int retVal = buttonFunction;
	protected:
		std::function<void()> function;
	};

	class ImButtonFuncStr : public ImButton {
	public:
		ImButtonFuncStr(const std::string& name_, const std::function<void(std::string)>& func_, const std::string& arg_, ImGuiKey key_ = ImGuiKey_None, ImGuiKey key2_ = ImGuiKey_None);
		void DoCall() override;
		int retVal = buttonFunction;
	protected:
		std::function<void(std::string)> function;
		std::string argument;
	};

	class ImButtonFuncInt : public ImButton {
	public:
		ImButtonFuncInt(const std::string& name_, const std::function<void(int)>& func, int arg, ImGuiKey key_= ImGuiKey_None, ImGuiKey key2_= ImGuiKey_None);
		void DoCall() override;
		int retVal = buttonFunction;
	protected:
		std::function<void(int)> function;
		int argument;
	};

	class ImButtonInt : public ImButton {
	public:
		ImButtonInt(const std::string& name_, int retVal_=0, ImGuiKey key_ = ImGuiKey_None, ImGuiKey key2_ = ImGuiKey_None);
		void DoCall() {};
	};

	class ImPopup {
	public:
		void Close();
		void Open(const std::string& title_, const std::string& message_, const std::vector<std::shared_ptr< ImButton >>& buttons_); // main popup function to be called by others, should toggle a popup, set it's message, define buttons and if available return the button pressed
		void Draw(); // call this every ImGui Render
		bool WasResponse();
		int GetResponse(); // returns the recorded value from a button press
	protected:
		int returnValue = notDrawn;
		std::string message = "";
		std::string title = "";
		bool drawn = false;
		std::vector<std::shared_ptr< ImButton >> buttons;
		friend class ImTest;
	};

	class ImInputPopup : public ImPopup {
	public:
		void Open(const std::string& title_,const std::string& message_, const std::function<void(std::string)> func_, const std::string& deafultArg_ = "");
		void Draw();
	protected:
		std::function<void(std::string)> function;
		std::string value;
		friend class ImTest;
	};
}