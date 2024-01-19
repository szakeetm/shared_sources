#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiPopup.h"
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

namespace ImIOWrappers {

	bool DoSave() {
		std::string fn = mApp->worker.GetCurrentFileName();
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
				mApp->imWnd->popup.Open("Error", errMsg, { std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk) });
				mApp->RemoveRecent(fn.c_str());
			}
		}
		if (fn == "") {
			LockWrapper myLock(mApp->imguiRenderLock);
			mApp->SaveFileAs();
		}
		return true;
	}

	void ImPopup::Open(const std::string& title_, const std::string& message_, const std::vector<std::shared_ptr< ImButton >>& buttons_)
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

	bool ImPopup::WasResponse() {
		return (returnValue > popupError);
	}

	int ImPopup::GetResponse() {
		int storage = returnValue;
		returnValue = notDrawn;
		return storage;
	}

	void ImPopup::Draw()
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
				if (buttons.at(i)->key == ImGuiKey_Enter && buttons.at(i)->key2 == -1) buttons.at(i)->key2 = ImGuiKey_KeypadEnter;
				if (ImGui::Button(("  " + (buttons.at(i)->name) + "  ").c_str()) || ImGui::IsKeyPressed(buttons.at(i)->key) || ImGui::IsKeyPressed(buttons.at(i)->key2)) { // draw them
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

	void ImPopup::Close() {
		this->title = "";
		this->message = "";
		this->buttons = std::vector<std::shared_ptr< ImButton >>();
	}

	// ImButton methods

	ImButtonInt::ImButtonInt(const std::string& name_, int retVal_, ImGuiKey key_, ImGuiKey key2_) {
		this->name = name_;
		this->retVal = retVal_;
		this->key = key_;
		this->key2 = key2_;
		if (this->key == ImGuiKey_Enter && this->key2 == -1) this->key2 = ImGuiKey_KeypadEnter;
	}

	ImButtonFunc::ImButtonFunc(const std::string& name_, const std::function<void()>& func_, ImGuiKey key_, ImGuiKey key2_) {
		this->name = name_;
		this->function = func_;
		this->key = key_;
		this->key2 = key2_;
		this->retVal = buttonFunction;
		if (this->key == ImGuiKey_Enter && this->key2 == -1) this->key2 = ImGuiKey_KeypadEnter;
	}

	void ImButtonFunc::DoCall() {
		return this->function();
	}

	ImButtonFuncStr::ImButtonFuncStr(const std::string& name_, const std::function<void(std::string)>& func_, const std::string& arg_, ImGuiKey key_, ImGuiKey key2_) {
		this->name = name_;
		this->function = func_;
		this->argument = arg_;
		this->key = key_;
		this->key2 = key2_;
		this->retVal = buttonFunction;
		if (this->key == ImGuiKey_Enter && this->key2 == -1) this->key2 = ImGuiKey_KeypadEnter;
	}

	void ImButtonFuncStr::DoCall() {
		return this->function(argument);
	}

	ImButtonFuncInt::ImButtonFuncInt(const std::string& name_, const std::function<void(int)>& func, int arg, ImGuiKey key_, ImGuiKey key2_) {
		this->name = name_;
		this->function = func;
		this->argument = arg;
		this->key = key_;
		this->key2 = key2_;
		this->retVal = buttonFunction;
		if (this->key == ImGuiKey_Enter && this->key2 == -1) this->key2 = ImGuiKey_KeypadEnter;
	}

	void ImButtonFuncInt::DoCall() {
		return this->function(argument);
	}

	void ImInputPopup::Open(const std::string& title_, const std::string& message_, const std::function<void(std::string)> func_, const std::string& deafultArg_) {
		if (this->returnValue == drawnNoResponse) { // already drawing
			return;
		}
		else if (!this->drawn && this->returnValue == notDrawn) { // not drawing
			// initialzie
			this->value = "";
			this->title = title_;
			this->message = message_;
			this->drawn = true;
			this->returnValue = drawnNoResponse;
			this->function = func_;
			this->Draw();
			return;
		}
		else { // there was a response
			return;
		}
	}

	void ImInputPopup::Draw() {
		if (!drawn) { // return not drawn code
			returnValue = notDrawn;
			return;
		}
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(" ").x * 70, 0));
		if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(message.c_str()).x);
			ImGui::InputText(this->message.c_str(), &this->value);
			if (ImGui::Button("  OK  ") || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
				ImGui::CloseCurrentPopup();
				this->drawn = false;
				this->returnValue = buttonFunction;
				function(this->value);
			} ImGui::SameLine();
			if (ImGui::Button("  Cancel  ") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
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
	void InfoPopup(const std::string& title, const std::string& msg) {
		mApp->imWnd->popup.Open(title, msg, { std::make_shared<ImButtonInt>("Ok", buttonOk,ImGuiKey_Enter, ImGuiKey_KeypadEnter) });
	}
	void AskToSaveBeforeDoing(const std::function<void()>& action)
	{
		if (!mApp->changedSinceSave) { // there were no changes, do not ask, just do - Note: this will happen inline so action will not go out of scope
			action();
		} else {
			auto Y = [action]() { if (DoSave()) action(); }; // note: this can happen at an arbitrary time during execution
			mApp->imWnd->popup.Open("File not saved", "Save current geometry?", {
				std::make_shared<ImButtonFunc>("Yes", Y, ImGuiKey_Enter, ImGuiKey_KeypadEnter), // save, then do action
				std::make_shared<ImButtonFunc>("No", action), // just do the action
				std::make_shared<ImButtonInt>("Cancel", buttonCancel, ImGuiKey_Escape) // do nothing
				});
		}
	}
}