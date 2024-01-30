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
#include <sstream>
#include "GLApp/GLList.h"
#include "GLApp/GLLabel.h"
#include "GlobalSettings_shared.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLInputBox.h"
#include "Helper/MathTools.h"
#include "Interface/AppUpdater.h"
#include "SMP.h"
#include "ProcessControl.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#include "../../src/MolflowGeometry.h"
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#include "../../src/SynradGeometry.h"
extern SynRad*mApp;
#endif

static const int   plWidth[] = { 60,40,70,70,335 };
static const char *plName[] = { "#","PID","Mem Usage","Mem Peak",/*"CPU",*/"Status" };
static const int   plAligns[] = { ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT };

//HANDLE synradHandle;

GlobalSettingsBase::GlobalSettingsBase(Worker *w) : GLWindow(){
    worker = w;
}

void GlobalSettingsBase::Display(Worker *w) {
    worker = w;
    Update();
    SetVisible(true);
}

/**
* \brief Function to update the thread information table in the global settings window.
*/
void GlobalSettingsBase::UpdateProcessList() {
	{ //Frame limiter. Doing it here so "lastUpdate" is own member
		if (!IsVisible() || IsIconic()) return;
		int time = SDL_GetTicks();
		if (time - lastUpdate < 500) return;
	}

	size_t nb = worker->GetProcNumber();
	if (processList->GetNbRow() != (nb + 2)) processList->SetSize(5, nb + 2, true);

	ProcComm procInfo;
	worker->GetProcStatus(procInfo);

	//processList->ClearValues(); //Will be overwritten
	const double byte_to_mbyte = 1.0 / (1024.0 * 1024.0);
	//Interface
#ifdef _WIN32
	size_t currPid = GetCurrentProcessId();
	const double memDenominator_sys = (1024.0 * 1024.0);
#else
	size_t currPid = getpid();
	const double memDenominator_sys = (1024.0);
#endif
	PROCESS_INFO parentInfo{};
	GetProcInfo(currPid, &parentInfo);

	processList->SetValueAt(0, 0, "Interface");
	processList->SetValueAt(1, 0, fmt::format("{}", currPid), currPid);
	processList->SetValueAt(2, 0, fmt::format("{:.0f} MB", (double)parentInfo.mem_use / memDenominator_sys));
	processList->SetValueAt(3, 0, fmt::format("{:.0f} MB", (double)parentInfo.mem_peak / memDenominator_sys));
	processList->SetValueAt(4, 0, fmt::format("[Geom: {}]", worker->model->sh.name));

	processList->SetValueAt(0, 1, "SimManager");
	processList->SetValueAt(2, 1, fmt::format("{:.0f} MB", (double)worker->model->memSizeCache * byte_to_mbyte));
	processList->SetValueAt(4, 1, worker->GetSimManagerStatus());

	size_t i = 2;
	for (auto& proc : procInfo.threadInfos)
	{
		processList->SetValueAt(0, i, fmt::format("Thread {}", i - 1));
		processList->SetValueAt(1, i, fmt::format("")); //placeholder for thread id
		processList->SetValueAt(2, i, fmt::format("{:.0f} MB", (double)proc.runtimeInfo.counterSize * byte_to_mbyte));
		processList->SetValueAt(3, i, ""); //mem peak placeholder
		processList->SetValueAt(4, i, fmt::format("[{}] {}", threadStateStrings[proc.threadState], proc.threadStatus));
		++i;
	}
	lastUpdate = SDL_GetTicks();
}


