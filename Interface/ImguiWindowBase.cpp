#include "ImguiWindowBase.h"

void ImWindow::Show()
{
	drawn = true;
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
