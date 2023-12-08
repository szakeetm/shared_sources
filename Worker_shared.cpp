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
#ifdef _WIN32
#define NOMINMAX
//#include <Windows.h>
//#include <Process.h>
//#include <direct.h>
#else

#endif

#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "Worker.h"
#include "Facet_shared.h"
#include "GLApp/GLApp.h"
#include "GLApp/GLMessageBox.h"
#include "Helper/MathTools.h" //Min max
#include "GLApp/GLList.h"

#include "GLApp/GLUnitDialog.h"
#include "Interface/LoadStatus.h"
//#include "ProcessControl.h" // defines for process commands
//#include "SimulationManager.h"
//#include "Buffer_shared.h"

#include "SimulationFacet.h"

#if defined(MOLFLOW)
#include "../src/MolFlow.h"
#include "../src/MolflowGeometry.h"
#include "../src/Interface/FacetAdvParams.h"
#include "../src/Interface/GlobalSettings.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#include "../src/SynradGeometry.h"
#endif

//#include "File.h" //File utils (Get extension, etc)

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
extern MolFlow* mApp;
#endif

#if defined(SYNRAD)
extern SynRad* mApp;
#endif

Worker::~Worker() {
	simManager.KillSimulation(); // kill in case of preemptive Molflow close, prevents updates on already deleted globalsimustate
	delete interfGeom;
}

InterfaceGeometry* Worker::GetGeometry() {
	return interfGeom;
}

std::string Worker::GetCurrentFileName() const {
	return fullFileName;
}

