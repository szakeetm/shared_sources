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
#pragma once

#include <string>
#include <vector>
#include "Buffer_shared.h" //LEAK, HIT
#include "SimulationManager.h"

class Geometry;
class GLProgress;
class LoadStatus;


#if defined(MOLFLOW)
#include "../src/Parameter.h"

#define CDF_SIZE 100 //points in a cumulative distribution function

class MolflowGeometry;
#endif

#if defined(SYNRAD)
#include "../src/Region_full.h"

class SynradGeometry;
class Material;
#endif


class Worker
{

public:

  // Constructor
  Worker();
  ~Worker();

  // Return a handle to the currently loaded geometry
  Geometry *GetGeometry();

  
  void LoadGeometry(const std::string& fileName, bool insert=false, bool newStr=false);// Loads or inserts a geometry (throws Error)
  void LoadTexturesSYN(FileReader* f, BYTE* buffer, int version);  // Load a textures(throws Error)
  void RebuildTextures();
  
  // Save a geometry (throws Error)
  void SaveGeometry(std::string fileName,GLProgress *prg,bool askConfirm=true,bool saveSelected=false,bool autoSave=false,bool crashSave=false);

  // Export textures (throws Error)
  void ExportTextures(const char *fileName,int grouping,int mode,bool askConfirm=true,bool saveSelected=false);
  //void ExportRegionPoints(const char *fileName,GLProgress *prg,int regionId,int exportFrequency,bool doFullScan);
  //void ExportDesorption(const char *fileName,bool selectedOnly,int mode,double eta0,double alpha,const Distribution2D &distr);

  std::vector<std::vector<std::string>> ImportCSV_string(FileReader *file);
  std::vector<std::vector<double>> ImportCSV_double(FileReader *file);

  // Return/Set the current filename
  char *GetCurrentFileName();
  char *GetCurrentShortFileName();
  //char *GetShortFileName(char* longFileName);
  void  SetCurrentFileName(const char *fileName);

  void SetProcNumber(size_t n);// Set number of processes [1..32] (throws Error)
  size_t GetProcNumber() const;  // Get number of processes
 // void SetMaxDesorption(size_t max);// Set the number of maximum desorption
 size_t GetPID(size_t prIdx);// Get PID
  void ResetStatsAndHits(float appTime);
  void Reload();    // Reload simulation (throws Error)
  void RealReload(bool sendOnly=false);
  std::ostringstream SerializeForLoader();
    virtual std::ostringstream SerializeParamsForLoader();

  void SerializeForExternal(std::string outputName);
    void ChangeSimuParams();
  void Stop_Public();// Switch running/stopped
  //void Exit(); // Free all allocated resource
  void KillAll(bool keppDpHit=false);// Kill all sub processes
  void Update(float appTime);// Get hit counts for sub process
  void RetrieveHistogramCache(BYTE* dpHitStartAddress);
  //void SendLeakCache(Dataport *dpHit); // From worker cache to dpHit shared memory
  //void SendHitCache(Dataport *dpHit);  // From worker cache to dpHit shared memory
  void GetProcStatus(size_t *states,std::vector<std::string>& statusStrings);// Get process status
    void GetProcStatus(std::vector<SubProcInfo>& procInfoList);// Get process status

    BYTE *GetHits(); // Access to dataport (HIT)
  std::tuple<size_t,ParticleLoggerItem*> GetLogBuff();
  void ReleaseLogBuff();
  void ReleaseHits();
 
  void RemoveRegion(int index);
  void AddRegion(const char *fileName,int position=-1); //load region (position==-1: add as new region)
  void RecalcRegion(int regionId);
  void SaveRegion(const char *fileName,int position,bool overwrite=false);
  bool CheckFilenameConflict(const std::string& newPath, const size_t& regionId, std::vector<std::string>& paths, std::vector<std::string>& fileNames, std::vector<size_t>& regionIds);

  FileReader* ExtractFrom7zAndOpen(const std::string& fileName, const std::string& geomName);

#if defined(MOLFLOW)
  MolflowGeometry* GetMolflowGeometry();
  void ExportProfiles(const char *fileName);
  std::vector<std::string> ExportAngleMaps(std::string fileName, bool saveAll=false);
  bool ImportAngleMaps(std::string fileName);

