
#pragma once
#include "ImguiWindowBase.h"

class Interface;
class ImGlobalSettings : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
protected:
	Interface* mApp;
	int nbProc;
	bool updateNbProc = true;
	void ProcessControlTable();
	void RecalculateOutgassing();
	void RestartProc();
};
