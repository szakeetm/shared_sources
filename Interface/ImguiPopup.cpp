#include "ImguiPopup.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include <exception>
#include <memory>
#include "ImguiExtensions.h"
#include <functional>
#include "ImguiWindow.h"
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif
extern char fileSaveFilters[];
#if defined(SYNRAD)
extern SynRad* mApp;
#include "../src/SynRad.h"
#endif

namespace WrappersIO {

	bool DoSave() {
		std::string fn = NFD_SaveFile_Cpp(fileSaveFilters, "");
		if (!fn.empty()) {
			try {
				mApp->imWnd->progress.Show();
				LockWrapper myLock(mApp->imguiRenderLock);
				mApp->worker.SaveGeometry(fn, mApp->imWnd->progress);
				mApp->imWnd->progress.Hide();
				mApp->changedSinceSave = false;
				mApp->UpdateTitle();
				mApp->AddRecent(fn);
			}
			catch (const std::exception& e) {
				std::string errMsg = ("%s\nFile:%s", e.what(), fn.c_str());
				mApp->imWnd->popup.Open("Error", errMsg, { std::make_shared<WrappersIO::MyButtonInt>("OK", WrappersIO::buttonOk) });
				mApp->RemoveRecent(fn.c_str());
			}
		}
		if (fn == "") return false;
		return true;
	}

	void MyPopup::Open(std::string title_, std::string message_, std::vector <std::shared_ptr< MyButton >> buttons_)
	{
		if (this->returnValue == drawnNoResponse) { // already drawing
			return;
		}
		else if (!this->drawn && this->returnValue == notDrawn) { // not drawing
			// initialzie
			this->title = title_;
			this->message = message_;
			this->buttons = buttons_;
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
			ImGui::TextWrapped(message);
			for (int i = 0; i < buttons.size(); i++) { // go over all the buttons on the list
				if (buttons.at(i)->key == SDL_SCANCODE_RETURN && buttons.at(i)->key2 == -1) buttons.at(i)->key2 = SDL_SCANCODE_KP_ENTER;
				if (ImGui::Button(("  " + (buttons.at(i)->name) + "  ").c_str()) || io.KeysDown[buttons.at(i)->key] || io.KeysDown[buttons.at(i)->key2]) { // draw them
					buttons.at(i)->DoCall(); // call the function
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
		this->title = "";
		this->message = "";
		this->buttons = std::vector<std::shared_ptr< MyButton >>();
	}

	// MyButton methods

	MyButtonInt::MyButtonInt(std::string name_, int retVal_, int key_, int key2_) {
		this->name = name_;
		this->retVal = retVal_;
		this->key = key_;
		this->key2 = key2_;
		if (this->key == SDL_SCANCODE_RETURN && this->key2 == -1) this->key2 = SDL_SCANCODE_KP_ENTER;
	}

	MyButtonFunc::MyButtonFunc(std::string name_, std::function<void()> func_, int key_, int key2_) {
		this->name = name_;
		this->function = func_;
		this->key = key_;
		this->key2 = key2_;
		this->retVal = buttonFunction;
		if (this->key == SDL_SCANCODE_RETURN && this->key2 == -1) this->key2 = SDL_SCANCODE_KP_ENTER;
	}

	void MyButtonFunc::DoCall() {
		return this->function();
	}

	MyButtonFuncStr::MyButtonFuncStr(std::string name_, std::function<void(std::string)> func_, std::string arg_, int key_, int key2_) {
		this->name = name_;
		this->function = func_;
		this->argument = arg_;
		this->key = key_;
		this->key2 = key2_;
		this->retVal = buttonFunction;
		if (this->key == SDL_SCANCODE_RETURN && this->key2 == -1) this->key2 = SDL_SCANCODE_KP_ENTER;
	}

	void MyButtonFuncStr::DoCall() {
		return this->function(argument);
	}

	MyButtonFuncInt::MyButtonFuncInt(std::string name_, std::function<void(int)> func, int arg, int key_, int key2_) {
		this->name = name_;
		this->function = func;
		this->argument = arg;
		this->key = key_;
		this->key2 = key2_;
		this->retVal = buttonFunction;
		if (this->key == SDL_SCANCODE_RETURN && this->key2 == -1) this->key2 = SDL_SCANCODE_KP_ENTER;
	}

	void MyButtonFuncInt::DoCall() {
		return this->function(argument);
	}

	void MyInput::Open(std::string title_, std::string message_, void (*func)(std::string), std::string deafultVal) {
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
			if (ImGui::Button("  OK  ") || io.KeysDown[SDL_SCANCODE_RETURN] || io.KeysDown[SDL_SCANCODE_KP_ENTER]) {
				ImGui::CloseCurrentPopup();
				this->drawn = false;
				this->returnValue = buttonFunction;
				function(this->value);
			} ImGui::SameLine();
			if (ImGui::Button("  Cancel  ") || io.KeysDown[SDL_SCANCODE_ESCAPE]) {
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
		mApp->imWnd->popup.Open(title, msg, { std::make_shared<MyButtonInt>("Ok", buttonOk,SDL_SCANCODE_RETURN, SDL_SCANCODE_KP_ENTER) });
	}
	void AskToSaveBeforeDoing(std::function<void()> action=0)
	{
		if (!mApp->changedSinceSave) { // there were no changes, do not ask, just do - Note: this will happen inline so action will not go out of scope
			action();
		} else {
			auto Y = [action]() { if (DoSave()) action(); }; // note: this can happen at an arbitrary time during execution
			mApp->imWnd->popup.Open("File not saved", "Save current geometry?", {
				std::make_shared<MyButtonFunc>("Yes", Y, SDL_SCANCODE_RETURN, SDL_SCANCODE_KP_ENTER), // save, then do action
				std::make_shared<MyButtonFunc>("No", action), // just do the action
				std::make_shared<MyButtonInt>("Cancel", buttonCancel, SDL_SCANCODE_ESCAPE) // do nothing
				});
		}
	}
}