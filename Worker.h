#pragma once

#include <string>
#include <vector>
#include "GLApp/GLTypes.h"
#include "Smp.h"
#include "Buffer_shared.h" //LEAK, HIT

class Geometry;
class GLProgress;
class LoadStatus;


#ifdef MOLFLOW
#include "Parameter.h"
#include "Vector.h" //moving parts
#include "MolflowTypes.h"

#define CDF_SIZE 100 //points in a cumulative distribution function

class MolflowGeometry;
#endif

#ifdef SYNRAD
#include "Region_full.h"

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

  
  void LoadGeometry(char *fileName, bool insert=false, bool newStr=false);// Loads or inserts a geometry (throws Error)
  void LoadTexturesSYN(FileReader* f,int version);  // Load a textures(throws Error)
  void RebuildTextures();
  
  // Save a geometry (throws Error)
  void SaveGeometry(char *fileName,GLProgress *prg,bool askConfirm=true,bool saveSelected=false,bool autoSave=false,bool crashSave=false);
  bool IsDpInitialized();
  
  // Export textures (throws Error)
  void ExportTextures(char *fileName,int grouping,int mode,bool askConfirm=true,bool saveSelected=false);
  //void ExportRegionPoints(char *fileName,GLProgress *prg,int regionId,int exportFrequency,bool doFullScan);
  //void ExportDesorption(char *fileName,bool selectedOnly,int mode,double eta0,double alpha,const Distribution2D &distr);

  std::vector<std::vector<std::string>> ImportCSV_string(FileReader *file);
  std::vector<std::vector<double>> ImportCSV_double(FileReader *file);

  // Return/Set the current filename
  char *GetFileName();
  char *GetShortFileName();
  char *GetShortFileName(char* longFileName);
  void  SetFileName(char *fileName);

  void SetProcNumber(size_t n);// Set number of processes [1..32] (throws Error)
  size_t GetProcNumber();  // Get number of processes
  void SetMaxDesorption(llong max);// Set the number of maximum desorption
  DWORD GetPID(size_t prIdx);// Get PID
  void ResetStatsAndHits(float appTime);
  void Reload();    // Reload simulation (throws Error)
  void RealReload();
  void ChangeSimuParams();
  void Stop_Public();// Switch running/stopped
  //void Exit(); // Free all allocated resource
  void KillAll();// Kill all sub processes
  void Update(float appTime);// Get hit counts for sub process
  void SetLeakCache(LEAK *buffer,size_t *nb,Dataport *dpHit);// Set Leak
  void SetHitCache(HIT *buffer,size_t *nb, Dataport *dpHit);  // Set HHit
  void GetProcStatus(size_t *states,std::vector<std::string>& statusStrings);// Get process status
  BYTE *GetHits(); // Access to dataport (HIT)
  void ReleaseHits();
 
  void RemoveRegion(int index);
  void AddRegion(const char *fileName,int position=-1); //load region (position==-1: add as new region)
  void RecalcRegion(int regionId);
  void SaveRegion(char *fileName,int position,bool overwrite=false);

#ifdef MOLFLOW
  MolflowGeometry* GetMolflowGeometry();
  void ExportProfiles(char *fileName);
  void ExportAngleMaps(std::vector<size_t> faceList, std::string fileName);
  void AnalyzeSYNfile(char *fileName, size_t *nbFacet, size_t *nbTextured, size_t *nbDifferent);
  void ImportDesorption_SYN(char *fileName, const size_t &source, const double &time,
	  const size_t &mode, const double &eta0, const double &alpha, const double &cutoffdose,
	  const std::vector<std::pair<double, double>> &convDistr,
	  GLProgress *prg);
  void LoadTexturesGEO(FileReader *f, int version);
  void OneACStep();
  void StepAC(float appTime); // AC iteration single step
  void ComputeAC(float appTime); // Send Compute AC matrix order
  void PrepareToRun(); //Do calculations necessary before launching simulation
  int GetParamId(const std::string); //Get ID of parameter name
  int AddMoment(std::vector<double> newMoments); //Adds a time serie to moments and returns the number of elements
  std::vector<double> ParseMoment(std::string userInput); //Parses a user input and returns a vector of time moments
  void ResetMoments();
  double GetMoleculesPerTP(size_t moment);
  std::vector<std::pair<double, double>> Generate_ID(int paramId);
  int GenerateNewID(int paramId);
  std::vector<std::pair<double, double>> Generate_CDF(double gasTempKelvins, double gasMassGramsPerMol, size_t size);
  int GenerateNewCDF(double temperature);
  void CalcTotalOutgassing();
  int GetCDFId(double temperature);
  int GetIDId(int paramId);
  //Different signature:
  void SendHits(bool skipFacetHits = false);// Send total and facet hit counts to subprocesses
  void StartStop(float appTime,size_t sMode);    // Switch running/stopped
#endif

