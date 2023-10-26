#include "ImguiWindowBase.h"
#include "imgui.h"

void ImWindow::Show()
{
	drawn = true;
	if (txtW == 0 || txtH == 0) {
		txtW = ImGui::CalcTextSize("0").x;
		txtH = ImGui::GetTextLineHeightWithSpacing();
	}
}

void ImWindow::Hide()
{
	drawn = false;
}

void ImWindow::Toggle()
{
	if (drawn) Hide();
	else Show();
}

const bool ImWindow::IsVisible() {
	return drawn;
}

void ImWindow::SetVisible(bool value)
{
	drawn = value;
}
