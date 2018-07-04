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
#define NOMINMAX
#include <Windows.h>

#include "Worker.h"
#include "Facet_shared.h"
#include "GLApp/GLApp.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp\MathTools.h" //Min max
#include "GLApp\GLList.h"
#include <math.h>
#include <stdlib.h>
#include <Process.h>
#include "GLApp/GLUnitDialog.h"
#include "LoadStatus.h"
#ifdef MOLFLOW
#include "MolFlow.h"
#include "MolflowGeometry.h"
#include "FacetAdvParams.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#include "SynradGeometry.h"
#endif

#include <direct.h>

#include "ZipUtils/zip.h"
#include "ZipUtils/unzip.h"
#include "File.h" //File utils (Get extension, etc)

/*
//Leak detection
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
*/
using namespace pugi;

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

Worker::~Worker() {
	CLOSEDP(dpHit);
	CLOSEDP(dpControl);
	CLOSEDP(dpLog);
	delete geom;
}

Geometry *Worker::GetGeometry() {
	return geom;
}

bool Worker::IsDpInitialized(){

	return (dpHit != NULL);
}

char *Worker::GetCurrentFileName() {
	return fullFileName;
}

char *Worker::GetCurrentShortFileName() {

	static char ret[512];
	char *r = strrchr(fullFileName,'/');
	if(!r) r = strrchr(fullFileName,'\\');
	if(!r) strcpy(ret,fullFileName);
	else   {
		r++;
		strcpy(ret,r);
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

void Worker::SetCurrentFileName(char *fileName) {

	strcpy(fullFileName,fileName);
}

void Worker::ExportTextures(char *fileName, int grouping, int mode, bool askConfirm, bool saveSelected) {

	char tmp[512];

	// Read a file
	FILE *f = NULL;

	bool ok = true;
	if (askConfirm) {
		if (FileUtils::Exist(fileName)) {
			sprintf(tmp, "Overwrite existing file ?\n%s", fileName);
			ok = (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK);
		}
	}
	if (ok) {
		f = fopen(fileName, "w");
		if (!f) {
			char tmp[256];
			sprintf(tmp, "Cannot open file for writing %s", fileName);
			throw Error(tmp);

		}
#ifdef MOLFLOW
		geom->ExportTextures(f, grouping, mode, dpHit, saveSelected,wp.sMode);
#endif
#ifdef SYNRAD
		geom->ExportTextures(f, grouping, mode, no_scans, dpHit, saveSelected);
#endif
		fclose(f);
	}

}

/*
void Worker::SendLeakCache(Dataport* dpHit) { //From worker.globalhitCache to dpHit
	if (dpHit) {
		AccessDataport(dpHit);
		GlobalHitBuffer *gHits = (GlobalHitBuffer *)dpHit->buff;
		size_t nbCopy = Min(LEAKCACHESIZE, globalHitCache.leakCacheSize);
		gHits->leakCache = globalHitCache.leakCache;
		gHits->lastLeakIndex = nbCopy-1;
		gHits->leakCacheSize = globalHitCache.leakCacheSize;
		ReleaseDataport(dpHit);
	}
}

void Worker::SendHitCache(Dataport* dpHit) { //From worker.globalhitCache to dpHit
	if (dpHit) {
		AccessDataport(dpHit);
		GlobalHitBuffer *gHits = (GlobalHitBuffer *)dpHit->buff;
		size_t nbCopy = Min(HITCACHESIZE, globalHitCache.hitCacheSize);
		gHits->hitCache = globalHitCache.hitCache;
		gHits->lastHitIndex = nbCopy - 1;
		gHits->hitCacheSize = globalHitCache.hitCacheSize;
		ReleaseDataport(dpHit);
	}
}
*/

void Worker::Stop_Public() {
	// Stop
	InnerStop(mApp->m_fTime);
	try {
		Stop();
		Update(mApp->m_fTime);
	}
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
	}
}

void  Worker::ReleaseHits() {
	ReleaseDataport(dpHit);
}

BYTE *Worker::GetHits() {
	try {
		if (needsReload) RealReload();
	}
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
	}
	if (dpHit)

		if (AccessDataport(dpHit))
			return (BYTE *)dpHit->buff;

	return NULL;

}

