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

bool ImWindow::IsVisible() {
	return drawn;
}
