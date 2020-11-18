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
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define NOMINMAX
//#include <Windows.h>
#include <Process.h>
#include <direct.h>
#else

#endif

#include "Worker.h"
#include "Facet_shared.h"
#include "GLApp/GLApp.h"
#include "GLApp/GLMessageBox.h"
#include "Helper/MathTools.h" //Min max
#include "GLApp/GLList.h"
#include <math.h>
#include <stdlib.h>
#include "GLApp/GLUnitDialog.h"
#include "Interface/LoadStatus.h"
#include "ProcessControl.h" // defines for process commands
#include "SimulationManager.h"
#include "Buffer_shared.h"

#if defined(MOLFLOW)
#include "../src/MolFlow.h"
#include "../src/MolflowGeometry.h"
#include "../src/FacetAdvParams.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#include "../src/SynradGeometry.h"
#endif

#include "File.h" //File utils (Get extension, etc)

/*
//Leak detection
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
*/
using namespace pugi;

#if defined(MOLFLOW)
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
extern SynRad*mApp;
#endif

Worker::~Worker() {
    delete geom;
}

Geometry *Worker::GetGeometry() {
    return geom;
}

char *Worker::GetCurrentFileName() {
    return fullFileName;
}

char *Worker::GetCurrentShortFileName() {

    static char ret[512];
    char *r = strrchr(fullFileName, '/');
    if (!r) r = strrchr(fullFileName, '\\');
    if (!r) strcpy(ret, fullFileName);
    else {
        r++;
        strcpy(ret, r);
    }

    return ret;

}

/*
char *Worker::GetShortFileName(char* longFileName) {

	static char ret[512];
	char *r = strrchr(longFileName, '/');
	if (!r) r = strrchr(longFileName, '\\');
	if (!r) strcpy(ret, longFileName);
	else   {
		r++;
		strcpy(ret, r);
	}

	return ret;

}
*/

void Worker::SetCurrentFileName(const char *fileName) {

    strcpy(fullFileName, fileName);
}

void Worker::ExportTextures(const char *fileName, int grouping, int mode, bool askConfirm, bool saveSelected) {

    // Read a file
    FILE *f = fopen(fileName, "w");
    if (!f) {
        char tmp[256];
        sprintf(tmp, "Cannot open file for writing %s", fileName);
        throw Error(tmp);
    }

    bool buffer_old = simManager.GetLockedHitBuffer();
#if defined(MOLFLOW)
    geom->ExportTextures(f, grouping, mode, globState, saveSelected, model.wp.sMode);
#endif
#if defined(SYNRAD)
    geom->ExportTextures(f, grouping, mode, no_scans, buffer, saveSelected);
#endif
    simManager.UnlockHitBuffer();
    fclose(f);
}