std::tuple<size_t, ParticleLoggerItem*> Worker::GetLogBuff() {
	try {
		if (needsReload) RealReload();
	}
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
	}
	size_t nbRec = 0;
	ParticleLoggerItem* logBuffPtr = NULL;
	if (dpLog)
		if (AccessDataport(dpLog)) {
			size_t* logBuff = (size_t*)dpLog->buff;
			nbRec = *logBuff;
			logBuff++;
			logBuffPtr = (ParticleLoggerItem*)logBuff;
		}
	return std::tie(nbRec,logBuffPtr);
}

void Worker::ReleaseLogBuff() {
	ReleaseDataport(dpLog);
}

void Worker::ThrowSubProcError(std::string message) {
	throw Error(message.c_str());
}

void Worker::ThrowSubProcError(char *message) {

	char errMsg[1024];
	if (!message)
		sprintf(errMsg, "Bad response from sub process(es):\n%s", GetErrorDetails());
	else
		sprintf(errMsg, "%s\n%s", message, GetErrorDetails());
	throw Error(errMsg);

}

void Worker::Reload(){
	needsReload = true;
}

/*
void Worker::SetMaxDesorption(llong max) {

	try {
		ResetStatsAndHits(0.0);

		desorptionLimit = max;
		Reload();

	}
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
	}

}
*/

char *Worker::GetErrorDetails() {

	static char err[1024];
	strcpy(err, "");

	AccessDataport(dpControl);
	SHCONTROL *master = (SHCONTROL *)dpControl->buff;
	for (size_t i = 0; i < ontheflyParams.nbProcess; i++) {
		char tmp[512];
		if (pID[i] != 0) {
			size_t st = master->states[i];
			if (st == PROCESS_ERROR) {
				sprintf(tmp, "[#%zd] Process [PID %d] %s: %s\n", i, pID[i], prStates[st], master->statusStr[i]);
			}
			else {
				sprintf(tmp, "[#%zd] Process [PID %d] %s\n", i, pID[i], prStates[st]);
			}
		}
		else {
			sprintf(tmp, "[#%zd] Process [PID ???] Not started\n", i);
		}
		strcat(err, tmp);
	}
	ReleaseDataport(dpControl);

	return err;
}

bool Worker::Wait(size_t readyState,LoadStatus *statusWindow) {
	
	abortRequested = false;
	bool finished = false;
	bool error = false;

	int waitTime = 0;
	allDone = true;

	// Wait for completion
	while(!finished && !abortRequested) {

		finished = true;
		AccessDataport(dpControl);
		SHCONTROL *shMaster = (SHCONTROL *)dpControl->buff;

		for(size_t i=0;i<ontheflyParams.nbProcess;i++) {

			finished = finished & (shMaster->states[i]==readyState || shMaster->states[i]==PROCESS_ERROR || shMaster->states[i]==PROCESS_DONE);
			if( shMaster->states[i]==PROCESS_ERROR ) {
				error = true;
			}
			allDone = allDone & (shMaster->states[i]==PROCESS_DONE);
		}
		ReleaseDataport(dpControl);

		if (!finished) {

			if (statusWindow) {
				if (waitTime >= 500) {
					statusWindow->SetVisible(true);
				}
				statusWindow->SMPUpdate();
				mApp->DoEvents(); //Do a few refreshes during waiting for subprocesses
			}
			Sleep(250);
			waitTime+=250;
		}
	}

	if (statusWindow) {
		statusWindow->SetVisible(false);
		statusWindow->EnableStopButton();
	}
	return finished && !error;

}

