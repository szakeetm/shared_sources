/*
File:        LoadStatus.cpp
Description: Subprocess load status
Program:     SynRad

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
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
	int wD = 450;
	int hD = 100+(int)worker->GetProcNumber()*15;

	SetTitle("Waiting for subprocesses...");
	SetIconfiable(true);

	processList = new GLList(0);
	processList->SetHScrollVisible(false);
	processList->SetVScrollVisible(false);
	processList->SetSize(5,worker->GetProcNumber()+1);
	processList->SetColumnWidths((int*)plWidth);
	processList->SetColumnLabels((char **)plName);
	processList->SetColumnAligns((int *)plAligns);
	processList->SetColumnLabelVisible(true);
	processList->SetBounds(7,8,wD-17,hD-55);
	Add(processList);

	cancelButton = new GLButton(0,"Stop waiting");
	cancelButton->SetBounds(wD/2-45,hD-43,90,19);
	Add(cancelButton);

	// Place dialog lower right
	int wS,hS;
	GLToolkit::GetScreenSize(&wS,&hS);
	int xD = wS-wD-215;
	int yD = hS-hD-30;
	SetBounds(xD,yD,wD,hD);

	RestoreDeviceObjects();
}

LoadStatus::~LoadStatus()
{
	//SAFE_DELETE(processList);
}

void LoadStatus::SMPUpdate() {

		char tmp[512];
		PROCESS_INFO pInfo;
		int  states[MAX_PROCESS];
		char statusStr[MAX_PROCESS][64];

		memset(states,0,MAX_PROCESS*sizeof(int));
		worker->GetProcStatus(states,(char **)statusStr);

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
				_snprintf(tmp,127,"%s: %s",prStates[states[i]],statusStr[i]);

				//if (states[i] == PROCESS_ERROR) processList->SetFontColor(255, 0, 0);
				//else if (states[i] == PROCESS_READY) processList->SetFontColor(0, 150, 0);
				processList->SetValueAt(2,i+1,tmp);
				//processList->SetFontColor(0, 0, 0);

			}
		}
}
// ----------------------------------------------------------------


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
