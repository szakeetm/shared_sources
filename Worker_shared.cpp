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
#include "GLApp/MathTools.h" //Min max
#include "GLApp/GLList.h"
#include <math.h>
#include <stdlib.h>
#include "GLApp/GLUnitDialog.h"
#include "Interface/LoadStatus.h"
#include "ProcessControl.h" // defines for process commands

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
    BYTE *buffer = simManager.GetLockedHitBuffer();
#if defined(MOLFLOW)
    geom->ExportTextures(f, grouping, mode, buffer, saveSelected, wp.sMode);
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

BYTE *Worker::GetHits() {
    try {
        if (needsReload) RealReload();
    }
    catch (Error &e) {
        GLMessageBox::Display(e.what(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
    }
    return simManager.GetLockedHitBuffer();
}

std::tuple<size_t, ParticleLoggerItem *> Worker::GetLogBuff() {
    try {
        if (needsReload) RealReload();
    }
    catch (Error &e) {
        GLMessageBox::Display((char *) e.what(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
    }
    size_t nbRec = 0;
    ParticleLoggerItem *logBuffPtr = NULL;
    size_t *logBuff = (size_t *) simManager.UnlockLogBuffer();
    nbRec = *logBuff;
    logBuff++;
    logBuffPtr = (ParticleLoggerItem *) logBuff;
    return std::tie(nbRec, logBuffPtr);
}

void Worker::ReleaseLogBuff() {
    simManager.UnlockLogBuffer();
}

void Worker::ThrowSubProcError(std::string message) {
    throw Error(message.c_str());
}

void Worker::ThrowSubProcError(const char *message) {

    char errMsg[1024];
    if (!message)
        sprintf(errMsg, "Bad response from sub process(es):\n%s", GetErrorDetails());
    else
        sprintf(errMsg, "%s\n%s", message, GetErrorDetails());
    throw Error(errMsg);

}

void Worker::Reload() {
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

    std::vector<ProcInfo> procInfo;
    simManager.GetProcStatus(procInfo);

    static char err[1024];
    strcpy(err, "");

    for (size_t i = 0; i < procInfo.size(); i++) {
        char tmp[512];
        size_t state = procInfo[i].statusId;
        if (state == PROCESS_ERROR) {
            sprintf(tmp, "[#%zd] Process [PID %lu] %s: %s\n", i, procInfo[i].procId, prStates[state],
                    procInfo[i].statusString.c_str());
            strcat(err, tmp);
        } else {
            sprintf(tmp, "[#%zd] Process [PID %lu] %s\n", i, procInfo[i].procId, prStates[state]);
        }
    }

    return err;
}

void Worker::ResetStatsAndHits(float appTime) {

    if (calcAC) {
        GLMessageBox::Display("Reset not allowed while calculating AC", "Error", GLDLG_OK, GLDLG_ICONERROR);
        return;
    }
    stopTime = 0.0f;
    startTime = 0.0f;
    simuTime = 0.0f;
    isRunning = false;
    if (ontheflyParams.nbProcess == 0)
        return;

    try {
        ResetWorkerStats();
        if (simManager.ExecuteAndWait(COMMAND_RESET, PROCESS_READY))
            ThrowSubProcError();
        ClearHits(false);
        Update(appTime);
    }
    catch (Error &e) {
        GLMessageBox::Display(e.what(), "Error", GLDLG_OK, GLDLG_ICONERROR);
    }
}

void Worker::Stop() {

    if (ontheflyParams.nbProcess == 0)
        throw Error("No sub process found. (Simulation not available)");

    simManager.ForwardCommand(COMMAND_PAUSE);
    if (simManager.WaitForProcStatus(PROCESS_READY))
        ThrowSubProcError();
}

void Worker::SetProcNumber(size_t n, bool keepDpHit) {

    // Kill all sub process
    simManager.KillAllSimUnits();

    // Restart Control Dataport if necessary
    if (!keepDpHit) simManager.CloseHitsDP();
    // Create new control dataport
    simManager.CreateControlDP();

    simManager.useCPU = true;
    simManager.nbCores = n;
    // Launch n subprocess

    if (simManager.InitSimUnits()) {
        ontheflyParams.nbProcess = 0;
        throw Error("Starting subprocesses failed!");
    }

    ontheflyParams.nbProcess = n;

    //if (!mApp->loadStatus) mApp->loadStatus = new LoadStatus(this);
}

size_t Worker::GetPID(size_t prIdx) {
    return simManager.simHandles.at(prIdx).first;
}

void Worker::RebuildTextures() {
    BYTE *buffer = simManager.GetLockedHitBuffer();
    if (!buffer)
        return;
    if (needsReload)
        RealReload();

    if (mApp->needsTexture || mApp->needsDirection) {
        try {
#if defined(MOLFLOW)
            geom->BuildFacetTextures(buffer, mApp->needsTexture, mApp->needsDirection, wp.sMode);
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
    return ontheflyParams.nbProcess;
}

void Worker::Update(float appTime) {
    //Refreshes interface cache:
    //Global statistics, leak/hits, global histograms
    //Facet hits, facet histograms, facet angle maps
    //No cache for profiles, textures, directions (plotted directly from shared memory hit buffer)


    if (needsReload) RealReload();

    // Check calculation ending
    bool done = true;
    bool error = true;

    std::vector<ProcInfo> procInfo;
    simManager.GetProcStatus(procInfo);

    for (size_t i = 0; i < procInfo.size() && done; i++) {
        const size_t procState = procInfo[i].statusId;
        done = done && (procState == PROCESS_DONE);
        error = error && (procState == PROCESS_ERROR);

#if defined(MOLFLOW)
        if (procState == PROCESS_RUNAC) calcACprg = procInfo[i].cmdParam;
#endif
    }

    // End of simulation reached (Stop GUI)
    if ((error || done) && isRunning && appTime != 0.0f) {
        InnerStop(appTime);
        if (error) ThrowSubProcError();
    }

    // Retrieve hit count recording from the shared memory
    BYTE *bufferStart = simManager.GetLockedHitBuffer();
    if (!bufferStart)
        return;

    BYTE *buffer = bufferStart;


    mApp->changedSinceSave = true;
    // Globals
    globalHitCache = READBUFFER(GlobalHitBuffer);

    // Global hits and leaks
#if defined(MOLFLOW)
    bool needsAngleMapStatusRefresh = false;
#endif

#if defined(SYNRAD)

    if (globalHitCache.globalHits.hit.nbDesorbed && wp.nbTrajPoints) {
        no_scans = (double)globalHitCache.globalHits.hit.nbDesorbed / (double)wp.nbTrajPoints;
    }
    else {
        no_scans = 1.0;
    }
#endif


    //Copy global histogram
    //Prepare vectors to receive data
    globalHistogramCache.Resize(wp.globalHistogramParams);

    BYTE *globalHistogramAddress = buffer; //Already increased by READBUFFER(GlobalHitBuffer) above
#if defined(MOLFLOW)
    globalHistogramAddress += displayedMoment * wp.globalHistogramParams.GetDataSize();
#endif

    memcpy(globalHistogramCache.nbHitsHistogram.data(), globalHistogramAddress,
           wp.globalHistogramParams.GetBouncesDataSize());
    memcpy(globalHistogramCache.distanceHistogram.data(),
           globalHistogramAddress + wp.globalHistogramParams.GetBouncesDataSize(),
           wp.globalHistogramParams.GetDistanceDataSize());
#if defined(MOLFLOW)
    memcpy(globalHistogramCache.timeHistogram.data(),
           globalHistogramAddress + wp.globalHistogramParams.GetBouncesDataSize() +
           wp.globalHistogramParams.GetDistanceDataSize(), wp.globalHistogramParams.GetTimeDataSize());
#endif
    buffer = bufferStart;

    // Refresh local facet hit cache for the displayed moment
    size_t nbFacet = geom->GetNbFacet();
    for (size_t i = 0; i < nbFacet; i++) {
        Facet *f = geom->GetFacet(i);
#if defined(SYNRAD)
        memcpy(&(f->facetHitCache), buffer + f->sh.hitOffset, sizeof(FacetHitBuffer));
#endif
#if defined(MOLFLOW)
        memcpy(&(f->facetHitCache), buffer + f->sh.hitOffset + displayedMoment * sizeof(FacetHitBuffer),
               sizeof(FacetHitBuffer));

        if (f->sh.anglemapParams.record) {
            if (f->selected && f->angleMapCache.empty())
                needsAngleMapStatusRefresh = true; //Will update facetadvparams panel

            //Retrieve angle map from hits dp
            BYTE *angleMapAddress = buffer
                                    + f->sh.hitOffset
                                    + (1 + moments.size()) * sizeof(FacetHitBuffer)
                                    + (f->sh.isProfile ? PROFILE_SIZE * sizeof(ProfileSlice) * (1 + moments.size()) : 0)
                                    + (f->sh.isTextured ? f->sh.texWidth * f->sh.texHeight * sizeof(TextureCell) *
                                                          (1 + moments.size()) : 0)
                                    + (f->sh.countDirection ? f->sh.texWidth * f->sh.texHeight * sizeof(DirectionCell) *
                                                              (1 + moments.size()) : 0);
            f->angleMapCache.resize(f->sh.anglemapParams.phiWidth * (f->sh.anglemapParams.thetaLowerRes +
                                                                     f->sh.anglemapParams.thetaHigherRes)); //Will be filled with values
            memcpy(f->angleMapCache.data(), angleMapAddress, sizeof(size_t) * (f->sh.anglemapParams.phiWidth *
                                                                               (f->sh.anglemapParams.thetaLowerRes +
                                                                                f->sh.anglemapParams.thetaHigherRes)));
        }

#endif
#if defined(MOLFLOW)

        //Prepare vectors for receiving data
        f->facetHistogramCache.Resize(f->sh.facetHistogramParams);

        //Retrieve histogram map from hits dp
        BYTE *histogramAddress = buffer
                                 + f->sh.hitOffset
                                 + (1 + moments.size()) * sizeof(FacetHitBuffer)
                                 + (f->sh.isProfile ? PROFILE_SIZE * sizeof(ProfileSlice) * (1 + moments.size()) : 0)
                                 + (f->sh.isTextured ? f->sh.texWidth * f->sh.texHeight * sizeof(TextureCell) *
                                                       (1 + moments.size()) : 0)
                                 + (f->sh.countDirection ? f->sh.texWidth * f->sh.texHeight * sizeof(DirectionCell) *
                                                           (1 + moments.size()) : 0)
                                 //+ f->sh.anglemapParams.GetRecordedDataSize();
                                 + sizeof(size_t) * (f->sh.anglemapParams.phiWidth *
                                                     (f->sh.anglemapParams.thetaLowerRes +
                                                      f->sh.anglemapParams.thetaHigherRes));
        histogramAddress += displayedMoment * f->sh.facetHistogramParams.GetDataSize();

        memcpy(f->facetHistogramCache.nbHitsHistogram.data(), histogramAddress,
               f->sh.facetHistogramParams.GetBouncesDataSize());
        memcpy(f->facetHistogramCache.distanceHistogram.data(),
               histogramAddress + f->sh.facetHistogramParams.GetBouncesDataSize(),
               f->sh.facetHistogramParams.GetDistanceDataSize());
        memcpy(f->facetHistogramCache.timeHistogram.data(),
               histogramAddress + f->sh.facetHistogramParams.GetBouncesDataSize() +
               f->sh.facetHistogramParams.GetDistanceDataSize(), f->sh.facetHistogramParams.GetTimeDataSize());
#endif

    }
    try {

        if (mApp->needsTexture || mApp->needsDirection)
            geom->BuildFacetTextures(buffer, mApp->needsTexture, mApp->needsDirection
#if defined(MOLFLOW)
                    , wp.sMode // not necessary for Synrad
#endif
            );
    }
    catch (Error &e) {
        GLMessageBox::Display(e.what(), "Error building texture", GLDLG_OK, GLDLG_ICONERROR);
        simManager.UnlockHitBuffer();
        return;
    }
#if defined(MOLFLOW)
    if (mApp->facetAdvParams && mApp->facetAdvParams->IsVisible() && needsAngleMapStatusRefresh)
        mApp->facetAdvParams->Refresh(geom->GetSelectedFacets());
#endif
    simManager.UnlockHitBuffer();


}

void Worker::GetProcStatus(size_t *states, std::vector<std::string> &statusStrings) {

    if (ontheflyParams.nbProcess == 0) return;

    simManager.GetProcStatus(states, statusStrings);
}

std::vector<std::vector<std::string>> Worker::ImportCSV_string(FileReader *file) {
    std::vector<std::vector<string>> table; //reset table
    do {
        std::vector<std::string> row;
        std::string line = file->ReadLine();
        std::stringstream token;
        size_t cursor = 0;
        size_t length = line.length();
        while (cursor < length) {
            char c = line[cursor];
            if (c == ',') {
                row.push_back(token.str());
                token.str("");
                token.clear();
            } else {
                token << c;
            }
            cursor++;
        }
        if (token.str().length() > 0) row.push_back(token.str());

        table.push_back(row);
    } while (!file->IsEof());
    return table;
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
    if (ontheflyParams.nbProcess == 0 || !geom->IsLoaded()) return;
    if (needsReload) RealReload(); //Sync (number of) regions

    GLProgress *progressDlg = new GLProgress("Creating dataport...", "Passing simulation mode to workers");
    progressDlg->SetVisible(true);
    progressDlg->SetProgress(0.0);

    // Create the temporary geometry shared structure
    size_t loadSize = sizeof(OntheflySimulationParams);
#if defined(SYNRAD)
    loadSize += regions.size() * sizeof(bool); //Show photons or not
#endif

    //To do: only close if parameters changed
    simManager.CloseLogDP();
    progressDlg->SetMessage("Waiting for subprocesses to release log dataport...");
    if (simManager.ExecuteAndWait(COMMAND_RELEASEDPLOG, isRunning ? PROCESS_RUN : PROCESS_READY,
                                  isRunning ? PROCESS_RUN : PROCESS_READY)) {
        char errMsg[1024];
        sprintf(errMsg, "Subprocesses didn't release dpLog handle:\n%s", GetErrorDetails());
        GLMessageBox::Display(errMsg, "Warning (Updateparams)", GLDLG_OK, GLDLG_ICONWARNING);

        progressDlg->SetVisible(false);
        SAFE_DELETE(progressDlg);
        return;
    }
    if (ontheflyParams.enableLogging) {
        size_t logDpSize = sizeof(size_t) + ontheflyParams.logLimit * sizeof(ParticleLoggerItem);
        if (simManager.CreateLogDP(logDpSize)) {
            progressDlg->SetVisible(false);
            SAFE_DELETE(progressDlg);
            throw Error(
                    "Failed to create 'dpLog' dataport.\nMost probably out of memory.\nReduce number of logged particles in Particle Logger.");
        }
        //Fills values with 0
    }


    //Dataport *loader = CreateDataport(loadDpName, loadSize);
    if (simManager.CreateLoaderDP(loadSize)) {
        progressDlg->SetVisible(false);
        SAFE_DELETE(progressDlg);
        throw Error(
                "Failed to create 'loader' dataport.\nMost probably out of memory.\nReduce number of subprocesses or texture size.");
    }
    progressDlg->SetMessage("Assembling parameters to pass...");

    BYTE *buffer = new BYTE[loadSize]();
    BYTE * bufferStart = buffer;
    WRITEBUFFER(ontheflyParams, OntheflySimulationParams);


#if defined(SYNRAD)
    for (size_t i = 0; i < regions.size(); i++) {
        WRITEBUFFER(regions[i].params.showPhotons, bool);
    }
#endif

    progressDlg->SetMessage("Uploading new parameters...");
    simManager.UploadToLoader(bufferStart, loadSize);
    delete[] bufferStart;

    // Pass to workers
    progressDlg->SetMessage("Waiting for subprocesses to read mode...");
    if (simManager.ExecuteAndWait(COMMAND_UPDATEPARAMS, isRunning ? PROCESS_RUN : PROCESS_READY, isRunning ? PROCESS_RUN : PROCESS_READY)) {
        simManager.CloseLoaderDP();
        char errMsg[1024];
        sprintf(errMsg, "Failed to send params to sub process:\n%s", GetErrorDetails());
        GLMessageBox::Display(errMsg, "Warning (Updateparams)", GLDLG_OK, GLDLG_ICONWARNING);

        progressDlg->SetVisible(false);
        SAFE_DELETE(progressDlg);
        return;
    }

    progressDlg->SetMessage("Closing dataport...");
    simManager.CloseLoaderDP();
    progressDlg->SetVisible(false);
    SAFE_DELETE(progressDlg);

#if defined(SYNRAD)
    //Reset leak and hit cache
    /*
    leakCacheSize = 0;
    SetLeakCache(leakCache, &leakCacheSize, dpHit); //will only write leakCacheSize
    hitCacheSize = 0;
    SetHitCache(hitCache, &hitCacheSize, dpHit); //will only write hitCacheSize
    */
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
#endif

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    sevenZipName += "7za.exe";
#else //Linux, MacOS
    if (FileUtils::Exist("./7za")) {
        sevenZipName = "./7za"; //use 7za binary shipped with Molflow
    } else if (FileUtils::Exist("/usr/bin/7za")) {
        sevenZipName = "/usr/bin/7za"; //use p7zip installed system-wide
    } else {
        sevenZipName = "7za"; //so that Exist() check fails and we get an error message on the next command
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
        toOpen = prefix + (shortFileName).substr(0, shortFileName.length() -
                                                    2); //Inside the zip, try original filename with extension changed from geo7z to geo

    return new FileReader(toOpen); //decompressed file opened
}
