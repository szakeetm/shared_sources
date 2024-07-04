
#pragma once
#include "ImguiWindowBase.h"
#include <ProcessControl.h>

class Interface;
class ImGlobalSettings : public ImWindow {
public:
	void Draw();
protected:
	int nbProc = 0;
	ProcComm procInfo;
	size_t currPid = 0;
	double memDenominator_sys = 0.f;
	int lastUpdate = 0;
	double byte_to_mbyte = 0.f;
	PROCESS_INFO parentInfo;
	bool updateNbProc = true;
	void ProcessControlTable();
	void RecalculateOutgassing();
	void RestartProc();
};
