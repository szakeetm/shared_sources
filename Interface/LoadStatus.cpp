/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#include "LoadStatus.h"
#include "GLApp/GLToolkit.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLList.h"
#include "SMP.h"

#ifndef _WIN32
//getpid in linux
#include <sys/types.h>
#include <unistd.h>
#endif

extern GLApplication *theApp;

#if defined(MOLFLOW)
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
extern SynRad*mApp;
#endif

static const int   plWidth[] = {60,70,300};
static const char *plName[] = {"#","Mem Usage","Status"};
static const int   plAligns[] = { ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT };

LoadStatus::LoadStatus(Worker* w):GLWindow() {

	worker = w;

	SetTitle("Waiting for subprocesses...");
	SetIconfiable(true);

	processList = new GLList(0);
	processList->SetHScrollVisible(false);
	processList->SetVScrollVisible(false);
	processList->SetColumnLabelVisible(true);
	Add(processList);

	cancelButton = new GLButton(0,"Stop waiting");
	Add(cancelButton);

	//RefreshNbProcess(); //Determines size, position and processList nbRows, position and cancelButton position

	RestoreDeviceObjects();
}

void LoadStatus::EnableStopButton() {
	cancelButton->SetText("Stop waiting");
	cancelButton->SetEnabled(true);
}

void LoadStatus::RefreshNbProcess()
{
	int wD = 450;
	int hD = 100 + (int)worker->GetProcNumber() * 15;
	processList->SetSize(3, worker->GetProcNumber() + 1);
	processList->SetColumnWidths((int*)plWidth);
	processList->SetColumnLabels(plName);
	processList->SetColumnAligns((int *)plAligns);
	processList->SetBounds(7, 8, wD - 17, hD - 55);
	cancelButton->SetBounds(wD / 2 - 45, hD - 43, 90, 19);
	// Place dialog lower right
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = wS - wD - 215;
	int yD = hS - hD - 30;
	SetBounds(xD, yD, wD, hD);
}

LoadStatus::~LoadStatus()
{
	//SAFE_DELETE(processList);
}

void LoadStatus::SMPUpdate() {
		
	if ((processList->GetNbRow() - 1) != worker->model->otfParams.nbProcess) RefreshNbProcess();

    ProcComm procInfo;
    worker->GetProcStatus(procInfo);

		processList->ResetValues();

		//Interface
#ifdef _WIN32
		size_t currPid = GetCurrentProcessId();
		double memDenominator = (1024.0*1024.0);
#else
		size_t currPid = getpid();
		double memDenominator = (1024.0);
#endif
		PROCESS_INFO parentInfo{};
		GetProcInfo(currPid, &parentInfo);

		processList->SetValueAt(0, 0, "Interface");
		processList->SetValueAt(1, 0, fmt::format("{}", currPid), currPid);
		processList->SetValueAt(2, 0, fmt::format("{:.0f}", (double)parentInfo.mem_use / memDenominator));
		processList->SetValueAt(3, 0, fmt::format("{:.0f}", (double)parentInfo.mem_peak / memDenominator));
		processList->SetValueAt(4, 0, fmt::format("[Geom. {}]", worker->model->sh.name));

		size_t i = 1;
		for (auto& proc : procInfo.subProcInfo)
		{
			DWORD pid = proc.procId;
			processList->SetValueAt(0, i, fmt::format("Thread {}", i));
			processList->SetValueAt(1, i, ""); //placeholder for thread id
			processList->SetValueAt(2, i, ""); //placeholder for memory
			processList->SetValueAt(3, i, ""); //placeholder for memory
			processList->SetValueAt(4, i, fmt::format("[{}] {}", procInfo.subProcInfo[i - 1].slaveState, procInfo.subProcInfo[i - 1].slaveStatus));
			++i;
		}
}

void LoadStatus::ProcessMessage(GLComponent *src,int message) {
	switch (message) {
	case MSG_BUTTON:
		if (src == cancelButton) {
			cancelButton->SetText("Stopping...");
			cancelButton->SetEnabled(false);
			worker->abortRequested = true;
		}
	case MSG_CLOSE:
			cancelButton->SetText("Stopping...");
			cancelButton->SetEnabled(false);
			worker->abortRequested = true;
			break;
	}
}