#ifdef SYNRAD
  SynradGeometry* GetSynradGeometry();
  void AddMaterial(std::string *fileName);
  void ClearRegions();
  //Different signature:
  void SendHits();// Send total and facet hit counts to subprocesses
  void StartStop(float appTime);    // Switch running/stopped
#endif

  // Global simulation parameters
  OntheflySimulationParams ontheflyParam;

  double  nbAbsEquiv;      // Total number of molecules absorbed (64 bit integer)
  size_t  nbDesorption;      // Total number of molecules generated (64 bit integer)
  size_t  nbMCHit;             // Total number of hit (64 bit integer)
  double  nbHitEquiv;          // Equivalent number of hits (low-flux mode), for MFP calculation
  size_t  nbLeakTotal;            // Total number of leak
  
  size_t  desorptionLimit;     // Number of desoprtion before halting
  double distTraveled_total; // Total distance traveled by particles (for mean free path calc.)

  bool   isRunning;           // Started/Stopped state
  float  startTime;         // Start time
  float  stopTime;          // Stop time
  float  simuTime;          // Total simulation time

  char fullFileName[512]; // Current loaded file

  bool needsReload;
  bool abortRequested;

  bool calcAC; //Not used in Synrad, kept for ResetStatsAndHits function shared with Molflow

			   // Caches
  HIT  hitCache[HITCACHESIZE];
  LEAK leakCache[LEAKCACHESIZE];
  size_t hitCacheSize;            // Total number of hhit
  size_t leakCacheSize;
  
#ifdef MOLFLOW
  size_t sMode; //MC or AC

  double totalDesorbedMolecules; //Number of molecules desorbed between t=0 and latest_moment
  double finalOutgassingRate; //Number of outgassing molecules / second at latest_moment (constant flow)
  double finalOutgassingRate_Pa_m3_sec; //For the user to see on Global Seetings and in formulas. Not shared with workers
  double gasMass;
  bool   enableDecay;
  double halfLife;
  double timeWindowSize;
  bool useMaxwellDistribution; //true: Maxwell-Boltzmann distribution, false: All molecules have the same (V_avg) speed
  bool calcConstantFlow;

  int motionType;
  Vector3d motionVector1; //base point for rotation
  Vector3d motionVector2; //rotation vector or velocity vector

  std::vector<Parameter> parameters;
  int displayedMoment;

  double distTraveledTotal_fullHitsOnly; // Total distance traveled by particles between full hits (for mean free path calc.)

  std::vector<std::vector<std::pair<double, double>>> CDFs; //cumulative distribution function for each temperature
  std::vector<std::vector<std::pair<double, double>>> IDs; //integrated distribution function for each time-dependent desorption type
  std::vector<double> temperatures; //keeping track of all temperatures that have a CDF already generated
  std::vector<double> moments;             //moments when a time-dependent simulation state is recorded
  std::vector<size_t> desorptionParameterIDs; //time-dependent parameters which are used as desorptions, therefore need to be integrated
  double latestMoment;
  std::vector<std::string> userMoments;    //user-defined text values for defining time moments (can be time or time series)

  size_t    calcACprg;         // AC matrix progress
#endif

#ifdef SYNRAD
	double totalFlux;         // Total desorbed Flux
	double totalPower;        // Total desorbed power

	size_t    nbTrajPoints;       // number of all points in trajectory
	double no_scans;           // = nbDesorption/nbTrajPoints. Stored separately for saving/loading

	bool   newReflectionModel;
	std::vector<Region_full> regions;
	std::vector<Material> materials;
	std::vector<std::vector<double>> psi_distro; //psi-energy map for full (par+ort) polarization
	std::vector<std::vector<double>> parallel_polarization; //ratio of parallel polarization for a given E/E_crit ratio and psi vertical angle
	std::vector<std::vector<std::vector<double>>> chi_distros; //3 psi-chi    maps for full/parallel/orthogonal polarizations
#endif

private:

  // Process management
  size_t    nbProcess;
  DWORD  pID[MAX_PROCESS];
  DWORD  pid;
  bool   allDone;

  // Dataport handles and names
  Dataport *dpControl;
  Dataport *dpHit;

  char      ctrlDpName[32];
  char      loadDpName[32];
  char      hitsDpName[32];


  // Methods
  bool ExecuteAndWait(int command, size_t waitState, size_t param = 0);
  bool Wait(size_t waitState, LoadStatus *statusWindow);
  void ResetWorkerStats();
  void ClearHits(bool noReload);
  char *GetErrorDetails();
  void ThrowSubProcError(std::string message);
  void ThrowSubProcError(char *message = NULL);
  void Start();
  void Stop();
  void InnerStop(float appTime);

  // Geometry handle
#ifdef MOLFLOW
  MolflowGeometry *geom;
#endif
#ifdef SYNRAD
  SynradGeometry *geom;
  Dataport *dpMat;
  char      materialsDpName[32];
#endif

};