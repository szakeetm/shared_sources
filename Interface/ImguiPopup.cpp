#include "ImguiPopup.h"
#include "imgui_stdlib/imgui_stdlib.h"

MyPopup::MyPopup()
{
	this->returnValue = notDrawn;
	this->title = "";
	this->message = "";
	this->drawn = false;
	this->buttons = std::vector<button>();
}

void MyPopup::OpenImMsgBox(std::string title, std::string message, std::vector<button> buttons)
{
	if (this->returnValue == drawnNoResponse) { // already drawing
		return;
	} else if (!this->drawn && this->returnValue==notDrawn) { // not drawing
		// initialzie
		this->title = title;
		this->message = message;
		this->buttons = buttons;
		this->drawn = true;
		this->returnValue = drawnNoResponse;
		return;
	}
	else { // there was a response
		// reset
		this->returnValue = notDrawn;
		this->title = "";
		this->message = "";
		this->drawn = false;
		this->buttons = std::vector<button>();
		return;
	}
}

bool MyPopup::WasResponse() {
	return returnValue > popupError;
}

int MyPopup::GetResponse() {
	return returnValue;
}

void MyPopup::Draw()
{
	if (!drawn) { // return not drawn code
		returnValue = notDrawn;
		return;
	}
	ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(" ").x * 70, 0));
	if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		//this->returnValue = DrawnNoResponse; // make sure MessageBox returns DrawnNoRsponseCode by deafult
		ImGui::TextWrapped(message);
		for (int i = 0; i < buttons.size(); i++) { // go over all the buttons on the list
			if (ImGui::Button(("  " + (buttons.at(i).name) + "  ").c_str())) { // draw them
				returnValue = (buttons.at(i)).retVal; // if pressed change the return value
			} ImGui::SameLine();
		}
		if (returnValue != drawnNoResponse) { // if a button was pressed
			ImGui::CloseCurrentPopup(); // close popup in ImGui
			drawn = false; // and inside the object
		}
		ImGui::EndPopup();
	}
	else if (drawn) { // if was not open and is set to be drawn open it here
		// this must be here because ImGui::OpenPopup does not work when called between any ::Begin and ::End statements
		ImGui::OpenPopup(title.c_str());
	}
}
