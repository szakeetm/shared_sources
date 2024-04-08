
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

	cancelButton = new GLButton(0,"Stop waiting (keep clicking to capture)");
	cancelButton->SetEnabled(false); //Have to be explicitly enabled by cancellable process
	cancelButton->SetVisible(false);
	Add(cancelButton);

	//RefreshNbProcess(); //Determines size, position and processList nbRows, position and cancelButton position

	RestoreDeviceObjects();
	lastUpd = SDL_GetTicks();
}

void LoadStatus::EnableStopButton() {
	cancelButton->SetVisible(true);
	cancelButton->SetEnabled(true);
}

void LoadStatus::RefreshNbProcess()
{
	size_t nbProc = procStateCache.threadInfos.size();
	int wD = 550;
	int hD = 100 + (int)nbProc * 15;
	processList->SetSize(3, nbProc + 1);
	processList->SetColumnWidths((int*)plWidth);
	processList->SetColumnLabels(plName);
	processList->SetColumnAligns((int *)plAligns);
	processList->SetBounds(7, 8, wD - 17, hD - 55);
	cancelButton->SetBounds(wD / 2 - 100, hD - 43, 200, 19);
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
	size_t nbProc = procStateCache.threadInfos.size();
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

	processList->SetValueAt(0, 0, "SimManager");
	processList->SetValueAt(1, 0, simCommandStrings[procStateCache.masterCmd]);
	processList->SetValueAt(2, 0, fmt::format("[{}] {}", controllerStateStrings[procStateCache.controllerState], procStateCache.controllerStatus));

	size_t i = 1;
	for (auto& proc : procStateCache.threadInfos)
	{
		DWORD pid = proc.threadId;
		processList->SetValueAt(0, i, fmt::format("Thread {}", i));
		processList->SetValueAt(1, i, threadStateStrings[procStateCache.threadInfos[i - 1].threadState]);
		processList->SetValueAt(2, i, procStateCache.threadInfos[i - 1].threadStatus);

		++i;
	}
	Uint32 now = SDL_GetTicks();
	int delay_ms= IsVisible() ? 250 : 500; //Update frequency vs. initial display delay
	if ((now - lastUpd) > delay_ms) {
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
