/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp\GLList.h"

extern GLApplication *theApp;

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
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
	processList->SetColumnLabels((char **)plName);
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
		
	if ((processList->GetNbRow() - 1) != worker->ontheflyParams.nbProcess) RefreshNbProcess();

		char tmp[512];
		PROCESS_INFO pInfo;
		size_t  states[MAX_PROCESS];
		std::vector<std::string> statusStrings(MAX_PROCESS);

		memset(states,0,MAX_PROCESS*sizeof(int));
		worker->GetProcStatus(states,statusStrings);

		processList->ResetValues();

		//Interface
		DWORD currpid = GetCurrentProcessId();
		GetProcInfo(currpid, &pInfo);
		processList->SetValueAt(0, 0, "Interface");
		sprintf(tmp, "%.0f MB", (double)pInfo.mem_use/(1024.0*1024.0));
		processList->SetValueAt(1, 0, tmp);

		
		for(size_t i=0;i<worker->GetProcNumber();i++) {
			DWORD pid = worker->GetPID(i);
			sprintf(tmp,"Subproc.%zd",i+1);
			processList->SetValueAt(0,i+1,tmp);
			if( !GetProcInfo(pid,&pInfo) ) {
				processList->SetValueAt(1,i+1,"0 MB");
				processList->SetValueAt(2,i+1,"Dead");
			} else {
				sprintf(tmp, "%.0f MB", (double)pInfo.mem_use / (1024.0*1024.0));
				processList->SetValueAt(1, i+1, tmp);
				// State/Status
				strncpy(tmp, statusStrings[i].c_str(), 127); tmp[127] = 0;
				processList->SetValueAt(2,i+1,tmp);

			}
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
	}
}
