#pragma once
#include <string>
#include "imgui.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"

namespace ImguiPopup {
	void Popup(std::string msg);
	void Popup(std::string msg, std::string title);
	void ShowPopup();
	
	static bool showPopup{ false };
	static std::string message{" "};
	static std::string title{" "};
	static struct {
		float posX, posY, sizeX, sizeY;
	} window;
};