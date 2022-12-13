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
    lastUpdate = 0;
}

void GlobalSettingsBase::Display(Worker *w) {
    worker = w;
    Update();
    SetVisible(true);
}

/**
* \brief Function to update the thread information table in the global settings window.
*/
void GlobalSettingsBase::SMPUpdate() {

    int time = SDL_GetTicks();

    if (!IsVisible() || IsIconic()) return;
    size_t nb = worker->GetProcNumber();
    if (processList->GetNbRow() != (nb + 1)) processList->SetSize(5, nb + 1,true);

    if( time-lastUpdate>333 ) {

        ProcComm procInfo;
        worker->GetProcStatus(procInfo);

        processList->ResetValues();

        //Interface
#ifdef _WIN32
        size_t currPid = GetCurrentProcessId();
        double memDenominator = (1024.0 * 1024.0);
#else
        size_t currPid = getpid();
        double memDenominator = (1024.0);
#endif
        PROCESS_INFO parentInfo{};
        GetProcInfo(currPid, &parentInfo);

        processList->SetValueAt(0, 0, "Interface");
        processList->SetValueAt(1, 0, fmt::format("{}", currPid), currPid);
        processList->SetValueAt(2, 0, fmt::format("{:.0f} MB", (double)parentInfo.mem_use / memDenominator));
        processList->SetValueAt(3, 0, fmt::format("{:.0f} MB", (double)parentInfo.mem_peak / memDenominator));
        processList->SetValueAt(4, 0, fmt::format("[Geom: {}]", worker->model->sh.name));

        size_t i = 1;
        for (auto& proc : procInfo.subProcInfo)
        {
            DWORD pid = proc.procId;
            processList->SetValueAt(0, i, fmt::format("Thread {}", i));
            processList->SetValueAt(1, i, ""); //placeholder for thread id
            processList->SetValueAt(2, i, ""); //placeholder for memory
            processList->SetValueAt(3, i, ""); //placeholder for memory
            processList->SetValueAt(4, i, fmt::format("[{}] {}", prStates[procInfo.subProcInfo[i - 1].slaveState], procInfo.subProcInfo[i - 1].statusString));
            ++i;
        }
        lastUpdate = SDL_GetTicks();
    }
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
            else if (src==maxButton) {
                if( worker->GetGeometry()->IsLoaded() ) {
                    char tmp[128];
                    sprintf(tmp,"%zd",worker->model->otfParams.desorptionLimit);
                    char *val = GLInputBox::GetInput(tmp,"Desorption max (0=>endless)","Edit MAX");
                    if (val) {
                        char* endptr;
                        size_t maxDes = strtold(val,&endptr); // use double function to allow exponential format
                        if (val==endptr) {
                            GLMessageBox::Display("Invalid 'maximum desorption' number", "Error", GLDLG_OK, GLDLG_ICONERROR);
                        }
                        else {
                            worker->model->otfParams.desorptionLimit = maxDes;
                            worker->ChangeSimuParams(); //Sync with subprocesses
                        }
                    }
                } else {
                    GLMessageBox::Display("No geometry loaded.","No geometry",GLDLG_OK,GLDLG_ICONERROR);
                }
            }
            else if (src==applyButton) {
                mApp->useOldXMLFormat = useOldXMLFormat->GetState();
                mApp->antiAliasing = chkAntiAliasing->GetState();
                mApp->whiteBg = chkWhiteBg->GetState();
                mApp->highlightSelection = highlightSelectionToggle->GetState();
                if(mApp->highlightSelection)
                    worker->GetGeometry()->UpdateSelection();
                mApp->highlightNonplanarFacets = highlightNonplanarToggle->GetState();
                mApp->leftHandedView = (bool)leftHandedToggle->GetState();
                for (auto & i : mApp->viewer) {
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

                mApp->autoUpdateFormulas = chkAutoUpdateFormulas->GetState();
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

                GLWindow::ProcessMessage(NULL,MSG_CLOSE);
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
            else if (src == prioToggle) {
                worker->ChangePriority(prioToggle->GetState());
            }
            break;
    }
    GLWindow::ProcessMessage(src, message);
}

/**
* \brief Function to change global settings values (may happen due to change or load of file etc.)
*/
void GlobalSettingsBase::Update_shared() {
    char tmp[256];
    chkAntiAliasing->SetState(mApp->antiAliasing);
    chkWhiteBg->SetState(mApp->whiteBg);
    highlightSelectionToggle->SetState(mApp->highlightSelection);
    leftHandedToggle->SetState(mApp->leftHandedView);


    cutoffText->SetText(worker->model->otfParams.lowFluxCutoff);
    cutoffText->SetEditable(worker->model->otfParams.lowFluxMode);
    lowFluxToggle->SetState(worker->model->otfParams.lowFluxMode);

    //syn
    //chkNewReflectionModel->SetState(worker->model->wp.newReflectionModel);

    // mol?
    //highlightNonplanarToggle->SetState(mApp->highlightNonplanarFacets);

    autoSaveText->SetText(mApp->autoSaveFrequency);
    chkSimuOnly->SetState(mApp->autoSaveSimuOnly);
    if (mApp->appUpdater) { //Updater initialized
        chkCheckForUpdates->SetState(mApp->appUpdater->IsUpdateCheckAllowed());
    }
    else {
        chkCheckForUpdates->SetState(0);
        chkCheckForUpdates->SetEnabled(false);
    }
    chkAutoUpdateFormulas->SetState(mApp->autoUpdateFormulas);
    chkCompressSavedFiles->SetState(mApp->compressSavedFiles);

    size_t nb = worker->GetProcNumber();
    sprintf(tmp, "%zd", nb);
    nbProcText->SetText(tmp);
}