  void AnalyzeSYNfile(const char *fileName, size_t *nbFacet, size_t *nbTextured, size_t *nbDifferent);
  void ImportDesorption_SYN(const char *fileName, const size_t &source, const double &time,
	  const size_t &mode, const double &eta0, const double &alpha, const double &cutoffdose,
	  const std::vector<std::pair<double, double>> &convDistr,
	  GLProgress *prg);
  void LoadTexturesGEO(FileReader *f, int version);
  void OneACStep();
  void StepAC(float appTime); // AC iteration single step
  void ComputeAC(float appTime); // Send Compute AC matrix order
  void PrepareToRun(); //Do calculations necessary before launching simulation
  int GetParamId(const std::string); //Get ID of parameter name
  void SendFacetHitCounts();
  static int CheckIntervalOverlap(const std::vector<Moment>& vecA, const std::vector<Moment>& vecB);
  static std::pair<int, int> CheckIntervalOverlap(const std::vector<std::vector<Moment>>& vecParsedMoments);
  int AddMoment(std::vector<Moment> newMoments); //Adds a time serie to moments and returns the number of elements
  std::vector<Moment> ParseMoment(std::string userInput, double timeWindow); //Parses a user input and returns a vector of time moments
  void ResetMoments();
  double GetMoleculesPerTP(size_t moment);
  IntegratedDesorption Generate_ID(int paramId);
  int GenerateNewID(int paramId);
  std::vector<std::pair<double, double>> Generate_CDF(double gasTempKelvins, double gasMassGramsPerMol, size_t size);
  int GenerateNewCDF(double temperature);
  void CalcTotalOutgassing();
  int GetCDFId(double temperature);
  int GetIDId(int paramId);
  //Different signature:
  void SendToHitBuffer();// Send total and facet hit counts to subprocesses
  void StartStop(float appTime,size_t sMode);    // Switch running/stopped
  #endif

#if defined(SYNRAD)
  SynradGeometry* GetSynradGeometry();
  void AddMaterial(std::string *fileName);
  void ClearRegions();
  void WriteHitBuffer();
    //Different signature:
    void SendFacetHitCounts();
    void SendToHitBuffer();// Send total and facet hit counts to subprocesses
  void StartStop(float appTime);    // Switch running/stopped
#endif
    bool   IsRunning();           // Started/Stopped state

  // Global simulation parameters
  OntheflySimulationParams ontheflyParams;
  WorkerParams wp;
  GlobalHitBuffer globalHitCache;
  FacetHistogramBuffer globalHistogramCache;

  float  startTime;         // Start time
  float  stopTime;          // Stop time
  float  simuTime;          // Total simulation time

  char fullFileName[512]; // Current loaded file

  bool needsReload;
  bool abortRequested;

  bool calcAC; //Not used in Synrad, kept for ResetStatsAndHits function shared with Molflow

#if defined(MOLFLOW)

  std::vector<Parameter> parameters;
  int displayedMoment;
  size_t InsertParametersBeforeCatalog(const std::vector<Parameter>& newParams);

  std::vector<std::vector<std::pair<double, double>>> CDFs; //cumulative distribution function for each temperature
  std::vector<IntegratedDesorption> IDs; //integrated distribution function for each time-dependent desorption type
  std::vector<double> temperatures; //keeping track of all temperatures that have a CDF already generated
  std::vector<size_t> desorptionParameterIDs; //time-dependent parameters which are used as desorptions, therefore need to be integrated
    std::vector<Moment> moments;             //moments when a time-dependent simulation state is recorded
    std::vector<UserMoment> userMoments;    //user-defined text values for defining time moments (can be time or time series)

  size_t    calcACprg;         // AC matrix progress
#endif

#if defined(SYNRAD)

	double no_scans;           // = nbDesorption/nbTrajPoints. Stored separately for saving/loading

	std::vector<Region_full> regions;
	std::vector<Material> materials;
	std::vector<std::vector<double>> psi_distro; //psi-energy map for full (par+ort) polarization
	std::vector<std::vector<double>> parallel_polarization; //ratio of parallel polarization for a given E/E_crit ratio and psi vertical angle
	std::vector<std::vector<std::vector<double>>> chi_distros; //3 psi-chi    maps for full/parallel/orthogonal polarizations
#endif
	/*
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			CEREAL_NVP(wp),
			CEREAL_NVP(ontheflyParams),
			CEREAL_NVP(CDFs),
			CEREAL_NVP(IDs),
			CEREAL_NVP(parameters),
			CEREAL_NVP(temperatures),
			CEREAL_NVP(moments),
			CEREAL_NVP(desorptionParameterIDs)
		);
	}
	*/
private:

  // Process management
  SimulationManager simManager;

  //size_t  pID[MAX_PROCESS];
  //DWORD  pid;
  //bool   allDone;

  //Dataport handles and names
  //Dataport *dpControl;
  //Dataport *dpHit;
  //Dataport *dpLog;

  //char      ctrlDpName[32];
  //char      loadDpName[32];
  //char      hitsDpName[32];
  //char      logDpName[32];

  // Methods
  //bool ExecuteAndWait(int command, size_t waitState, size_t param = 0);
  //bool Wait(size_t waitState, LoadStatus *statusWindow);
  void ResetWorkerStats();
  //void ClearHits();
  const char *GetErrorDetails();
  //void ThrowSubProcError(std::string message);
  void ThrowSubProcError(const char *message = nullptr);
  void Start();
  void Stop();
  void InnerStop(float appTime);

  // Geometry handle
#if defined(MOLFLOW)
  MolflowGeometry *geom;

#endif
#if defined(SYNRAD)
  SynradGeometry *geom;
  Dataport *dpMat;
#endif
};