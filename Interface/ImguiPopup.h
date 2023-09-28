#include "imgui.h"
#include <string>
#include <vector>

class ImGui::MyPopup {
public:
	int MessageBox(); // main popup function to be called by others, should toggle a popup, set it's message, define buttons and if available return the button pressed
protected:
	int returnValue;
	std::string message;
	std::string title;
	std::vector<std::string> buttons;
	bool drawn;
	void DrawMessageBox();
	enum button { // most common responses and buttons
		NotDrawn = -3,
		DrawnNoResponse = -2,
		Error = -1,
		Cancel = 0,
		Ok = 1,
	};
};