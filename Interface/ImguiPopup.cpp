#include "ImguiPopup.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include <exception>
#include <memory>

MyPopup::MyPopup()
{
	this->returnValue = notDrawn;
	this->title = "";
	this->message = "";
	this->drawn = false;
	this->buttons = std::vector<std::shared_ptr< MyButton >>();
}

void MyPopup::OpenImMsgBox(std::string title, std::string message, std::vector <std::shared_ptr< MyButton >> buttons)
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
	if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		//this->returnValue = DrawnNoResponse; // make sure MessageBox returns DrawnNoRsponseCode by deafult
		ImGui::TextWrapped(message);
		for (int i = 0; i < buttons.size(); i++) { // go over all the buttons on the list
			if (ImGui::Button(("  " + (buttons.at(i)->name) + "  ").c_str())) { // draw them
				if (buttons.at(i)->retVal == buttonFunction) {
					buttons.at(i)->DoCall(); // calls superclass, should call child class
				}
				
				returnValue = (buttons.at(i))->retVal; // if pressed change the return value
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

MyButtonInt::MyButtonInt(std::string name, int retVal)
{
	this->name = name;
	this->retVal = retVal;
}

MyButtonFunc::MyButtonFunc(std::string name, void (*func)())
{
	this->name = name;
	this->retVal = buttonFunction;
	this->function = func;
}

MyButtonFuncStr::MyButtonFuncStr(std::string name, void (*func)(std::string), std::string arg)
{
	this->name = name;
	this->retVal = buttonFunction;
	this->function = func;
	this->argument = arg;
}

void MyButtonFuncStr::DoCall() {
	return this->function(argument);
}

void MyButtonFunc::DoCall() {
	return this->function();
}