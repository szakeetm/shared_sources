#include <sstream>
#include <cmath> //std::ceil
#include "GLApp/GLTypes.h" //Error
#include "SimulationController.h"
#include "Helper/StringHelper.h"
#include "../../src/Simulation/Particle.h" //Synrad or Molflow

#ifdef MOLFLOW
#include "../src/Simulation/MolflowSimulation.h"
#endif

#ifdef SYNRAD
#include "../src/Simulation/SynradSimulation.h"
#endif

#include "ProcessControl.h"
#include <Helper/ConsoleLogger.h>
#include <Helper/OutputHelper.h>

#include <omp.h>

#if !defined(_MSC_VER)
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifdef _WIN32
#include <process.h>
#endif

#define WAITTIME 500 //controlled loop idle wait


SimThreadHandle::SimThreadHandle(ProcComm& procInfo, Simulation_Abstract *simPtr, size_t threadNum, size_t nbThreads) : masterProcInfo(procInfo) {
    this->threadNum = threadNum;
    this->nbThreads = nbThreads;
    this->simulationPtr = simPtr;
}

/*
// todo: fuse with RunSimulation1sec()
// Should allow simulation for N steps opposed to T seconds
int SimThreadHandle::advanceForSteps(size_t desorptions) {
    size_t nbStep = (stepsPerSec <= 0.0) ? 250 : (size_t)std::ceil(stepsPerSec + 0.5);

    double timeStart = omp_get_wtime();
    double timeEnd = timeStart;
    do {
        desLimitReachedOrDesError = particleTracerPtr->SimulationMCStep(nbStep, threadNum, desorptions);
        timeEnd = omp_get_wtime();

        const double elapsedTimeMs = (timeEnd - timeStart); //std::chrono::duration<float, std::ratio<1, 1>>(end_time - start_time).count();
        if (elapsedTimeMs > 1e-6)
            stepsPerSec = (10.0 * nbStep) / (elapsedTimeMs); // every 1.0 second
        else
            stepsPerSec = (100.0 * nbStep); // in case of fast initial run

        desorptions -= particleTracerPtr->tmpState->globalStats.globalHits.nbDesorbed;
    } while (desorptions);


    return 0;
}
*/

/*
// run until end or until autosaveTime check
int SimThreadHandle::advanceForTime(double simDuration) {

    size_t nbStep = (stepsPerSec <= 0.0) ? 250 : (size_t)std::ceil(stepsPerSec + 0.5);

    double timeStart = omp_get_wtime();
    double timeEnd = timeStart;
    do {
        desLimitReachedOrDesError = particleTracerPtr->SimulationMCStep(nbStep, threadNum, 0);
        timeEnd = omp_get_wtime();

        const double elapsedTimeMs = (timeEnd - timeStart); //std::chrono::duration<float, std::ratio<1, 1>>(end_time - start_time).count();
        if (elapsedTimeMs > 1e-6)
            stepsPerSec = (10.0 * nbStep) / (elapsedTimeMs); // every 1.0 second
        else
            stepsPerSec = (100.0 * nbStep); // in case of fast initial run

    } while (simDuration > timeEnd - timeStart);


    return 0;
}
*/

void SimThreadHandle::MarkIdle(){
    SetMyState(ThreadState::Idle);
}

/**
* \brief Simulation loop for an individual thread
 * \return true when simulation end has been reached via desorption limit, false otherwise
 */