bool Worker::ExecuteAndWait(int command, size_t readyState,size_t param) {

	if(!dpControl) return false;

	// Send command
	AccessDataport(dpControl);
	SHCONTROL *shMaster = (SHCONTROL *)dpControl->buff;
	for(size_t i=0;i<ontheflyParams.nbProcess;i++) {
		shMaster->states[i]=command;
		shMaster->cmdParam[i]=param;
	}
	ReleaseDataport(dpControl);

	Sleep(100);

	if (!mApp->loadStatus) mApp->loadStatus = new LoadStatus(this);
	bool result= Wait(readyState, mApp->loadStatus);
	//SAFE_DELETE(statusWindow);
	return result;
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
		if (!ExecuteAndWait(COMMAND_RESET, PROCESS_READY))
			ThrowSubProcError();
		ClearHits(false);
		Update(appTime);
	}
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
	}
}

void Worker::Stop() {

	if(ontheflyParams.nbProcess==0 )
		throw Error("No sub process found. (Simulation not available)");

	if( !ExecuteAndWait(COMMAND_PAUSE,PROCESS_READY) )
		ThrowSubProcError();
}

void Worker::KillAll() {

	if( dpControl && ontheflyParams.nbProcess>0 ) {
		if( !ExecuteAndWait(COMMAND_EXIT,PROCESS_KILLED) ) {
			AccessDataport(dpControl);
			SHCONTROL *shMaster = (SHCONTROL *)dpControl->buff;
			for(size_t i=0;i<ontheflyParams.nbProcess;i++)
				if(shMaster->states[i]==PROCESS_KILLED) pID[i]=0;
			ReleaseDataport(dpControl);
			// Force kill
			for(size_t i=0;i<ontheflyParams.nbProcess;i++)
				if(pID[i]) KillProc(pID[i]);
		}
		CLOSEDP(dpHit);
	}
	ontheflyParams.nbProcess = 0;

}

void Worker::SetProcNumber(size_t n) {

	char cmdLine[512];

	// Kill all sub process
	KillAll();

	// Create new control dataport
	if( !dpControl ) 
		dpControl = CreateDataport(ctrlDpName,sizeof(SHCONTROL));
	if( !dpControl )
		throw Error("Failed to create 'control' dataport");
	AccessDataport(dpControl);
	memset(dpControl->buff,0,sizeof(SHCONTROL));
	ReleaseDataport(dpControl);

	// Launch n subprocess
	for(size_t i=0;i<n;i++) {
		#ifdef MOLFLOW
		sprintf(cmdLine,"molflowSub.exe %d %zd",pid,i);
		#endif
		#ifdef SYNRAD
		sprintf(cmdLine,"synradSub.exe %d %zd",pid,i);
		#endif
		pID[i] = StartProc(cmdLine,STARTPROC_NORMAL);
		Sleep(25); // Wait a bit
		if( pID[i]==0 ) {
			ontheflyParams.nbProcess = 0;
			throw Error(cmdLine);
		}
	}

	ontheflyParams.nbProcess = n;

	if (!mApp->loadStatus) mApp->loadStatus = new LoadStatus(this);
	bool result = Wait(PROCESS_READY, mApp->loadStatus);
	//IVALIDATE_DLG(statusWindow);
	//SAFE_DELETE(statusWindow);
	if( !result )
		ThrowSubProcError("Sub process(es) starting failure");
}

DWORD Worker::GetPID(size_t prIdx) {
	return pID[prIdx];
}

void Worker::RebuildTextures() {
	if (!dpHit) return;
	if (needsReload) RealReload();
	if (AccessDataport(dpHit)) {
		BYTE *buffer = (BYTE *)dpHit->buff;
		if (mApp->needsTexture || mApp->needsDirection) try{ geom->BuildFacetTextures(buffer,mApp->needsTexture,mApp->needsDirection,wp.sMode); }
		catch (Error &e) {
			ReleaseDataport(dpHit);
			throw e;
		}
		ReleaseDataport(dpHit);
	}
}

