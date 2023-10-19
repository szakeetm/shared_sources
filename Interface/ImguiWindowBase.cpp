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
	drawn = !drawn;
}

bool ImWindow::IsVisible() {
	return drawn;
}