LoopResult SimThreadHandle::RunLoop() {
    bool finishLoop = false;
    bool lastUpdateOk = false;
    LoopResult loopResult = LoopResult::Continue;
    double timeStart = omp_get_wtime();
    double timeLoopStart = timeStart;
    double timeEnd;

    SetMyState(ThreadState::Running);
    do {
        SetMyStatus(ConstructMyThreadStatus());
        RunResult runResult = RunSimulation1sec(localDesLimit); // Run for 1 sec
        
        timeEnd = omp_get_wtime();

        bool forceQueue = timeEnd-timeLoopStart > 60 || threadNum == 0; // update after 60s of no update or when thread 0 is called
        if (masterProcInfo.hitUpdateQueue.front() == threadNum || forceQueue) {
            size_t readdOnFail = 0;
            if(simulationPtr->model->otfParams.desorptionLimit > 0){
                if(localDesLimit > particleTracerPtr->tmpState->globalStats.globalHits.nbDesorbed) {
                    localDesLimit -= particleTracerPtr->tmpState->globalStats.globalHits.nbDesorbed;
                    readdOnFail = particleTracerPtr->tmpState->globalStats.globalHits.nbDesorbed;
                }
                else localDesLimit = 0;
            }

            size_t timeOut_ms = lastUpdateOk ? 0 : 100; //ms
            lastUpdateOk = particleTracerPtr->UpdateHitsAndLog(simulationPtr->globalState, simulationPtr->globParticleLog,
                masterProcInfo.threadInfos[threadNum].threadState, masterProcInfo.threadInfos[threadNum].threadStatus, masterProcInfo.procDataMutex, timeOut_ms); // Update hit with 100ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
            
            //set back from HitUpdate state
            SetMyState(ThreadState::Running); 
            SetMyStatus(ConstructMyThreadStatus());

            if(!lastUpdateOk) // if update failed, the desorption limit is invalid and has to be reverted
                localDesLimit += readdOnFail;

            if(masterProcInfo.hitUpdateQueue.front() == threadNum)
                masterProcInfo.PlaceFrontToBack();
            timeLoopStart = omp_get_wtime();
        }
        else{
            lastUpdateOk = false;
        }

        if (runResult == RunResult::DesError) {
            loopResult = LoopResult::DesorptionError;
        }
        else if (runResult == RunResult::MaxReached) {
            loopResult = LoopResult::DesLimitReached;
        }
        else if (this->particleTracerPtr->model->otfParams.timeLimit != 0 && ((timeEnd - timeStart) >= this->particleTracerPtr->model->otfParams.timeLimit)) {
            loopResult = LoopResult::TimeLimitReached;
        }
        else if (masterProcInfo.masterCmd != SimCommand::Run) {
            loopResult = LoopResult::AbortCommand;
        }
        else if (masterProcInfo.threadInfos[threadNum].threadState == ThreadState::ThreadError) {
            loopResult = LoopResult::HasThreadError;
        }
    } while (loopResult == LoopResult::Continue);

    masterProcInfo.RemoveFromHitUpdateQueue(threadNum);
    if (!lastUpdateOk) {
        particleTracerPtr->UpdateHitsAndLog(simulationPtr->globalState, simulationPtr->globParticleLog,
            masterProcInfo.threadInfos[threadNum].threadState, masterProcInfo.threadInfos[threadNum].threadStatus, masterProcInfo.procDataMutex, 20000); // Update hit with 20s timeout
    }
    SetMyStatus(ConstructMyThreadStatus());
    if (loopResult == LoopResult::DesLimitReached) {
        SetMyState(ThreadState::LimitReached);
    }
    else {
        SetMyState(ThreadState::Idle);
    }
    return loopResult;
}

void SimThreadHandle::SetMyStatus(const std::string& msg) const { //Writes to master's procInfo
    masterProcInfo.procDataMutex.lock();
    masterProcInfo.threadInfos[threadNum].threadStatus=msg;
    masterProcInfo.procDataMutex.unlock();
}
void SimThreadHandle::SetMyState(const ThreadState state) const { //Writes to master's procInfo
    masterProcInfo.procDataMutex.lock();
    masterProcInfo.threadInfos[threadNum].threadState=state;
    masterProcInfo.procDataMutex.unlock();
}

[[nodiscard]] std::string SimThreadHandle::ConstructMyThreadStatus() const {
    if (!particleTracerPtr) return "[No particle tracer constructed.]";

    size_t count = particleTracerPtr->totalDesorbed + particleTracerPtr->tmpState->globalStats.globalHits.nbDesorbed;

    size_t max = 0;
    if (nbThreads)
        max = (simulationPtr->model->otfParams.desorptionLimit / nbThreads)
                + ((this->threadNum < simulationPtr->model->otfParams.desorptionLimit % nbThreads) ? 1 : 0);

    if (max != 0) {
        double percent = (double) (count) * 100.0 / (double) (max);
        return fmt::format("{}/{} des ({:.1f}%)",count, max, percent);
    } else {
        return fmt::format("{} des", count);
    }
}