// Given an absolute path (e.g. dir1\\dir2\\filename) it extracts and returns only the filename
std::string Worker::GetCurrentShortFileName() const {
	std::string::size_type filePos = fullFileName.rfind('/');
	if (filePos != std::string::npos)
		++filePos;
	else {
		filePos = fullFileName.rfind('\\');
		if (filePos != std::string::npos)
			++filePos;
		else
			filePos = 0;
	}

	return fullFileName.substr(filePos);
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

void Worker::SetCurrentFileName(const char* fileName) {
	fullFileName = fileName;
}

void Worker::ExportTextures(const char* fileName, int grouping, int mode, bool askConfirm, bool saveSelected) {

	// Read a file
	FILE* f = fopen(fileName, "w");
	if (!f) {
		char tmp[256];
		sprintf(tmp, "Cannot open file for writing %s", fileName);
		throw Error(tmp);
	}

#if defined(MOLFLOW)
	interfGeom->ExportTextures(f, grouping, mode, globalState, saveSelected);
#endif
#if defined(SYNRAD)
	interfGeom->ExportTextures(f, grouping, mode, no_scans, globalState, saveSelected);
#endif
	fclose(f);
}

void Worker::Stop_Public() {
	// Stop
	InnerStop(mApp->m_fTime);
	try {
		Stop();
		Update(mApp->m_fTime);
	}
	catch (const std::exception& e) {
		GLMessageBox::Display(e.what(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
	}
}

bool Worker::ReloadIfNeeded() {
	try {
		if (needsReload) RealReload();
	}
	catch (const std::exception& e) {
		GLMessageBox::Display(e.what(), "Error (RealReload)", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}
	return true;
}

std::shared_ptr<ParticleLog> Worker::GetLog() {
	try {
		if (!particleLog->particleLogMutex.try_lock_for(std::chrono::seconds(1)))
			throw Error("Couldn't get log access");
		ReloadIfNeeded();
	}
	catch (const std::exception& e) {
		GLMessageBox::Display(e.what(), "Error (GetLog)", GLDLG_OK, GLDLG_ICONERROR);
	}
	return particleLog;
}

void Worker::UnlockLog() {
	particleLog->particleLogMutex.unlock();
}

void Worker::ThrowSubProcError(const char* message) {

	char errMsg[1024];
	if (!message)
		sprintf(errMsg, "Bad response from sub process(es):\n%s", GetErrorDetails().c_str());
	else
		sprintf(errMsg, "%s\n%s", message, GetErrorDetails().c_str());
	throw Error(errMsg);

}

void Worker::MarkToReload() {
	//Schedules a reload
	//Actual reloading is done by RealReload() method
	needsReload = true;
#ifdef MOLFLOW
	if (mApp->globalSettings) mApp->globalSettings->UpdateOutgassing();
#endif
}

/*
void Worker::SetMaxDesorption(size_t max) {

	try {
		ResetStatsAndHits(0.0);

		desorptionLimit = max;
		Reload();

	}
	catch (const std::exception &e) {
		GLMessageBox::Display(e.what(), "Error", GLDLG_OK, GLDLG_ICONERROR);
	}

}
*/

std::string Worker::GetErrorDetails() {
	return simManager.GetErrorDetails();
}

void Worker::InnerStop(float appTime) {
	simuTimer.Stop();
}

void Worker::StartStop(float appTime) {
	if (IsRunning()) {

		// Stop
		InnerStop(appTime);
		try {
			Stop();
			Update(appTime);

		}
		catch (const std::exception& e) {
			GLMessageBox::Display((char*)e.what(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);

		}
	}
	else {

		// Start
		try {
			ReloadIfNeeded();
			Start();
			simuTimer.Start();
		}
		catch (const std::exception& e) {
			//isRunning = false;
			GLMessageBox::Display(e.what(), "Error (Start)", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}

		// Particular case when simulation ends before getting RUN state
		if (simManager.allProcsFinished) {
			Update(appTime);
			GLMessageBox::Display("Max desorption reached", "Information (Start)", GLDLG_OK, GLDLG_ICONINFO);
		}
	}

}

void Worker::ResetStatsAndHits(float appTime) {
	simuTimer.ReInit();
	if (simManager.nbThreads == 0)
		return;

	try {
		// First reset local states, then sim handles (to communicate cleared stats)
		ResetWorkerStats();
		{
			LoadStatus loadStatus(this);
			simManager.ResetSimulations(&loadStatus);
		}
		ReloadIfNeeded();
		Update(appTime);
	}
	catch (const std::exception& e) {
		GLMessageBox::Display(e.what(), "Error", GLDLG_OK, GLDLG_ICONERROR);
	}
}

void Worker::Stop() {
	LoadStatus loadWindow(this);
	simManager.StopSimulation(&loadWindow);
}

void Worker::InitSimProc() {

	simManager.nbThreads = 0; // set to 0 to init max threads

	// Launch n subprocess
	LoadStatus loadStatus(this);
	if (simManager.SetUpSimulation(&loadStatus)) {
		throw Error("Failed to init simulation.");
	}

	//if (!mApp->loadStatus) mApp->loadStatus = new LoadStatus(this);
}

void Worker::SetProcNumber(size_t n) {
	LoadStatus loadStatus(this);
	try {
		simManager.KillSimulation(&loadStatus);
	}
	catch (const std::exception&) {
		throw Error("Killing simulation failed");
	}

	simManager.nbThreads = std::clamp((size_t)n, (size_t)0, MAX_PROCESS);

	// Launch n subprocess
	if (simManager.SetUpSimulation(&loadStatus)) {
		throw Error("Starting subprocesses failed");
	}
}

size_t Worker::GetPID(size_t prIdx) {
	return 0;
}

#ifdef MOLFLOW
void Worker::CalculateTextureLimits() {
	// first get tmp limit

	MolflowSimulationModel* mf_model = dynamic_cast<MolflowSimulationModel*>(model.get());
	TEXTURE_MIN_MAX limits[3];
	for (auto& lim : limits) {
		lim.min.steady_state = lim.min.moments_only = std::numeric_limits<double>::infinity();
		lim.max.steady_state = lim.max.moments_only = -std::numeric_limits<double>::infinity();
	}

	for (const auto& facet : model->facets) {
		if (facet->sh.isTextured) {
			for (size_t m = 0; m < (1 + mf_model->tdParams.moments.size()); m++) {
				{
					// go on if the facet was never hit before
					auto& facetHitBuffer = globalState->facetStates[facet->globalId].momentResults[m].hits;
					if (facetHitBuffer.nbMCHit == 0 && facetHitBuffer.nbDesorbed == 0) continue;
				}

				//double dCoef = globalState->globalStats.globalStats.hit.nbDesorbed * 1E4 * model->sp.gasMass / 1000 / 6E23 * MAGIC_CORRECTION_FACTOR;  //1E4 is conversion from m2 to cm2
				const double timeCorrection =
					m == 0 ? model->sp.finalOutgassingRate : (model->sp.totalDesorbedMolecules) /
					interfaceMomentCache[m - 1].window;
				//model->sp.timeWindowSize;
				//Timecorrection is required to compare constant flow texture values with moment values (for autoscaling)
				const auto& texture = globalState->facetStates[facet->globalId].momentResults[m].texture;
				const size_t textureSize = texture.size();
				for (size_t t = 0; t < textureSize; t++) {
					//Add temporary hit counts

					if (facet->largeEnough[t]) {
						double val[3];  //pre-calculated autoscaling values (Pressure, imp.rate, density)

						val[0] = texture[t].sum_v_ort_per_area * timeCorrection; //pressure without dCoef_pressure
						val[1] = texture[t].countEquiv * facet->textureCellIncrements[t] * timeCorrection; //imp.rate without dCoef
						val[2] = texture[t].sum_1_per_ort_velocity * facet->textureCellIncrements[t] * timeCorrection; //particle density without dCoef

						//Global autoscale
						for (int v = 0; v < 3; v++) {
							if (m == 0) {
								limits[v].max.steady_state = std::max(val[v], limits[v].max.steady_state);

								if (val[v] > 0.0) {
									limits[v].min.steady_state = std::min(val[v], limits[v].min.steady_state);
								}
							}
							//Autoscale ignoring constant flow (moments only)
							else { //if (m != 0)
								limits[v].max.moments_only = std::max(val[v], limits[v].max.moments_only);

								if (val[v] > 0.0)
									limits[v].min.moments_only = std::min(val[v], limits[v].min.moments_only);
							}
						}
					} // if largeenough
				}
			}
		}
	}

	double dCoef_custom[] = { 1.0, 1.0, 1.0 };  //Three coefficients for pressure, imp.rate, density
	//Autoscaling limits come from the subprocess corrected by "time factor", which makes constant flow and moment values comparable
	//Time correction factor in subprocess: MoleculesPerTP * nbDesorbed
	dCoef_custom[0] = 1E4 / (double)globalState->globalStats.globalHits.nbDesorbed * mApp->worker.model->sp.gasMass / 1000 / 6E23 * 0.0100; //multiplied by timecorr*sum_v_ort_per_area: pressure
	dCoef_custom[1] = 1E4 / (double)globalState->globalStats.globalHits.nbDesorbed;
	dCoef_custom[2] = 1E4 / (double)globalState->globalStats.globalHits.nbDesorbed;

	// Add coefficient scaling
	for (int v = 0; v < 3; ++v) {
		limits[v].max.steady_state *= dCoef_custom[v];
		limits[v].max.moments_only *= dCoef_custom[v];
		limits[v].min.steady_state *= dCoef_custom[v];
		limits[v].min.moments_only *= dCoef_custom[v];
	}

	// Last put temp limits into global struct
	auto geom = GetGeometry();
	for (int v = 0; v < 3; ++v) {
		/*
		if (!std::isinf(limits[v].min.steady_state)) geom->texture_limits[v].autoscale.min.steady_state = limits[v].min.steady_state;
		if (!std::isinf(limits[v].max.steady_state)) geom->texture_limits[v].autoscale.max.steady_state = limits[v].max.steady_state;
		if (!std::isinf(limits[v].min.moments_only)) geom->texture_limits[v].autoscale.min.moments_only = limits[v].min.moments_only;
		if (!std::isinf(limits[v].max.moments_only)) geom->texture_limits[v].autoscale.max.moments_only = limits[v].max.moments_only;
		*/		
		geom->texture_limits[v].autoscale = limits[v];
	}
	
}
#endif

#ifdef SYNRAD
#define HITMAX 1E38
void Worker::CalculateTextureLimits() {
	// first get tmp limit
	TextureCell limitMin, limitMax;
	TEXTURE_MIN_MAX limits[3]; // count, flux, power
	for (auto& lim : limits) {
		lim.max = 0;
		lim.min = HITMAX;
	}

	for (const auto& facet : model->facets) {
		auto& subF = *facet;
		if (subF.sh.isTextured) {
			{
				// go on if the facet was never hit before
				auto& facetHitBuffer = globalState->facetStates[subF.globalId].momentResults[0].hits;
				if (facetHitBuffer.nbMCHit == 0 && facetHitBuffer.nbDesorbed == 0) continue;
			}

			const auto& texture = globalState->facetStates[subF.globalId].momentResults[0].texture;
			const size_t textureSize = texture.size();
			for (size_t t = 0; t < textureSize; t++) {
				//Add temporary hit counts

				if (subF.largeEnough[t]) { // TODO: For count it wasn't applied in Synrad so far
					double val[3];  //pre-calculated autoscaling values (Pressure, imp.rate, density)

					val[0] = texture[t].count;
					val[1] = texture[t].flux /** subF.textureCellIncrements[t]*/;
					val[2] = texture[t].power /** subF.textureCellIncrements[t]*/;

					//Global autoscale
					for (int v = 0; v < 3; v++) {
						limits[v].max = std::max(val[v], limits[v].max);

						if (val[v] > 0.0) {
							limits[v].min = std::min(val[v], limits[v].min);
						}
					}
				} // if largeenough
			}
		}
	}

	/*double dCoef_custom[] = { 1.0, 1.0, 1.0 };  //Three coefficients for pressure, imp.rate, density
	//Autoscaling limits come from the subprocess corrected by "time factor", which makes constant flow and moment values comparable
	//Time correction factor in subprocess: MoleculesPerTP * nbDesorbed
	dCoef_custom[0] = 1E4 / (double)globalState->globalStats.globalStats.nbDesorbed * mApp->worker.model->sp.gasMass / 1000 / 6E23*0.0100; //multiplied by timecorr*sum_v_ort_per_area: pressure
	dCoef_custom[1] = 1E4 / (double)globalState->globalStats.globalStats.nbDesorbed;
	dCoef_custom[2] = 1E4 / (double)globalState->globalStats.globalStats.nbDesorbed;

	// Add coefficient scaling
	for(int v = 0; v < 3; ++v) {
		limits[v].max.steady_state *= dCoef_custom[v];
		limits[v].max.moments_only *= dCoef_custom[v];
		limits[v].min.steady_state *= dCoef_custom[v];
		limits[v].min.moments_only *= dCoef_custom[v];
	}*/

	// Last put temp limits into global struct
	for (int v = 0; v < 3; ++v) {
		GetGeometry()->texture_limits[v].autoscale = limits[v];
	}
}
#endif



void Worker::RebuildTextures() {

	if (mApp->needsTexture || mApp->needsDirection) {

		// Only reload if we are even rebuilding textures
		ReloadIfNeeded();
		auto lock = GetHitLock(globalState.get(), 10000);
		if (!lock) return;
		CalculateTextureLimits();
		interfGeom->BuildFacetTextures(globalState, mApp->needsTexture, mApp->needsDirection);
	}
}

size_t Worker::GetProcNumber() const {
	return simManager.nbThreads;
}

bool Worker::IsRunning() {
	// In case a simulation ended prematurely escaping the check routines in FrameMove (only executed after at least one second)
	bool state = simManager.IsRunning();
	if (!state && simuTimer.isActive) {
		simuTimer.Stop();
	}
	return state;
}

void Worker::Update(float appTime) {
	//Refreshes interface cache:
	//Global statistics, leak/hits, global histograms
	//Facet hits, facet histograms, facet angle maps
	//No cache for profiles, textures, directions (plotted directly from shared memory hit buffer)

	if (!ReloadIfNeeded()) return;

	// End of simulation reached (Stop GUI)
	if ((simManager.allProcsFinished || simManager.hasErrorStatus) && (IsRunning() || (!IsRunning() && simuTimer.isActive)) && (appTime != 0.0f)) {
		InnerStop(appTime);
		if (simManager.hasErrorStatus) ThrowSubProcError();
	}

	// Retrieve hit count recording from the shared memory
	// Globals
	if (!globalState->initialized || !simManager.nbThreads) return;
	mApp->changedSinceSave = true;

	size_t waitTime = (this->simManager.isRunning) ? 100 : 10000;
	auto lock = GetHitLock(globalState.get(),waitTime);
	if (!lock) return;
	globalStatCache = globalState->globalStats; //Make a copy for quick GUI access (nbDesorbed, etc. can't always wait for lock)

#if defined(SYNRAD)

	if (globalStatCache.globalHits.nbDesorbed && model->sp.nbTrajPoints) {
		no_scans = (double)globalStatCache.globalHits.nbDesorbed / (double)model->sp.nbTrajPoints;
	}
	else {
		no_scans = 1.0;
	}
#endif


	//Copy global histogram
	UpdateInterfaceCaches();

	try {
		if (mApp->needsTexture || mApp->needsDirection) {
			CalculateTextureLimits();
			interfGeom->BuildFacetTextures(globalState, mApp->needsTexture, mApp->needsDirection);
		}
	}
	catch (const std::exception& e) {
		GLMessageBox::Display(e.what(), "Error building texture", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}

#if defined(MOLFLOW)
	if (mApp->facetAdvParams && mApp->facetAdvParams->IsVisible() && needsAngleMapStatusRefresh) {
		needsAngleMapStatusRefresh = false;
		mApp->facetAdvParams->Refresh(interfGeom->GetSelectedFacets());
	}
#endif
}

void Worker::GetProcStatus(ProcComm& procInfoList) {
	simManager.GetProcStatus(procInfoList);
}

/*
void Worker::ChangePriority(int prioLevel) {
	if (prioLevel)
		simManager.IncreasePriority();
	else
		simManager.DecreasePriority();
}
*/

void Worker::ChangeSimuParams() { //Send simulation mode changes to subprocesses without reloading the whole geometry
	if (simManager.nbThreads == 0 || !interfGeom->IsLoaded()) return;
	ReloadIfNeeded(); //Sync (number of) regions

	//auto *prg = new GLProgress_GUI("Creating dataport...", "Passing simulation mode to workers");
	//prg.SetVisible(true);

	//To do: only close if parameters changed
	//prg.SetMessage("Waiting for subprocesses to release log dataport...");

	particleLog->clear();

	//prg.SetProgress(0.5);
	//prg.SetMessage("Assembling parameters to pass...");

	std::string loaderString = SerializeParamsForLoader().str();
	simManager.SetOntheflyParams(&model->otfParams);
	try {
		LoadStatus loadStatus(this);
		simManager.ShareWithSimUnits((BYTE*)loaderString.c_str(), loaderString.size(), LoadType::LOADPARAM, &loadStatus);
	}
	catch (Error& err) {
		auto dlg = GLMessageBox::Display(err.what(), "Couldn't change simulation parameters", GLDLG_OK, GLDLG_ICONERROR);
	}
}

/**
* \brief Saves current facet hit counter from cache to results, only to const. flow (moment 0)
* Sufficient for .geo and .txt formats, for .xml moment results are written during the loading
*/
void Worker::FacetHitCacheToSimModel() {
	size_t nbFacet = interfGeom->GetNbFacet();
	std::vector<FacetHitBuffer*> facetHitCaches;
	for (size_t i = 0; i < nbFacet; i++) {
		InterfaceFacet* f = interfGeom->GetFacet(i);
		facetHitCaches.push_back(&f->facetHitCache);
	}
	simManager.SetFacetHitCounts(facetHitCaches);
}

void Worker::UpdateInterfaceCaches()
{
	//Gets hits, histograms and angle maps for currently displayed moment
	//Global: histograms
	//Facets: hits, histograms and angle maps

	//GLOBAL HISTOGRAMS
	//Prepare vectors to receive data
#if defined(MOLFLOW)
	globalHistogramCache = globalState->globalHistograms[displayedMoment];
#endif
#if defined(SYNRAD)
	if (!globalState->globalHistograms.empty()) {
		globalHistogramCache = globalState->globalHistograms[0];
	}
#endif
	//FACET HISTOGRAMS
	for (size_t i = 0; i < interfGeom->GetNbFacet(); i++) {
		InterfaceFacet* f = interfGeom->GetFacet(i);
#if defined(MOLFLOW)
		f->facetHitCache = globalState->facetStates[i].momentResults[displayedMoment].hits;
		f->facetHistogramCache = globalState->facetStates[i].momentResults[displayedMoment].histogram;
		if (f->sh.anglemapParams.record) { //Recording, so needs to be updated
			if (f->sh.desorbType != DES_ANGLEMAP) { //safeguard that not desorbing and recordig at same time, should not happen
				if (f->selected && f->angleMapCache.empty() && !globalState->facetStates[i].recordedAngleMapPdf.empty()) needsAngleMapStatusRefresh = true; //angleMapCache copied during an update
				f->angleMapCache = globalState->facetStates[i].recordedAngleMapPdf;
			}
		}
#endif
#if defined(SYNRAD)
		f->facetHitCache = globalState->facetStates[i].momentResults[0].hits;
		f->facetHistogramCache = globalState->facetStates[i].momentResults[0].histogram;
#endif
	}
}

// returns -1 on error, 0 on success
void Worker::ReloadSim(bool sendOnly, GLProgress_Abstract& prg) {
	// Send and Load geometry
	prg.SetMessage("Converting geometry to simulation model...");
	
	InterfaceGeomToSimModel();

	prg.SetMessage("Initializing physics...");
	try {
		model->PrepareToRun();
	}
	catch (std::exception& err) {
		throw Error(("Error in model->PrepareToRun():\n{}", err.what()));
	}

	prg.SetMessage("Constructing memory structure to store results...");
	if (!sendOnly) {
		globalState->Resize(model);
	}

	prg.SetMessage("Forwarding simulation model...");
	simManager.ShareSimModel(model); //set shared pointer simManager::simulation.model to worker::model
	prg.SetMessage("Forwarding global simulation state...");
	simManager.ShareGlobalCounter(globalState, particleLog);  //set worker::globalState and particleLog pointers to simManager::simulations[0]
	
}