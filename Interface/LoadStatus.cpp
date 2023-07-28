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
#include "GLApp/GLWindowManager.h"
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

static const int   plWidth[] = {60,170,300};
static const char *plName[] = {"#","State","Status"};
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
	lastUpd = SDL_GetTicks();
}

void LoadStatus::EnableStopButton() {
	cancelButton->SetText("Stop waiting");
	cancelButton->SetEnabled(true);
}

void LoadStatus::RefreshNbProcess()
{
	size_t nbProc = procStateCache.subProcInfos.size();
	int wD = 550;
	int hD = 100 + (int)nbProc * 15;
	processList->SetSize(3, nbProc + 1);
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

void LoadStatus::Update() {
	//SimulationManager calls it, after it has updated its procStateCache
	auto guard = std::lock_guard(procStateCache.procDataMutex);
	size_t nbProc = procStateCache.subProcInfos.size();
	if ((processList->GetNbRow() - 1) != nbProc) {
		RefreshNbProcess();
	}

	processList->ClearValues(); //Zero out all

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

	processList->SetValueAt(0, 0, "Sim.Manager");
	processList->SetValueAt(1, 0, simCommandStrings.at(procStateCache.masterCmd));
	processList->SetValueAt(2, 0, procStateCache.masterStatus);

	size_t i = 1;
	for (auto& proc : procStateCache.subProcInfos)
	{
		DWORD pid = proc.procId;
		processList->SetValueAt(0, i, fmt::format("Thread {}", i));
		processList->SetValueAt(1, i, simStateStrings.at(procStateCache.subProcInfos[i - 1].slaveState));
		processList->SetValueAt(2, i, procStateCache.subProcInfos[i - 1].slaveStatus);

		++i;
	}
	Uint32 now = SDL_GetTicks();
	if ((now - lastUpd) > 500) {
		SetVisible(true);
		mApp->DoEvents(); //draw table and catch stop button press
		lastUpd = SDL_GetTicks();
	}
}

void LoadStatus::ProcessMessage(GLComponent *src,int message) {
	switch (message) {
	case MSG_BUTTON:
		if (src == cancelButton) {
			cancelButton->SetText("Stopping...");
			cancelButton->SetEnabled(false);
			abortRequested = true; //SimulationManager will handle it
		}
	case MSG_CLOSE:
			cancelButton->SetText("Stopping...");
			cancelButton->SetEnabled(false);
			abortRequested = true; //SimulationManager will handle it
			break;
	}
}
