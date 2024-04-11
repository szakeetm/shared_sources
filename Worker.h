
#pragma once

#include <string>
#include <vector>
#include <set>
#include <list>

#include "Worker.h"
#include <Helper/Chronometer.h>
#include "Buffer_shared.h" //LEAK, HIT
#include "SimulationManager.h"
#include "Buffer_shared.h"
#include "GLApp/GLProgress_GUI.hpp"

//Molflow/Synrad-specific
class InterfaceGeometry;
class GLProgress_Abstract;
struct InterfaceSettings;
struct ParticleLog;
struct UserMoment;
class GlobalSimuState;


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


class Worker {

public:

  // Constructor
  Worker();
  ~Worker();

  // Return a handle to the currently loaded geometry
  InterfaceGeometry *GetGeometry();

  
  void LoadGeometry(const std::string& fileName, bool insert=false, bool newStr=false);// Loads or inserts a geometry (throws Error)
  void RebuildTextures();
    void CalculateTextureLimits();

  // Save a geometry (throws Error)
  void SaveGeometry(std::string fileName,GLProgress_Abstract& prg,bool askConfirm=true,bool saveSelected=false,bool autoSave=false,bool crashSave=false);

  // Export textures (throws Error)
  void ExportTextures(const char *fileName,int grouping,int mode,bool askConfirm=true,bool saveSelected=false);
  
  // Return/Set the current filename
  std::string GetCurrentFileName() const;
  std::string GetCurrentShortFileName() const;
  //char *GetShortFileName(char* longFileName);
  void  SetCurrentFileName(const char *fileName);

  void InitSimProc(); // Init default CPU proc with max threads
  void SetProcNumber(size_t n);// Set number of processes [1..32] (throws Error)
  size_t GetProcNumber() const;  // Get number of processes
 // void SetMaxDesorption(size_t max);// Set the number of maximum desorption
 static size_t GetPID(size_t prIdx);// Get PID
  void ResetStatsAndHits(float appTime);
  void MarkToReload();    // Reload simulation (throws Error)
  void ReloadSim(bool sendOnly, GLProgress_Abstract& prg);
  void RealReload(bool sendOnly=false); //throws error
  //std::ostringstream SerializeForLoader();
    virtual std::ostringstream SerializeParamsForLoader();

    void ChangeSimuParams();
  void Stop_Public();// Switch running/stopped
    void StartStop(float appTime);    // Switch running/stopped
    //void Exit(); // Free all allocated resource
  //void KillAll(bool keppDpHit=false);// Kill all sub processes
  void Update(float appTime);// Get hit counts for sub process
  void UpdateInterfaceCaches();
  //void SendLeakCache(Dataport *dpHit); // From worker cache to dpHit shared memory
  //void SendHitCache(Dataport *dpHit);  // From worker cache to dpHit shared memory
    void GetProcStatus(ProcComm &procInfoList);// Get process status
    //void ChangePriority(int prioLevel);

    bool ReloadIfNeeded(); // Access to dataport (HIT)
    std::shared_ptr<ParticleLog> GetLog();
    void UnlockLog();

    void InterfaceGeomToSimModel();
    std::unique_ptr<MolflowInterfaceSettings> InterfaceSettingsToWriterSettings(std::shared_ptr<SimulationModel> model);
    void SimModelToInterfaceGeom();
    void LoaderSettingsToInterfaceSettings(const std::unique_ptr<MolflowInterfaceSettings>& interfaceSettings, GLProgress_GUI& prg);

    std::string GetSimManagerStatus();

#if defined(MOLFLOW)
  MolflowGeometry* GetMolflowGeometry();
  void ExportProfiles(const char *fileName);
  std::optional<std::vector<std::string>> ExportAngleMaps(const std::string& fileName, bool saveAll=false);

  void AnalyzeSYNfile(const char *fileName, size_t *nbFacet, size_t *nbTextured, size_t *nbDifferent);
  void ImportDesorption_SYN(const char *fileName, const size_t source, const double time,
	  const size_t mode, const double eta0, const double alpha, const double cutoffdose,
	  const std::vector<std::pair<double, double>> &convDistr,
	  GLProgress_Abstract& prg);
  void LoadTexturesGEO(FileReader& f, int version);
  void FacetHitCacheToSimModel(); //only .geo and .txt with no time-dep moment state loading
  int SendAngleMaps();
  void ResetMoments();

