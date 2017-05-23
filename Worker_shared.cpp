#include <Windows.h>

#include "Worker.h"
#include "Facet.h"
#include "GLApp/GLApp.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp\MathTools.h" //Min max
#include <math.h>
#include <stdlib.h>
#include <Process.h>
#include "GLApp/GLUnitDialog.h"
#ifdef MOLFLOW
#include "MolFlow.h"
#include "MolflowGeometry.h"
#include "LoadStatus.h"
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
	delete geom;
}

// -------------------------------------------------------------

Geometry *Worker::GetGeometry() {
	return geom;
}

bool Worker::IsDpInitialized(){

	return (dpHit != NULL);
}
// -------------------------------------------------------------

char *Worker::GetFileName() {
	return fullFileName;
}

char *Worker::GetShortFileName() {

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

void Worker::SetFileName(char *fileName) {

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
		geom->ExportTextures(f, grouping, mode, dpHit, saveSelected);
#endif
#ifdef SYNRAD
		geom->ExportTextures(f, grouping, mode, no_scans, dpHit, saveSelected);
#endif
		fclose(f);
	}

}

void Worker::SetLeakCache(LEAK *buffer,size_t *nb,Dataport* dpHit) { //When loading from file
	if (dpHit) {
		AccessDataport(dpHit);
		SHGHITS *gHits = (SHGHITS *)dpHit->buff;
		size_t nbCopy = MIN(LEAKCACHESIZE, *nb);
		memcpy(leakCache, buffer, sizeof(LEAK)*nbCopy);
		memcpy(gHits->leakCache, buffer, sizeof(LEAK)*nbCopy);
		gHits->lastLeakIndex = nbCopy % LEAKCACHESIZE;
		gHits->leakCacheSize = nbCopy;
		ReleaseDataport(dpHit);
	}
}

void Worker::SetHitCache(HIT *buffer, size_t *nb, Dataport *dpHit) {
	if (dpHit) {
		AccessDataport(dpHit);
		SHGHITS *gHits = (SHGHITS *)dpHit->buff;
		size_t nbCopy = MIN(HITCACHESIZE, *nb);
		memcpy(hitCache, buffer, sizeof(HIT)*nbCopy);
		memcpy(gHits->hitCache, buffer, sizeof(HIT)*nbCopy);
		gHits->lastHitIndex = nbCopy % HITCACHESIZE;
		gHits->hitCacheSize = nbCopy;
		ReleaseDataport(dpHit);
	}
}

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

// -------------------------------------------------------------

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

char *Worker::GetErrorDetails() {

	static char err[1024];
	strcpy(err, "");

	AccessDataport(dpControl);
	SHCONTROL *master = (SHCONTROL *)dpControl->buff;
	for (int i = 0; i < nbProcess; i++) {
		char tmp[256];
		if (pID[i] != 0) {
			int st = master->states[i];
			if (st == PROCESS_ERROR) {
				sprintf(tmp, "[#%d] Process [PID %d] %s: %s\n", i, pID[i], prStates[st], master->statusStr[i]);
			}


			else {
				sprintf(tmp, "[#%d] Process [PID %d] %s\n", i, pID[i], prStates[st]);
			}
		}

		else {
			sprintf(tmp, "[#%d] Process [PID ???] Not started\n", i);
		}
		strcat(err, tmp);
	}
	ReleaseDataport(dpControl);

	return err;
}

bool Worker::Wait(int waitState,LoadStatus *statusWindow) {
	
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

		for(size_t i=0;i<nbProcess;i++) {

			finished = finished & (shMaster->states[i]==waitState || shMaster->states[i]==PROCESS_ERROR || shMaster->states[i]==PROCESS_DONE);
			if( shMaster->states[i]==PROCESS_ERROR ) {
				error = true;

			}
			allDone = allDone & (shMaster->states[i]==PROCESS_DONE);
		}
		ReleaseDataport(dpControl);

		if (!finished) {

			if (statusWindow) {
				if (waitTime >= 500) statusWindow->SetVisible(true);
				statusWindow->SMPUpdate();
				mApp->DoEvents();
			}
			Sleep(250);
			waitTime+=250;
		}
	}

	if (statusWindow) statusWindow->SetVisible(false);
	return finished && !error;

}

bool Worker::ExecuteAndWait(int command,int readyState,size_t param) {

	if(!dpControl) return false;

	// Send command
	AccessDataport(dpControl);
	SHCONTROL *shMaster = (SHCONTROL *)dpControl->buff;
	for(size_t i=0;i<nbProcess;i++) {
		shMaster->states[i]=command;
		shMaster->cmdParam[i]=param;
	}
	ReleaseDataport(dpControl);

	Sleep(100);

	LoadStatus *statusWindow = NULL;
	statusWindow = new LoadStatus(this);
	bool result= Wait(readyState,statusWindow);
	SAFE_DELETE(statusWindow);
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
	running = false;
	if (nbProcess == 0)
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

	if( nbProcess==0 )
		throw Error("No sub process found. (Simulation not available)");

	if( !ExecuteAndWait(COMMAND_PAUSE,PROCESS_READY) )
		ThrowSubProcError();
}

