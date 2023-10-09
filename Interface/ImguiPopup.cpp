#include "ImguiPopup.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include <exception>
#include <memory>
#include "ImguiExtensions.h"
#include <functional>
#include "ImguiWindow.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

#if defined(SYNRAD)
extern SynRad* mApp;
#include "../src/SynRad.h"
#endif

namespace WrappersIO {

	void MyPopup::Open(std::string title, std::string message, std::vector <std::shared_ptr< MyButton >> buttons)
	{
		if (this->returnValue == drawnNoResponse) { // already drawing
			return;
		}
		else if (!this->drawn && this->returnValue == notDrawn) { // not drawing
			// initialzie
			this->title = title;
			this->message = message;
			this->buttons = buttons;
			this->drawn = true;
			this->returnValue = drawnNoResponse;
			return;
		}
		else { // there was a response
			return;
		}
	}

	bool MyPopup::WasResponse() {
		return (returnValue > popupError);
	}

	int MyPopup::GetResponse() {
		int storage = returnValue;
		returnValue = notDrawn;
		return storage;
	}

	void MyPopup::Draw()
	{
		if (!drawn) { // return not drawn code
			returnValue = notDrawn;
			return;
		}
		ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(" ").x * 70, 0));
		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			if (returnValue == closeMe) {
				ImGui::CloseCurrentPopup();
				ImGui::End();
				returnValue = notDrawn;
				this->Close();
				return;
			}
			ImGui::TextWrapped(message);
			for (int i = 0; i < buttons.size(); i++) { // go over all the buttons on the list
				if (buttons.at(i)->key == ImGui::keyEnter && buttons.at(i)->key2 == -1) buttons.at(i)->key2 = ImGui::keyNumEnter;
				if (ImGui::Button(("  " + (buttons.at(i)->name) + "  ").c_str()) || io.KeysDown[buttons.at(i)->key] || io.KeysDown[buttons.at(i)->key2]) { // draw them
					if (buttons.at(i)->retVal == buttonFunction) { // if the button is the  function type
						buttons.at(i)->DoCall(); // call the function
					}
					returnValue = (buttons.at(i))->retVal; // if pressed change the return value
				} ImGui::SameLine();
			}
			if (returnValue != drawnNoResponse) { // if any button was pressed
				ImGui::CloseCurrentPopup(); // close popup in ImGui
				this->Close();
				drawn = false; // and inside the object
			}
			ImGui::EndPopup();
		}
		else if (drawn) { // if was not open and is set to be drawn open it here
			// this must be here because ImGui::OpenPopup does not work when called between any ::Begin and ::End statements
			ImGui::OpenPopup(title.c_str());
			ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			this->Draw();
		}
	}

	void MyPopup::Close() {
		this->returnValue = closeMe;
		this->title = "";
		this->message = "";
		this->buttons = std::vector<std::shared_ptr< MyButton >>();
	}

	// MyButton methods

	MyButtonInt::MyButtonInt(std::string name, int retVal, int key, int key2) {
		this->name = name;
		this->retVal = retVal;
		this->key = key;
		this->key2 = key2;
		if (this->key == ImGui::keyEnter && this->key2 == -1) this->key2 = ImGui::keyNumEnter;
	}

	MyButtonFunc::MyButtonFunc(std::string name, std::function<void()> func, int key, int key2) {
		this->name = name;
		this->function = func;
		this->key = key;
		this->key2 = key2;
		if (this->key == ImGui::keyEnter && this->key2 == -1) this->key2 = ImGui::keyNumEnter;
	}

	void MyButtonFunc::DoCall() {
		return this->function();
	}

	MyButtonFuncStr::MyButtonFuncStr(std::string name, std::function<void(std::string)> func, std::string arg, int key, int key2) {
		this->name = name;
		this->function = func;
		this->argument = arg;
		this->key = key;
		this->key2 = key2;
	}

	void MyButtonFuncStr::DoCall() {
		return this->function(argument);
	}

	MyButtonFuncInt::MyButtonFuncInt(std::string name, std::function<void(int)> func, int arg, int key, int key2) {
		this->name = name;
		this->function = func;
		this->argument = arg;
		this->key = key;
		this->key2 = key2;
		if (this->key == ImGui::keyEnter && this->key2 == -1) this->key2 = ImGui::keyNumEnter;
	}

	void MyButtonFuncInt::DoCall() {
		return this->function(argument);
	}

	void MyInput::Open(std::string title, std::string message, void (*func)(std::string), std::string deafultVal) {
		if (this->returnValue == drawnNoResponse) { // already drawing
			return;
		}
		else if (!this->drawn && this->returnValue == notDrawn) { // not drawing
			// initialzie
			this->value = deafultVal;
			this->title = title;
			this->message = message;
			this->drawn = true;
			this->returnValue = drawnNoResponse;
			this->function = func;
			this->Draw();
			return;
		}
		else { // there was a response
			return;
		}
	}

	void MyInput::Draw() {
		if (!drawn) { // return not drawn code
			returnValue = notDrawn;
			return;
		}
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(" ").x * 70, 0));
		if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - ImGui::CalcTextSize(message.c_str()).x);
			ImGui::InputText(this->message.c_str(), &this->value);
			if (ImGui::Button("  OK  ") || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
				ImGui::CloseCurrentPopup();
				this->drawn = false;
				this->returnValue = buttonFunction;
				function(this->value);
			} ImGui::SameLine();
			if (ImGui::Button("  Cancel  ") || io.KeysDown[ImGui::keyEsc]) {
				ImGui::CloseCurrentPopup();
				this->drawn = false;
			}
			ImGui::EndPopup();
		}
		else if (drawn) { // if was not open and is set to be drawn open it here
			// this must be here because ImGui::OpenPopup does not work when called between any ::Begin and ::End statements
			ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::OpenPopup(title.c_str());
			this->Draw();
		}
	}
	void InfoPopup(std::string title, std::string msg) {
		mApp->imWnd->popup.Open(title, msg, { std::make_shared<MyButtonInt>("Ok", buttonOk,ImGui::keyEnter, ImGui::keyNumEnter) });
	}
}