  double GetMoleculesPerTP(size_t moment) const;
  //Different signature:
  #endif

#if defined(SYNRAD)
  SynradGeometry* GetSynradGeometry();
  void AddMaterial(std::string *fileName);
  void ClearRegions();
  void WriteHitBuffer();
    //Different signature:
    void FacetHitCacheToSimModel();

  void RemoveRegion(int index);
  void AddRegion(const char* fileName, int position = -1); //load region (position==-1: add as new region)
  void RecalcRegion(int regionId);
  void SaveRegion(const char* fileName, int position, bool overwrite = false);
  void SetRegionFileLocation(const std::string fileName, int position);
  bool CheckFilenameConflict(const std::string& newPath, const size_t regionId, std::vector<std::string>& paths, std::vector<std::string>& fileNames, std::vector<size_t>& regionIds);
  void LoadTexturesSYN(FileReader& f, const std::shared_ptr<GlobalSimuState> globalState, int version);  // Load a textures(throws Error)


#endif
    bool   IsRunning();           // Started/Stopped state

  // Global simulation parameters
  std::shared_ptr<SimulationModel> model = //Worker constructs it, then passes to simManager
#ifdef MOLFLOW
      std::make_shared<MolflowSimulationModel>(); 
#else
      std::make_shared<SynradSimulationModel>();
#endif
  std::shared_ptr<ParticleLog> particleLog = std::make_shared<ParticleLog>(); //shared with simManager
  std::shared_ptr<GlobalSimuState> globalState = std::make_shared<GlobalSimuState>(); //shared with simManager

  FacetHistogramBuffer globalHistogramCache;

  Chronometer simuTimer;

  std::string fullFileName; // Current loaded file

  bool needsReload=true;   //When main and subprocess have different geometries, needs to reload (synchronize)
  bool abortRequested=false;

#if defined(MOLFLOW)
  std::vector<Parameter> interfaceParameterCache; //parameter cache for interface, file loading/saving and parameter editor, catalog parameters last
  int displayedMoment = 0; //By default, steady-state is displayed
  bool needsAngleMapStatusRefresh = false; //Interface helper: if IterfaceGeomToSimModel() constructs angle map, mark that facet adv. params update is necessary
  
  std::vector<Moment> interfaceMomentCache; //cache of model->tdParams.moments as frequently accessed in interface
  std::vector<UserMoment> userMoments;    //user-defined text values for defining time moments (can be time or time series)

  #endif

#if defined(SYNRAD)

	double no_scans;           // = nbDesorption/nbTrajPoints. Stored separately for saving/loading

	std::vector<Region_full> regions;
	std::vector<Material> materials;
	std::vector<std::vector<double>> psi_distro; //psi-energy map for full (par+ort) polarization
	std::vector<std::vector<double>> parallel_polarization; //ratio of parallel polarization for a given E/E_crit ratio and psi vertical angle
	std::vector<std::vector<std::vector<double>>> chi_distros; //3 psi-chi    maps for full/parallel/orthogonal polarizations
#endif

private:

  // Process management
  SimulationManager simManager;

  // Methods
  void ResetWorkerStats();
  void ResetConvergenceData();
  //void ClearHits();
  std::string GetErrorDetails();
  //void ThrowSubProcError(std::string message);
  void ThrowSubProcError(const char *message = nullptr);
  void Start();
  void Stop();
  void InnerStop();

  // Geometry handle
#if defined(MOLFLOW)
  MolflowGeometry* interfGeom;

#endif
#if defined(SYNRAD)
  SynradGeometry *interfGeom;
  Dataport *dpMat;
#endif

public:
    GlobalHitBuffer globalStatCache; //A cache is copied of global counters at every Worker::Update(), so that we don't have to lock the globalState mutex every time we use nbDes, etc.
};