/**
* \brief Function to apply changes to the number of processes.
*/
void GlobalSettingsBase::RestartProc() {

    int nbProc;
    if (!nbProcText->GetNumberInt(&nbProc)) {
        GLMessageBox::Display("Invalid process number", "Error", GLDLG_OK, GLDLG_ICONERROR);
    }
    else {
        if(nbProc<=0 || nbProc>MAX_PROCESS) {
            GLMessageBox::Display("Invalid process number [1..64]","Error",GLDLG_OK,GLDLG_ICONERROR);
        } else {
            try {
                worker->Stop_Public();
                worker->SetProcNumber(nbProc);
                worker->RealReload(true);
                mApp->SaveConfig();
            } catch (const std::exception &e) {
                GLMessageBox::Display(e.what(), "Error", GLDLG_OK, GLDLG_ICONERROR);
            }
        }
    }
}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void GlobalSettingsBase::ProcessMessage_shared(GLComponent *src, int message) {

    switch(message) {
        case MSG_BUTTON:

            if (src==restartButton) {
                RestartProc();
            }
            else if (src==desLimitButton) {
                if( worker->GetGeometry()->IsLoaded() ) {
                    char tmp[128];
                    sprintf(tmp,"%g",(double)worker->model->otfParams.desorptionLimit);
                    char *val = GLInputBox::GetInput(tmp,"Desorption limit (0 = infinite)","Edit desorption limit");
                    if (val) {
                        char* endptr;
                        long double maxDes_double = strtod(val,&endptr); // use double function to allow exponential format
                        constexpr long double max_sizet_double = static_cast<long double>(std::numeric_limits<size_t>::max());
                        if (val==endptr) {
                            GLMessageBox::Display("Invalid desorption limit", "Error", GLDLG_OK, GLDLG_ICONERROR);
                        }
                        else if (maxDes_double < 0.0) {
                            GLMessageBox::Display("Des. limit must be non-negative", "Error", GLDLG_OK, GLDLG_ICONERROR);
                        }
                        else if (maxDes_double > max_sizet_double) {
                            GLMessageBox::Display(fmt::format("Des. limit must be smaller than {:.6g}", max_sizet_double).c_str(), "Error", GLDLG_OK, GLDLG_ICONERROR);
                        }
                        else {
                            worker->model->otfParams.desorptionLimit = static_cast<size_t>(maxDes_double);
                            worker->ChangeSimuParams(); //Sync with subprocesses
                            UpdateDesLimitButtonText();
                        }
                    }
                } else {
                    GLMessageBox::Display("No geometry loaded.","No geometry",GLDLG_OK,GLDLG_ICONERROR);
                }
            }
            else if (src==applyButton) {
                
                mApp->antiAliasing = chkAntiAliasing->GetState();
                mApp->whiteBg = chkWhiteBg->GetState();
                mApp->highlightSelection = highlightSelectionToggle->GetState();
                if(mApp->highlightSelection)
                    worker->GetGeometry()->UpdateSelection();
                mApp->highlightNonplanarFacets = highlightNonplanarToggle->GetState();
                mApp->leftHandedView = (bool)leftHandedToggle->GetState();
                for (auto & i : mApp->viewers) {
                    i->UpdateMatrix();
                    i->UpdateLabelColors();
                }

                mApp->wereEvents = true;
                bool updateCheckPreference = chkCheckForUpdates->GetState();
                if (mApp->appUpdater) {
                    if (mApp->appUpdater->IsUpdateCheckAllowed() != updateCheckPreference) {
                        mApp->appUpdater->SetUserUpdatePreference(updateCheckPreference);
                    }
                }

                mApp->compressSavedFiles = chkCompressSavedFiles->GetState();
                mApp->autoSaveSimuOnly = chkSimuOnly->GetState();

                double cutoffnumber;
                if (!cutoffText->GetNumber(&cutoffnumber) || !(cutoffnumber>0.0 && cutoffnumber<1.0)) {
                    GLMessageBox::Display("Invalid cutoff ratio, must be between 0 and 1", "Error", GLDLG_OK, GLDLG_ICONWARNING);
                    return;
                }

                if (!IsEqual(worker->model->otfParams.lowFluxCutoff, cutoffnumber) || (int)worker->model->otfParams.lowFluxMode != lowFluxToggle->GetState()) {
                    worker->model->otfParams.lowFluxCutoff = cutoffnumber;
                    worker->model->otfParams.lowFluxMode = lowFluxToggle->GetState();
                    worker->ChangeSimuParams();
                }

                double autosavefreq;
                if (!autoSaveText->GetNumber(&autosavefreq) || !(autosavefreq > 0.0)) {
                    GLMessageBox::Display("Invalid autosave frequency", "Error", GLDLG_OK, GLDLG_ICONERROR);
                    return;
                }
                mApp->autoSaveFrequency = autosavefreq;
                //GLWindow::ProcessMessage(NULL,MSG_CLOSE);
                return;
            }
            break;

        case MSG_TEXT:
            ProcessMessage(applyButton, MSG_BUTTON);
            break;


        case MSG_TOGGLE:
            if (src == lowFluxToggle) {
                cutoffText->SetEditable(lowFluxToggle->GetState());
            }
            /*
            else if (src == prioToggle) {
                worker->ChangePriority(prioToggle->GetState());
            }
            */
            break;
    }
    //GLWindow::ProcessMessage(src, message); //Let molflow/synrad child functions Gloablsettings::ProcessMessage call them
}

/**
* \brief Function to change global settings values (may happen due to change or load of file etc.)
*/
void GlobalSettingsBase::Update_shared() {
    chkAntiAliasing->SetState(mApp->antiAliasing);
    chkWhiteBg->SetState(mApp->whiteBg);
    highlightSelectionToggle->SetState(mApp->highlightSelection);
    leftHandedToggle->SetState(mApp->leftHandedView);


    cutoffText->SetText(worker->model->otfParams.lowFluxCutoff);
    cutoffText->SetEditable(worker->model->otfParams.lowFluxMode);
    lowFluxToggle->SetState(worker->model->otfParams.lowFluxMode);
    highlightNonplanarToggle->SetState(mApp->highlightNonplanarFacets);

    autoSaveText->SetText(mApp->autoSaveFrequency);
    chkSimuOnly->SetState(mApp->autoSaveSimuOnly);
    if (mApp->appUpdater) { //Updater initialized
        chkCheckForUpdates->SetState(mApp->appUpdater->IsUpdateCheckAllowed());
    }
    else {
        chkCheckForUpdates->SetState(0);
        chkCheckForUpdates->SetEnabled(false);
    }
    chkCompressSavedFiles->SetState(mApp->compressSavedFiles);
    nbProcText->SetText(fmt::format("{}", worker->GetProcNumber()));
}

void GlobalSettingsBase::UpdateDesLimitButtonText(){
    std::ostringstream desLimitLabel;
    desLimitLabel << "Desorption limit: ";
    if (worker->model->otfParams.desorptionLimit == 0) {
        desLimitLabel << "infinite";
    }
    else {
        desLimitLabel << (double)worker->model->otfParams.desorptionLimit; //Large numbers as floating point
    }
    desLimitButton->SetText(desLimitLabel.str().c_str());
}