/**
* \brief A "single (1sec)" MC step of a simulation run for a given thread
 * \return false when simulation continues, true when desorption limit is reached or des error
 */
RunResult SimThreadHandle::RunSimulation1sec(const size_t desorptionLimit) {
    // 1s step
    size_t nbStep = (stepsPerSec <= 0.0) ? 250 : (size_t)std::ceil(stepsPerSec + 0.5);

    SetMyStatus(fmt::format("{} [{} hits/s]",ConstructMyThreadStatus(), nbStep));

    // Check end of simulation
    bool limitReachedOrDesError = false;
    size_t remainingDes = 0;

    if (particleTracerPtr->model->otfParams.desorptionLimit > 0) {
        if (desorptionLimit <= particleTracerPtr->tmpState->globalStats.globalHits.nbDesorbed){
            return RunResult::MaxReached; //there is a des. limit and it was reached
        }
        else {
            //there is a des. limit and remainingDes is left
            remainingDes = desorptionLimit - particleTracerPtr->tmpState->globalStats.globalHits.nbDesorbed;
        }
    }

    
    double start_time = omp_get_wtime();
    MFSim::MCStepResult runResult = particleTracerPtr->SimulationMCStep(nbStep, threadNum, remainingDes); //run 1 sec
    if (runResult==MFSim::MCStepResult::MaxReached) {
        return RunResult::MaxReached;
    }
    else if (runResult == MFSim::MCStepResult::DesorptionError) {
        return RunResult::DesError;
    }
    double end_time = omp_get_wtime();

    // don't update on end, this will give a false ratio (SimMCStep could return actual steps instead of plain "false"
    
    const double elapsedTimeMs = (end_time - start_time);
    if (elapsedTimeMs != 0.0) {
        stepsPerSec = static_cast<double>(nbStep) / elapsedTimeMs; // every 1.0 second
    }
    else {
        stepsPerSec = (100.0 * static_cast<double>(nbStep)); // in case of fast initial run
    }    
    return RunResult::Success; //continue
}

SimulationController::SimulationController(size_t parentPID, size_t procIdx, size_t nbThreads,
                                           Simulation_Abstract *simulationInstance, ProcComm& pInfo) : procInfo(pInfo) {
    this->prIdx = procIdx;
    this->parentPID = parentPID;
    if (nbThreads == 0)
#if defined(DEBUG)
        this->nbThreads = 1;
#else
        this->nbThreads = omp_get_max_threads();
#endif
    else
        this->nbThreads = nbThreads;

    simulationPtr = simulationInstance; // TODO: Find a nicer way to manager derived simulationunit for Molflow and Synrad

    SetRuntimeInfo();
    //procInfo.controllerState = ControllerState::Ready;
}

//SimulationController::~SimulationController() = default;

/*
SimulationController::SimulationController(SimulationController &&o) noexcept {
    simulationPtr = o.simulationPtr;
    procInfoPtr = o.procInfoPtr;

    o.simulationPtr = nullptr;
    o.procInfoPtr = nullptr;

    stepsPerSec = o.stepsPerSec;
    exitRequested = o.exitRequested;
    lastHitUpdateOK = o.lastHitUpdateOK;

    prIdx = o.prIdx;
    parentPID = o.parentPID;
    nbThreads = o.nbThreads;
    loadOk = o.loadOk;

    simThreadHandles.reserve(nbThreads);
    for (size_t t = 0; t < nbThreads; t++) {
        simThreadHandles.emplace_back(
                SimThreadHandle(procInfoPtr, simulationPtr, t));
        simThreadHandles.back().particleTracerPtr = simulationPtr->GetParticleTracerPtr(t);
    }
}
*/

void SimulationController::ResetControls() {
    lastHitUpdateOK = true;
    exitRequested = false;
    stepsPerSec = 0.0;
}