void Worker::Stop_Public() {
    // Stop
    InnerStop(mApp->m_fTime);
    try {
        Stop();
        Update(mApp->m_fTime);
    }
    catch (Error &e) {
        GLMessageBox::Display(e.what(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
    }
}

void Worker::ReleaseHits() {
    simManager.UnlockHitBuffer();
}

bool Worker::GetHits() {
    try {
        if (needsReload) RealReload();
    }
    catch (Error &e) {
        GLMessageBox::Display(e.what(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
        return false;
    }
    return true;
}

std::tuple<size_t, ParticleLoggerItem *> Worker::GetLogBuff() {
    try {
        if (needsReload) RealReload();
    }
    catch (Error &e) {
        GLMessageBox::Display(e.what(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
    }
    size_t nbRec = 0;
    ParticleLoggerItem *logBuffPtr = NULL;
    size_t *logBuff = (size_t *) simManager.GetLockedLogBuffer();
    if(logBuff) {
        nbRec = *logBuff;
        logBuff++;
        logBuffPtr = (ParticleLoggerItem *) logBuff;
    }
    return std::tie(nbRec, logBuffPtr);
}

void Worker::ReleaseLogBuff() {
    simManager.UnlockLogBuffer();
}

void Worker::ThrowSubProcError(const char *message) {

    char errMsg[1024];
    if (!message)
        sprintf(errMsg, "Bad response from sub process(es):\n%s",GetErrorDetails());
    else
        sprintf(errMsg, "%s\n%s", message, GetErrorDetails());
    throw std::runtime_error(errMsg);

}

void Worker::Reload() {
    //Schedules a reload
    //Actual reloading is done by RealReload() method
    needsReload = true;
}

/*
void Worker::SetMaxDesorption(size_t max) {

	try {
		ResetStatsAndHits(0.0);

		desorptionLimit = max;
		Reload();

	}
	catch (Error &e) {
		GLMessageBox::Display(e.what(), "Error", GLDLG_OK, GLDLG_ICONERROR);
	}

}
*/

const char *Worker::GetErrorDetails() {
    return simManager.GetErrorDetails();
}

void Worker::ResetStatsAndHits(float appTime) {

    if (calcAC) {
        GLMessageBox::Display("Reset not allowed while calculating AC", "Error", GLDLG_OK, GLDLG_ICONERROR);
        return;
    }
    stopTime = 0.0f;
    startTime = 0.0f;
    simuTime = 0.0f;
    if (model.otfParams.nbProcess == 0)
        return;

    try {
        ResetWorkerStats();
        simManager.ResetHits();
        if (needsReload) RealReload();
        Update(appTime);
    }
    catch (std::exception &e) {
        GLMessageBox::Display(e.what(), "Error", GLDLG_OK, GLDLG_ICONERROR);
    }
}

void Worker::Stop() {
    try {
        if (simManager.StopSimulation()) {
            throw std::logic_error("No active simulation to stop!");
        }
    }
    catch (std::exception& e) {
        throw Error(e.what());
    }
}

void Worker::InitSimProc() {

    simManager.useCPU = true;
    simManager.nbCores = 1;
    simManager.nbThreads = 0;

    // Launch n subprocess
    if ((model.otfParams.nbProcess = simManager.InitSimUnits())) {
        throw Error("Starting subprocesses failed!");
    }

    model.otfParams.nbProcess = simManager.nbCores;

    //if (!mApp->loadStatus) mApp->loadStatus = new LoadStatus(this);
}

void Worker::SetProcNumber(size_t n) {

    // Kill all sub process
    try{
        simManager.KillAllSimUnits();
    }
    catch (std::exception& e) {
        throw Error("Killing subprocesses failed!");
    }

    simManager.useCPU = true;
    simManager.nbCores = 1;
    simManager.nbThreads = n;

    // Launch n subprocess
    if ((model.otfParams.nbProcess = simManager.InitSimUnits())) {
        throw Error("Starting subprocesses failed!");
    }

    model.otfParams.nbProcess = simManager.nbCores;

    //if (!mApp->loadStatus) mApp->loadStatus = new LoadStatus(this);
}

size_t Worker::GetPID(size_t prIdx) {
    return 0;
}

void Worker::RebuildTextures() {

    if (mApp->needsTexture || mApp->needsDirection) {

        // Only reload if we are even rebuilding textures
        if (needsReload)
            RealReload();

        bool buffer_old = simManager.GetLockedHitBuffer();
        if (!buffer_old)
            return;


        try {
#if defined(MOLFLOW)
            geom->BuildFacetTextures(globState, mApp->needsTexture, mApp->needsDirection, model.wp.sMode);
#endif
#if defined(SYNRAD)
            geom->BuildFacetTextures(buffer,mApp->needsTexture,mApp->needsDirection);
#endif
        }
        catch (Error &e) {
            simManager.UnlockHitBuffer();
            throw e;
        }
    }
    simManager.UnlockHitBuffer();
}

size_t Worker::GetProcNumber() const {
    return model.otfParams.nbProcess;
}

bool Worker::IsRunning(){
    return simManager.GetRunningStatus();
}

static float lastAccessTime = 0.0f;
void Worker::Update(float appTime) {
    //Refreshes interface cache:
    //Global statistics, leak/hits, global histograms
    //Facet hits, facet histograms, facet angle maps
    //No cache for profiles, textures, directions (plotted directly from shared memory hit buffer)


    if (needsReload) RealReload();

    // End of simulation reached (Stop GUI)
    if ((simManager.allProcsDone || simManager.hasErrorStatus) && IsRunning() && appTime != 0.0f) {
        InnerStop(appTime);
        if (simManager.hasErrorStatus) ThrowSubProcError();
    }

    // Retrieve hit count recording from the shared memory
    // Globals
    if(simManager.simUnits.empty())
        return;
    if(!globState.initialized)
        return;
    mApp->changedSinceSave = true;


#if defined(MOLFLOW)
    bool needsAngleMapStatusRefresh = false;
#endif




        lastAccessTime = appTime;

    //for(auto& simUnit : simManager.simUnits) {
        /*if (!simUnit->tMutex.try_lock_for(std::chrono::milliseconds (100))) {
            continue;
        }*/
        // Global hits and leaks
        if (!globState.tMutex.try_lock_for(std::chrono::milliseconds (100))) {
            return;
        }
        globalHitCache = globState.globalHits;

#if defined(SYNRAD)

        if (globalHitCache.globalHits.hit.nbDesorbed && model.wp.nbTrajPoints) {
            no_scans = (double)globalHitCache.globalHits.hit.nbDesorbed / (double)model.wp.nbTrajPoints;
        }
        else {
            no_scans = 1.0;
        }
#endif


        //Copy global histogram
        //Prepare vectors to receive data
        RetrieveHistogramCache();

    //buffer = bufferStart; //Commented out as not used anymore

        // Refresh local facet hit cache for the displayed moment
        size_t nbFacet = geom->GetNbFacet();
        for (size_t i = 0; i < nbFacet; i++) {
            Facet *f = geom->GetFacet(i);
#if defined(SYNRAD)
            //memcpy(&(f->facetHitCache), buffer + f->sh.hitOffset, sizeof(FacetHitBuffer));
#endif
#if defined(MOLFLOW)
            if (f->sh.anglemapParams.record) { //Recording, so needs to be updated
                if (f->selected && f->sh.anglemapParams.hasRecorded)
                    needsAngleMapStatusRefresh = true; //Will update facetadvparams panel
                //Retrieve angle map from hits dp
                f->angleMapCache = globState.facetStates[i].recordedAngleMapPdf.data(); // TODO: Transform anglemapcache to vector
            }
#endif
        }

        //simUnit->tMutex.unlock();

    try {

        if (mApp->needsTexture || mApp->needsDirection)
            geom->BuildFacetTextures(globState, mApp->needsTexture, mApp->needsDirection
#if defined(MOLFLOW)
                    , model.wp.sMode // not necessary for Synrad
#endif
            );
    }
    catch (Error &e) {
        globState.tMutex.unlock();
        GLMessageBox::Display(e.what(), "Error building texture", GLDLG_OK, GLDLG_ICONERROR);
        return;
    }
    globState.tMutex.unlock();
#if defined(MOLFLOW)
    if (mApp->facetAdvParams && mApp->facetAdvParams->IsVisible() && needsAngleMapStatusRefresh)
        mApp->facetAdvParams->Refresh(geom->GetSelectedFacets());
#endif
}

void Worker::GetProcStatus(size_t *states, std::vector<std::string> &statusStrings) {

    if (model.otfParams.nbProcess == 0) return;
    simManager.GetProcStatus(states, statusStrings);
}

void Worker::GetProcStatus(ProcComm &procInfoList) {
    simManager.GetProcStatus(procInfoList);
}

std::vector<std::vector<double>> Worker::ImportCSV_double(FileReader *file) {
    std::vector<std::vector<double>> table;
    do {
        std::vector<double> currentRow;
        do {
            currentRow.push_back(file->ReadDouble());
            if (!file->IsEol()) file->ReadKeyword(",");
        } while (!file->IsEol());
        table.push_back(currentRow);
    } while (!file->IsEof());
    return table;
}



void Worker::ChangeSimuParams() { //Send simulation mode changes to subprocesses without reloading the whole geometry
    if (model.otfParams.nbProcess == 0 || !geom->IsLoaded()) return;
    if (needsReload) RealReload(); //Sync (number of) regions

    GLProgress *progressDlg = new GLProgress("Creating dataport...", "Passing simulation mode to workers");
    progressDlg->SetVisible(true);
    progressDlg->SetProgress(0.0);

    //To do: only close if parameters changed
    progressDlg->SetMessage("Waiting for subprocesses to release log dataport...");
    try{
        size_t logDpSize = 0;
        if (model.otfParams.enableLogging) {
            logDpSize = sizeof(size_t) + model.otfParams.logLimit * sizeof(ParticleLoggerItem);
        }
        simManager.ReloadLogBuffer(logDpSize, false);
    }
    catch (std::exception& e) {
        GLMessageBox::Display(e.what(), "Warning (ReloadLogBuffer)", GLDLG_OK, GLDLG_ICONWARNING);
        progressDlg->SetVisible(false);
        SAFE_DELETE(progressDlg);
        return;
    }

    progressDlg->SetProgress(0.5);
    progressDlg->SetMessage("Assembling parameters to pass...");

    std::string loaderString = SerializeParamsForLoader().str();
    try {
        for(auto& sim : simManager.simUnits){
            sim->model.otfParams = model.otfParams;
        }
        if(simManager.ShareWithSimUnits((BYTE *) loaderString.c_str(), loaderString.size(),LoadType::LOADPARAM)){
            char errMsg[1024];
            sprintf(errMsg, "Failed to send params to sub process:\n");
            GLMessageBox::Display(errMsg, "Warning (Updateparams)", GLDLG_OK, GLDLG_ICONWARNING);

            progressDlg->SetVisible(false);
            SAFE_DELETE(progressDlg);
            return;
        }
    }
    catch (std::exception& e) {
        GLMessageBox::Display(e.what(), "Error (LoadGeom)", GLDLG_OK, GLDLG_ICONERROR);
    }

    progressDlg->SetVisible(false);
    SAFE_DELETE(progressDlg);

#if defined(SYNRAD)
    //Reset leak and hit cache
    ResetWorkerStats();
#endif
}

/**
* \brief Extract a 7z file and return the file handle
* \param fileName name of the input file
* \param geomName name of the geometry file
* \return handle to opened decompressed file
*/
FileReader *Worker::ExtractFrom7zAndOpen(const std::string &fileName, const std::string &geomName) {
    std::ostringstream cmd;
    std::string sevenZipName;

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    //Necessary push/pop trick to support UNC (network) paths in Windows command-line
    auto CWD = FileUtils::get_working_path();
    cmd << "cmd /C \"pushd \"" << CWD << "\"&&";
    sevenZipName += "7za.exe";
#else //Linux, MacOS
    sevenZipName = "7za"; //so that Exist() check fails and we get an error message on the next command
    std::string possibleLocations[] = {"./7za", //use 7za binary shipped with Molflow
                                       "/usr/bin/7za", //use p7zip installed system-wide
                                       "/usr/local/bin/7za"}; //use p7zip installed for user
    for(auto& path : possibleLocations){
        if (FileUtils::Exist(path)) {
            sevenZipName = path;
        }
    }
#endif

    if (!FileUtils::Exist(sevenZipName)) {
        throw Error("7-zip compressor not found, can't extract file.");
    }
    cmd << sevenZipName << " x -t7z -aoa \"" << fileName << "\" -otmp";

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    cmd << "&&popd\"";
#endif
    system(cmd.str().c_str());

    std::string toOpen, prefix;
    std::string shortFileName = FileUtils::GetFilename(fileName);
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    prefix = CWD + "\\tmp\\";
#else
    prefix = "tmp/";
#endif
    toOpen = prefix + geomName;
    if (!FileUtils::Exist(toOpen))
        //Inside the zip, try original filename with extension changed from geo7z to geo
        toOpen = prefix + (shortFileName).substr(0,shortFileName.length() - 2);


    return new FileReader(toOpen); //decompressed file opened
}


/**
* \brief Function that updates the global hit counter with the cached value + releases the mutex
* Send total hit counts to subprocesses
*/
void Worker::SendToHitBuffer() {
    for(auto& simUnit : simManager.simUnits){
        if(!simUnit->tMutex.try_lock_for(std::chrono::seconds(10)))
            return;
        simUnit->globState = &globState;
        simUnit->tMutex.unlock();
    }
}

/**
* \brief Saves current facet hit counter from cache to results
*/
void Worker::SendFacetHitCounts() {
    size_t nbFacet = geom->GetNbFacet();
    for(auto& simUnit : simManager.simUnits){
        if(!simUnit->tMutex.try_lock_for(std::chrono::seconds(10)))
            return;
        for (size_t i = 0; i < nbFacet; i++) {
            Facet *f = geom->GetFacet(i);
            simUnit->globState->facetStates[i].momentResults[0].hits = f->facetHitCache;
        }
        simUnit->tMutex.unlock();
    }
}

void Worker::RetrieveHistogramCache()
{
	//Copy histograms from hit dataport to local cache
	//The hit dataport contains histograms for all moments, the cache only for the displayed
    //dpHitStartAddress is the beginning of the dpHit buffer

    //GLOBAL HISTOGRAMS
	//Prepare vectors to receive data
    globalHistogramCache = globState.globalHistograms[displayedMoment];

    //FACET HISTOGRAMS
    for (size_t i = 0;i < geom->GetNbFacet();i++) {
        Facet* f = geom->GetFacet(i);
#if defined(MOLFLOW)
        f->facetHitCache = globState.facetStates[i].momentResults[displayedMoment].hits;
        f->facetHistogramCache = globState.facetStates[i].momentResults[displayedMoment].histogram;
#endif
    }
}