size_t Worker::GetProcNumber() {
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
	if (dpControl) {
		if (AccessDataport(dpControl)) {
			int i = 0;
			SHCONTROL *master = (SHCONTROL *)dpControl->buff;
			for (int i = 0; i<ontheflyParams.nbProcess && done; i++) {
				done = done && (master->states[i] == PROCESS_DONE);
				error = error && (master->states[i] == PROCESS_ERROR);
#ifdef MOLFLOW
				if (master->states[i] == PROCESS_RUNAC) calcACprg = master->cmdParam[i];
#endif
			}
			ReleaseDataport(dpControl);
		}
	}

	// End of simulation reached (Stop GUI)
	if ((error || done) && isRunning && appTime != 0.0f) {
		InnerStop(appTime);
		if (error) ThrowSubProcError();
	}

	// Retrieve hit count recording from the shared memory
	if (dpHit) {

		if (AccessDataport(dpHit)) {
			BYTE *bufferStart = (BYTE *)dpHit->buff;
			BYTE *buffer = bufferStart;


			mApp->changedSinceSave = true;
			// Globals
			globalHitCache = READBUFFER(GlobalHitBuffer);

// Global hits and leaks
#ifdef MOLFLOW
			bool needsAngleMapStatusRefresh = false;
#endif

#ifdef SYNRAD

			if (nbDesorption && nbTrajPoints) {
				no_scans = (double)nbDesorption / (double)nbTrajPoints;
			}
			else {
				no_scans = 1.0;
			}
#endif


			//Copy global histogram
			//Prepare vectors to receive data
			if (wp.globalHistogramParams.recordBounce) globalHistogramCache.nbHitsHistogram.resize(wp.globalHistogramParams.GetBounceHistogramSize());
			if (wp.globalHistogramParams.recordDistance) globalHistogramCache.distanceHistogram.resize(wp.globalHistogramParams.GetDistanceHistogramSize());
			if (wp.globalHistogramParams.recordTime) globalHistogramCache.timeHistogram.resize(wp.globalHistogramParams.GetTimeHistogramSize());
				

				BYTE* globalHistogramAddress = buffer; //Already increased by READBUFFER(GlobalHitBuffer) above
#ifdef MOLFLOW
				globalHistogramAddress += displayedMoment * wp.globalHistogramParams.GetDataSize();
#endif

			memcpy(globalHistogramCache.nbHitsHistogram.data(), globalHistogramAddress, wp.globalHistogramParams.GetBouncesDataSize());
			memcpy(globalHistogramCache.distanceHistogram.data(), globalHistogramAddress + wp.globalHistogramParams.GetBouncesDataSize(), wp.globalHistogramParams.GetDistanceDataSize());
			memcpy(globalHistogramCache.timeHistogram.data(), globalHistogramAddress + wp.globalHistogramParams.GetBouncesDataSize() + wp.globalHistogramParams.GetDistanceDataSize(), wp.globalHistogramParams.GetTimeDataSize());
			
			buffer = bufferStart;

			// Refresh local facet hit cache for the displayed moment
			size_t nbFacet = geom->GetNbFacet();
			for (size_t i = 0; i<nbFacet; i++) {
				Facet *f = geom->GetFacet(i);
#ifdef SYNRAD
				memcpy(&(f->facetHitCache), buffer + f->sh.hitOffset, sizeof(FacetHitBuffer));
#endif
#ifdef MOLFLOW
				memcpy(&(f->facetHitCache), buffer + f->sh.hitOffset + displayedMoment * sizeof(FacetHitBuffer), sizeof(FacetHitBuffer));
				
				if (f->sh.anglemapParams.record) {
					if (!f->sh.anglemapParams.hasRecorded) { //It was released by the user maybe
						//Initialize angle map
						f->angleMapCache = (size_t*)malloc(f->sh.anglemapParams.GetDataSize());
						if (!f->angleMapCache) {
							std::stringstream tmp;
							tmp << "Not enough memory for incident angle map on facet " << i + 1;
							throw Error(tmp.str().c_str());
						}
						f->sh.anglemapParams.hasRecorded = true;
						if (f->selected) needsAngleMapStatusRefresh = true;
					}
					//Retrieve angle map from hits dp
					BYTE* angleMapAddress = buffer
					+ f->sh.hitOffset
					+ (1 + moments.size()) * sizeof(FacetHitBuffer)
					+ (f->sh.isProfile ? PROFILE_SIZE * sizeof(ProfileSlice) *(1 + moments.size()) : 0)
					+ (f->sh.isTextured ? f->sh.texWidth*f->sh.texHeight * sizeof(TextureCell) *(1 + moments.size()) : 0)
					+ (f->sh.countDirection ? f->sh.texWidth*f->sh.texHeight * sizeof(DirectionCell)*(1 + moments.size()) : 0);
					memcpy(f->angleMapCache, angleMapAddress, f->sh.anglemapParams.GetRecordedDataSize());
				}
				
#endif
				
					//Prepare vectors for receiving data
					if (f->sh.facetHistogramParams.recordBounce) f->facetHistogramCache.nbHitsHistogram.resize(f->sh.facetHistogramParams.GetBounceHistogramSize());
					if (f->sh.facetHistogramParams.recordDistance) f->facetHistogramCache.distanceHistogram.resize(f->sh.facetHistogramParams.GetDistanceHistogramSize());
					if (f->sh.facetHistogramParams.recordTime) f->facetHistogramCache.timeHistogram.resize(f->sh.facetHistogramParams.GetTimeHistogramSize());
										
					//Retrieve histogram map from hits dp
					BYTE* histogramAddress = buffer
						+ f->sh.hitOffset
						+ (1 + moments.size()) * sizeof(FacetHitBuffer)
						+ (f->sh.isProfile ? PROFILE_SIZE * sizeof(ProfileSlice) *(1 + moments.size()) : 0)
						+ (f->sh.isTextured ? f->sh.texWidth*f->sh.texHeight * sizeof(TextureCell) *(1 + moments.size()) : 0)
						+ (f->sh.countDirection ? f->sh.texWidth*f->sh.texHeight * sizeof(DirectionCell)*(1 + moments.size()) : 0)
						+ f->sh.anglemapParams.GetRecordedDataSize();
#ifdef MOLFLOW
					histogramAddress += displayedMoment * f->sh.facetHistogramParams.GetDataSize();
#endif
					
					memcpy(f->facetHistogramCache.nbHitsHistogram.data(), histogramAddress, f->sh.facetHistogramParams.GetBouncesDataSize());
					memcpy(f->facetHistogramCache.distanceHistogram.data(), histogramAddress+ f->sh.facetHistogramParams.GetBouncesDataSize(), f->sh.facetHistogramParams.GetDistanceDataSize());
					memcpy(f->facetHistogramCache.timeHistogram.data(), histogramAddress + f->sh.facetHistogramParams.GetBouncesDataSize() + f->sh.facetHistogramParams.GetDistanceDataSize(), f->sh.facetHistogramParams.GetTimeDataSize());
				
			}
			try {
				if (mApp->needsTexture || mApp->needsDirection) geom->BuildFacetTextures(buffer, mApp->needsTexture, mApp->needsDirection, wp.sMode);
			}
			catch (Error &e) {
				GLMessageBox::Display((char *)e.GetMsg(), "Error building texture", GLDLG_OK, GLDLG_ICONERROR);
				ReleaseDataport(dpHit);
				return;
			}
#ifdef MOLFLOW
			if (mApp->facetAdvParams && mApp->facetAdvParams->IsVisible() && needsAngleMapStatusRefresh)
				mApp->facetAdvParams->Refresh(geom->GetSelectedFacets());
#endif
			ReleaseDataport(dpHit);
		}
	}
}