void SimulationController::SetRuntimeInfo() {

    // Update runtime information
    for (auto& pInfo : procInfo.threadInfos) {
        GetProcInfo(pInfo.threadId, &pInfo.runtimeInfo);
    }
}

void SimulationController::ClearCommand() {
    procInfo.procDataMutex.lock();
    procInfo.masterCmd = SimCommand::None;
    procInfo.cmdParam = 0;
    procInfo.cmdParam2 = 0;
    procInfo.procDataMutex.unlock();
}

/*
int SimulationController::SetThreadStates(SimState state, const std::string &status, bool changeState, bool changeStatus) {

    if (changeState) {
        for (auto &pInfo : procInfo.threadInfos) {
            pInfo.threadState = state;
        }
    }
    if (changeStatus) {
        for (auto &pInfo : procInfo.threadInfos) {
            pInfo.threadStatus=status;
        }
    }

    if(state == SimState::InError){
        procInfo.masterCmd = SimCommand::None;
    }
    return 0;
}
*/

/*
int SimulationController::SetThreadStates(SimState state, const std::vector<std::string> &status, bool changeState, bool changeStatus) {

    if (changeState) {
        //DEBUG_PRINT("setstate %s\n", threadStateStrings.at(state).c_str());
        for (auto &pInfo : procInfo.threadInfos) {
            pInfo.threadState = state;
        }
    }
    if (changeStatus) {
        if(procInfo.threadInfos.size() != status.size()){
            for (auto &pInfo : procInfo.threadInfos) {
               pInfo.threadStatus="invalid state (subprocess number mismatch)";
            }
        }
        else {
            size_t pInd = 0;
            for (auto &pInfo : procInfo.threadInfos) {
                pInfo.threadStatus=status[pInd++];
            }
        }
    }

    if(state == SimState::InError){
        procInfo.masterCmd = SimCommand::None;
    }
    return 0;
}
*/

/*
std::vector<std::string> SimulationController::GetThreadStatuses() {

    std::vector<std::string> tmpThreadStatuses(nbThreads);
    if (!simulationPtr) {
        tmpThreadStatuses.assign(nbThreads, "[No simulation loaded]");
    }
    else{
        for(size_t threadId = 0; threadId < nbThreads; ++threadId) {
            auto* particleTracer = simulationPtr->GetParticleTracerPtr(threadId);
            if (particleTracer == nullptr) {
                tmpThreadStatuses[threadId] = "[No particle tracer constructed]";
            }
            else if(!particleTracer->tmpState->initialized){
                tmpThreadStatuses[threadId]= "[Local counters not initialized]";
            }
            else {
                tmpThreadStatuses[threadId] = simThreadHandles[threadId].ConstructMyThreadStatus();
            }
        }
    }
    return tmpThreadStatuses;
}
*/

/*
void SimulationController::SetThreadError(const std::string& message) {
    Log::console_error("Error: {}\n", message);
    SetThreadStates(SimState::InError, message);
}
*/

/*
void SimulationController::SetStatus(const std::string& status) {
    for (auto &pInfo : procInfo.threadInfos) {
        pInfo.threadStatus=status;
    }
}
*/
/*
// Returns the thread state if equal for all, otherwise PROCESS_ERROR
size_t SimulationController::GetThreadStates() const {
    size_t locState = procInfo.threadInfos[0].threadState;
    for (auto &pInfo : procInfo.threadInfos) {
        if (locState != pInfo.threadState)
            return SimState::InError;
    }
    return locState; // All were equal to this
}
*/

// Main loop, listens to commands from Simulation Manager and executes blocking
void SimulationController::ControllerLoop() {
    procInfo.UpdateControllerStatus({ ControllerState::Ready }, std::nullopt);
    exitRequested = false;
    loadOk = false;
    while (!exitRequested) {
        switch (procInfo.masterCmd) {
            case SimCommand::Load: {
                try {
                    Load();
                }
                catch (Error& err) {
                    //nothing, as Load already set controller state to error
                }
                break;
            }
            case SimCommand::UpdateParams: {
                UpdateParams();
                break;
            }
            case SimCommand::Run: {
                StartAndRun();
                break;
            }
            case SimCommand::Pause: {
                //ClearCommand(); //threads will self-stop when command!=run and StartAndRun() will clear command
                procInfo.UpdateControllerStatus({ ControllerState::Ready }, std::nullopt);
                break;
            }
            case SimCommand::Reset: {
                Reset();
                break;
            }
            case SimCommand::Kill: {
                exitRequested = true;
                break;
            }
		    case SimCommand::MarkIdle: {
                MarkThreadsIdle();
			    break;
		    }
		    default: {
			    ProcessSleep(WAITTIME);
			    break;
		    }
		}
    }
    procInfo.UpdateControllerStatus({ ControllerState::Exit }, { "Exited on request" });
}

