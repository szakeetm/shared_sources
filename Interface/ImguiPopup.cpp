#include "ImguiPopup.h"
#include "imgui_stdlib/imgui_stdlib.h"

MyPopup::MyPopup()
{
	this->returnValue = NotDrawn;
	this->title = "";
	this->message = "";
	this->drawn = false;
	this->buttons = std::vector<button>();
}

int MyPopup::ImMsgBox(std::string title, std::string message, std::vector<button> buttons)
{
	if (this->returnValue == DrawnNoResponse) {
		return this->returnValue;
	} else if (!this->drawn && this->returnValue==NotDrawn) { // only do this if the popup us currently off
		this->title = title;
		this->message = message;
		this->buttons = buttons;
		//ImGui::OpenPopup(this->title.c_str());
		this->drawn = true;
		this->returnValue = DrawnNoResponse;
		return this->returnValue;
	}
	else {
		return this->returnValue;
		this->returnValue = NotDrawn;
		this->title = "";
		this->message = "";
		this->drawn = false;
		this->buttons = std::vector<button>();
	}
}

int MyPopup::GetResponse() {
	return this->returnValue;
}

void MyPopup::Draw()
{
	if (!this->drawn) { // return not drawn code
		this->returnValue = NotDrawn;
		return;
	}
	ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(" ").x * 70, 0));
	if (ImGui::BeginPopupModal(this->title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		//this->returnValue = DrawnNoResponse; // make sure MessageBox returns DrawnNoRsponseCode by deafult
		ImGui::TextWrapped(this->message);
		for (int i = 0; i < this->buttons.size(); i++) { // go over all the buttons on the list
			if (ImGui::Button(("  " + (this->buttons.at(i).name) + "  ").c_str())) { // draw them
				this->returnValue = (this->buttons.at(i)).retVal; // if pressed change the return value
			} ImGui::SameLine();
		}
		if (this->returnValue != DrawnNoResponse) { // if a button was pressed
			ImGui::CloseCurrentPopup(); // close popup in ImGui
			this->drawn = false; // and inside the object
		}
		ImGui::EndPopup();
	}
	else if (this->drawn) {
		ImGui::OpenPopup(this->title.c_str());
	}
}