void Worker::GetProcStatus(size_t *states, std::vector<std::string>& statusStrings) {

	if (ontheflyParams.nbProcess == 0) return;

	AccessDataport(dpControl);
	SHCONTROL *shMaster = (SHCONTROL *)dpControl->buff;
	memcpy(states, shMaster->states, MAX_PROCESS * sizeof(size_t));
	for (size_t i = 0; i < MAX_PROCESS; i++) {
		char tmp[128];
		strncpy(tmp, shMaster->statusStr[i], 127);
		tmp[127] = 0;
		statusStrings[i] = tmp;
	}
	ReleaseDataport(dpControl);

}

std::vector<std::vector<std::string>> Worker::ImportCSV_string(FileReader *file) {
	std::vector<std::vector<string>> table; //reset table
	do {
		std::vector<std::string> row;
		std::string line = file->ReadLine();
		std::stringstream token;
		size_t cursor = 0;
		size_t length = line.length();
		while (cursor<length) {
			char c = line[cursor];
			if (c == ',') {
				row.push_back(token.str());
				token.str(""); token.clear();
			}
			else {
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
#ifdef SYNRAD
	loadSize += regions.size() * sizeof(bool); //Show photons or not
#endif

	//To do: only close if parameters changed
	CLOSEDP(dpLog);
	progressDlg->SetMessage("Waiting for subprocesses to release log dataport...");
	if (!ExecuteAndWait(COMMAND_RELEASEDPLOG, isRunning ? PROCESS_RUN : PROCESS_READY, isRunning ? PROCESS_RUN : PROCESS_READY)) {
		char errMsg[1024];
		sprintf(errMsg, "Subprocesses didn't release dpLog handle:\n%s", GetErrorDetails());
		GLMessageBox::Display(errMsg, "Warning (Updateparams)", GLDLG_OK, GLDLG_ICONWARNING);

		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		return;
	}
	if (ontheflyParams.enableLogging) {
		size_t logDpSize = sizeof(size_t) + ontheflyParams.logLimit * sizeof(ParticleLoggerItem);
		dpLog = CreateDataport(logDpName, logDpSize);
		if (!dpLog) {
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw Error("Failed to create 'dpLog' dataport.\nMost probably out of memory.\nReduce number of logged particles in Particle Logger.");
		}
		//Fills values with 0
	}

	Dataport *loader = CreateDataport(loadDpName, loadSize);
	if (!loader) {
		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		throw Error("Failed to create 'loader' dataport.\nMost probably out of memory.\nReduce number of subprocesses or texture size.");
	}
	progressDlg->SetMessage("Accessing dataport...");
	AccessDataportTimed(loader, 1000);
	progressDlg->SetMessage("Assembling parameters to pass...");

	BYTE* buffer = (BYTE*)loader->buff;
	WRITEBUFFER(ontheflyParams, OntheflySimulationParams);


#ifdef SYNRAD
	for (size_t i = 0; i < regions.size(); i++) {
		WRITEBUFFER(regions[i].params.showPhotons, bool);
	}
#endif

	progressDlg->SetMessage("Releasing dataport...");
	ReleaseDataport(loader);

	// Pass to workers
	progressDlg->SetMessage("Waiting for subprocesses to read mode...");
	if (!ExecuteAndWait(COMMAND_UPDATEPARAMS, isRunning ? PROCESS_RUN : PROCESS_READY, isRunning ? PROCESS_RUN : PROCESS_READY)) {
		CLOSEDP(loader);
		char errMsg[1024];
		sprintf(errMsg, "Failed to send params to sub process:\n%s", GetErrorDetails());
		GLMessageBox::Display(errMsg, "Warning (Updateparams)", GLDLG_OK, GLDLG_ICONWARNING);

		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		return;
	}

	progressDlg->SetMessage("Closing dataport...");
	CLOSEDP(loader);
	progressDlg->SetVisible(false);
	SAFE_DELETE(progressDlg);

#ifdef SYNRAD
	//Reset leak and hit cache
	leakCacheSize = 0;
	SetLeakCache(leakCache, &leakCacheSize, dpHit); //will only write leakCacheSize
	hitCacheSize = 0;
	SetHitCache(hitCache, &hitCacheSize, dpHit); //will only write hitCacheSize
#endif
}