/**
* \brief Call a rebuild of the ADS
 * \return 0> error code, 0 when ok
 */
/*
int SimulationController::RebuildAccel(LoadStatus_abstract* loadStatus) {
    if (simulationPtr->RebuildAccelStructure()) {
        return 1;
    }
    return 0;
}
*/

/**
* \brief Load and init simulation geometry and initialize threads after previous sanity check
 * Setup inidividual particleTracers per thread and local desorption limits
 */
void SimulationController::Load(LoadStatus_abstract* loadStatus) {
    procInfo.UpdateControllerStatus({ ControllerState::Loading }, { "Checking if model is valid..." }, loadStatus);
    auto errors = simulationPtr->SanityCheckModel(false);
    procInfo.UpdateControllerStatus(std::nullopt, { "" }, loadStatus);

    if(errors.empty()) {
       // SetThreadStates(PROCESS_EXECUTING_COMMAND, "Loading simulation");
        bool loadError = false;

        // Init particleTracers / threads
        procInfo.UpdateControllerStatus(std::nullopt, { "Constructing particle tracers..." }, loadStatus);
        simulationPtr->ConstructParticleTracers(nbThreads, false);

        //master status set inside LoadSimulation() itself
        if (simulationPtr->LoadSimulation(procInfo,loadStatus)) {
            loadError = true;
        }

        if (loadError) {
            ClearCommand();
            return; //error
        }

        loadOk = true;

        simThreadHandles.clear();
        simThreadHandles.reserve(nbThreads);
        for (size_t t = 0; t < nbThreads; t++) {
            simThreadHandles.emplace_back(
                    SimThreadHandle(procInfo, simulationPtr, t, nbThreads));
            simThreadHandles.back().particleTracerPtr = simulationPtr->GetParticleTracerPtr(t);
        }
        
        // "Warm up" threads, to remove overhead for performance benchmarks
        double randomCounter = 0;
#pragma omp parallel default(none) shared(randomCounter)
        {
            double local_result = 0;
#pragma omp for
            for (int i=0; i < 1000; i++) {
                local_result += 1;
            }
#pragma omp critical
            randomCounter += local_result;
        }
        //DEBUG_PRINT("[OMP] Init: %f\n", randomCounter);
            
        // Calculate remaining work
        size_t desPerThread = 0;
        size_t remainder = 0;
        size_t des_global = simulationPtr->globalState->globalStats.globalHits.nbDesorbed;
        if (des_global > 0) {
            desPerThread = des_global / nbThreads;
            remainder = des_global % nbThreads;
        }
        for (auto &thread : simThreadHandles) {
            thread.particleTracerPtr->totalDesorbed = desPerThread;
            thread.particleTracerPtr->totalDesorbed += (thread.threadNum < remainder) ? 1 : 0;
        }

        SetRuntimeInfo();        
        procInfo.UpdateControllerStatus({ ControllerState::Ready }, { "" }, loadStatus);
    }
    else { //errors
        loadOk = false;
        procInfo.UpdateControllerStatus({ ControllerState::InError }, { fmt::format("Geometry error: {}", errors[0]) }, loadStatus);
    }
    ClearCommand();
}

/**
* \brief Update on the fly parameters when called from the GUI
 * \return true on success
 */