void Worker::KillAll() {

	if( dpControl && nbProcess>0 ) {
		if( !ExecuteAndWait(COMMAND_EXIT,PROCESS_KILLED) ) {
			AccessDataport(dpControl);
			SHCONTROL *shMaster = (SHCONTROL *)dpControl->buff;
			for(size_t i=0;i<nbProcess;i++)
				if(shMaster->states[i]==PROCESS_KILLED) pID[i]=0;
			ReleaseDataport(dpControl);
			// Force kill
			for(size_t i=0;i<nbProcess;i++)
				if(pID[i]) KillProc(pID[i]);
		}
		CLOSEDP(dpHit);
	}
	nbProcess = 0;

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
		sprintf(cmdLine,"synradSub.exe %d %d",pid,i);
		#endif
		pID[i] = StartProc(cmdLine,STARTPROC_NORMAL);
		Sleep(25); // Wait a bit
		if( pID[i]==0 ) {
			nbProcess = 0;
			throw Error(cmdLine);
		}
	}

	nbProcess = n;

	LoadStatus *statusWindow = NULL;
	statusWindow = new LoadStatus(this);
	bool result = Wait(PROCESS_READY, statusWindow);
	SAFE_DELETE(statusWindow);
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
		if (mApp->needsTexture || mApp->needsDirection) try{ geom->BuildFacetTextures(buffer,mApp->needsTexture,mApp->needsDirection); }
		catch (Error &e) {
			ReleaseDataport(dpHit);
			throw e;
		}
		ReleaseDataport(dpHit);
	}
}

size_t Worker::GetProcNumber() {
	return nbProcess;
}


void Worker::Update(float appTime) {
	if (needsReload) RealReload();

	// Check calculation ending
	bool done = true;
	bool error = true;
	if (dpControl) {
		if (AccessDataport(dpControl)) {
			int i = 0;
			SHCONTROL *master = (SHCONTROL *)dpControl->buff;
			for (int i = 0; i<nbProcess && done; i++) {
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
	if ((error || done) && running && appTime != 0.0f) {
		InnerStop(appTime);
		if (error) ThrowSubProcError();
	}

	// Retrieve hit count recording from the shared memory
	if (dpHit) {

		if (AccessDataport(dpHit)) {
			BYTE *buffer = (BYTE *)dpHit->buff;

			mApp->changedSinceSave = true;
			// Globals
			SHGHITS *gHits = (SHGHITS *)buffer;

// Global hits and leaks
#ifdef MOLFLOW
			nbHit = gHits->total.hit.nbHit;
			nbAbsorption = gHits->total.hit.nbAbsorbed;
			nbDesorption = gHits->total.hit.nbDesorbed;
			distTraveledTotal_total = gHits->distTraveledTotal_total;
			distTraveledTotal_fullHitsOnly = gHits->distTraveledTotal_fullHitsOnly;
#endif

#ifdef SYNRAD
			
			nbHit = gHits->total.nbHit;
			nbAbsorption = gHits->total.nbAbsorbed;
			nbDesorption = gHits->total.nbDesorbed;
			totalFlux = gHits->total.fluxAbs;
			totalPower = gHits->total.powerAbs;
			distTraveledTotal = gHits->distTraveledTotal;

			if (nbDesorption && nbTrajPoints) {
				no_scans = (double)nbDesorption / (double)nbTrajPoints;
			}
			else {
				no_scans = 1.0;
			}
#endif
			nbLeakTotal = gHits->nbLeakTotal;
			hitCacheSize = gHits->hitCacheSize;
			memcpy(hitCache, gHits->hitCache, sizeof(HIT)*hitCacheSize);
			leakCacheSize = gHits->leakCacheSize;
			memcpy(leakCache, gHits->leakCache, sizeof(LEAK)*leakCacheSize); //will display only first leakCacheSize leaks

			// Refresh local facet hit cache for the displayed moment
			size_t nbFacet = geom->GetNbFacet();
			for (size_t i = 0; i<nbFacet; i++) {
				Facet *f = geom->GetFacet(i);
#ifdef SYNRAD
				memcpy(&(f->counterCache), buffer + f->sh.hitOffset, sizeof(SHHITS));
#endif
#ifdef MOLFLOW
				memcpy(&(f->counterCache), buffer + f->sh.hitOffset + displayedMoment * sizeof(SHHITS), sizeof(SHHITS));
				
				if (f->sh.recordAngleMap) {
					if (!f->sh.hasRecordedAngleMap) { //It was released by the user maybe
						//Initialize angle map
						f->angleMapCache = (size_t*)malloc(f->sh.angleMapPhiWidth * f->sh.angleMapThetaHeight * sizeof(size_t));
						if (!f->angleMapCache) {
							std::stringstream tmp;
							tmp << "Not enough memory for incident angle map on facet " << i + 1;
							throw Error(tmp.str().c_str());
						}
						f->sh.hasRecordedAngleMap = true;
					}
					BYTE* angleMapAddress = buffer
					+ f->sh.hitOffset
					+ (1 + moments.size()) * sizeof(SHHITS)
					+ (f->sh.isProfile ? PROFILE_SIZE * sizeof(APROFILE) *(1 + moments.size()) : 0)
					+ (f->sh.isTextured ? f->sh.texWidth*f->sh.texHeight * sizeof(AHIT) *(1 + moments.size()) : 0)
					+ (f->sh.countDirection ? f->sh.texWidth*f->sh.texHeight * sizeof(VHIT)*(1 + moments.size()) : 0);
					memcpy(f->angleMapCache, angleMapAddress, f->sh.angleMapPhiWidth*f->sh.angleMapThetaHeight * sizeof(size_t));	
					angleMapAddress = 0;
				}
				
#endif
			}
			try {
				if (mApp->needsTexture || mApp->needsDirection) geom->BuildFacetTextures(buffer, mApp->needsTexture, mApp->needsDirection);
			}
			catch (Error &e) {
				GLMessageBox::Display((char *)e.GetMsg(), "Error building texture", GLDLG_OK, GLDLG_ICONERROR);
				ReleaseDataport(dpHit);
				return;
			}
			ReleaseDataport(dpHit);
		}
	}
}

