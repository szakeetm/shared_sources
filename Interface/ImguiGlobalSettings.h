
#pragma once
#include "ImguiWindowBase.h"
#include <ProcessControl.h>

class Interface;
class ImGlobalSettings : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
protected:
	Interface* mApp;
	int nbProc;
	ProcComm procInfo;
	size_t currPid;
	double memDenominator_sys;
	int lastUpdate;
	double byte_to_mbyte;
	PROCESS_INFO parentInfo;
	bool updateNbProc = true;
	void ProcessControlTable();
	void RecalculateOutgassing();
	void RestartProc();
};