bool SimulationController::UpdateParams(LoadStatus_abstract* loadStatus) {
    // Load geometry
    auto* sim = simulationPtr;
    if (sim->model->otfParams.enableLogging) {
        Log::console_msg(3, "Logging with size limit {}\n",
               sizeof(size_t) + sim->model->otfParams.logLimit * sizeof(ParticleLoggerItem));
    }

    procInfo.UpdateControllerStatus({ ControllerState::ParamUpdating }, { "Reinitializing particle log..." }, loadStatus);
    sim->ReinitializeParticleLog();
    procInfo.UpdateControllerStatus({ ControllerState::Ready }, { "" }, loadStatus);
    ClearCommand();
    return true;
}

/**
* \brief Start the simulation after previous sanity checks
 * return true if started and ran, false if can't find starting point (des. error)
 */
bool SimulationController::StartAndRun(LoadStatus_abstract* loadStatus) {

	if (simulationPtr->model->rayTracingStructures.empty()) {
		loadOk = false;
        procInfo.UpdateControllerStatus({ ControllerState::InError }, { "Failed building acceleration structure" }, loadStatus);
        ClearCommand();
        return false;
	}

    /*
    if (simulationPtr->model->otfParams.desorptionLimit > 0) {
        if (simulationPtr->totalDesorbed >=
            simulationPtr->model->otfParams.desorptionLimit /
            nbThreads) { //des. limit reached
            ClearCommand();
            procInfo.UpdateControllerStatus({ ControllerState::Ready }, { "Des. limit reached" }, loadStatus);
            return false;
        }
    }
    */

    procInfo.InitHitUpdateQueue();

    // Calculate remaining work
    size_t desPerThread = 0;
    size_t remainder = 0;
    if(simulationPtr->model->otfParams.desorptionLimit > 0){
        if(simulationPtr->model->otfParams.desorptionLimit > (simulationPtr->globalState->globalStats.globalHits.nbDesorbed)) {
            size_t limitDes_global = simulationPtr->model->otfParams.desorptionLimit;
            desPerThread = limitDes_global / nbThreads;
            remainder = limitDes_global % nbThreads;
        }
    }
    for(auto& thread : simThreadHandles){
        size_t localDes = (desPerThread > thread.particleTracerPtr->totalDesorbed) ? desPerThread - thread.particleTracerPtr->totalDesorbed : 0; //remaining
        thread.localDesLimit = (thread.threadNum < remainder) ? localDes + 1 : localDes;
    }

    bool desError_global = false;

#pragma omp parallel num_threads((int)nbThreads)
    {
        bool desError_private = false;

        // Set OpenMP thread priority on Windows whenever we start a simulation run
#if defined(_WIN32) && defined(_MSC_VER)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
#endif
        //Actually run the simulation:
        desError_private = simThreadHandles[omp_get_thread_num()].RunLoop() == LoopResult::DesorptionError;

        if(desError_private) {
//#pragma omp atomic
            desError_global = true; //Race condition ok, desError_global means "at least one thread finished"
        }
    }

    //Run finished
    if (procInfo.masterCmd != SimCommand::Kill) {
        //command is 'Run' if exited because des. reached
        //otherwise 'Pause'
        //exact reason is RunLoop() return value
        ClearCommand();
    }
    procInfo.UpdateControllerStatus({ ControllerState::Ready }, std::nullopt, loadStatus);
    return !desError_global;
}

void SimulationController::Reset(LoadStatus_abstract* loadStatus) {
    procInfo.UpdateControllerStatus({ ControllerState::Resetting }, { "Resetting simulation..." }, loadStatus);
    ResetControls();
    simulationPtr->ResetSimulation();
    procInfo.UpdateControllerStatus({ ControllerState::Ready }, { "" }, loadStatus);
    ClearCommand();
}

void SimulationController::EmergencyExit(){
    for (auto& thread : simThreadHandles) {
        thread.particleTracerPtr->exitRequested = true;
    }
};

void SimulationController::MarkThreadsIdle(LoadStatus_abstract* loadStatus) {
    procInfo.UpdateControllerStatus({ ControllerState::Initializing }, { "Marking threads idle..." }, loadStatus);
    
    for (auto& thread : simThreadHandles) {
        thread.MarkIdle();
    }

    procInfo.UpdateControllerStatus({ ControllerState::Ready }, { "" }, loadStatus);
    ClearCommand();
}