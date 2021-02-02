//
// Created by Pascal Baehr on 04.05.20.
//

#include <cmath>
#include <sstream>
#include "SimulationController.h"
#include "../src/Simulation/Simulation.h"
#include "ProcessControl.h"
#include <Helper/OutputHelper.h>

#include <omp.h>

#if not defined(_MSC_VER)
#include <sys/time.h>
#include <sys/resource.h>
#endif



#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <process.h>
#endif

#define WAITTIME    500  // Answer in STOP mode

SimThread::SimThread(ProcComm *procInfo, SimulationUnit *sim, size_t threadNum) {
    this->threadNum = threadNum;
    this->procInfo = procInfo;
    this->status = nullptr;
    simDuration = 0.0;
    simulation = sim;
    stepsPerSec = 1.0;
    particle = nullptr;
    simEos = false;
    localDesLimit = 0;
}

SimThread::~SimThread() = default;


// todo: fuse with runSimulation()
int SimThread::advanceForSteps(size_t desorptions) {
    size_t nbStep = (stepsPerSec <= 0.0) ? 250 : std::ceil(stepsPerSec + 0.5);

    double timeStart = omp_get_wtime();
    double timeEnd = timeStart;
    do {
        simEos = !particle->SimulationMCStep(nbStep, threadNum, desorptions);
        timeEnd = omp_get_wtime();

        const double elapsedTimeMs = (timeEnd - timeStart); //std::chrono::duration<float, std::ratio<1, 1>>(end_time - start_time).count();
        if (elapsedTimeMs > 1e-6)
            stepsPerSec = (10.0 * nbStep) / (elapsedTimeMs); // every 1.0 second
        else
            stepsPerSec = (100.0 * nbStep); // in case of fast initial run

        desorptions -= particle->tmpState.globalHits.globalHits.hit.nbDesorbed;
    } while (desorptions);


    return 0;
}

// run until end or until autosaveTime check
int SimThread::advanceForTime(double simDuration) {

    size_t nbStep = (stepsPerSec <= 0.0) ? 250 : std::ceil(stepsPerSec + 0.5);

    double timeStart = omp_get_wtime();
    double timeEnd = timeStart;
    do {
        simEos = !particle->SimulationMCStep(nbStep, threadNum, 0);
        timeEnd = omp_get_wtime();

        const double elapsedTimeMs = (timeEnd - timeStart); //std::chrono::duration<float, std::ratio<1, 1>>(end_time - start_time).count();
        if (elapsedTimeMs > 1e-6)
            stepsPerSec = (10.0 * nbStep) / (elapsedTimeMs); // every 1.0 second
        else
            stepsPerSec = (100.0 * nbStep); // in case of fast initial run

    } while (simDuration > timeEnd - timeStart);


    return 0;
}

bool SimThread::runLoop() {
    bool eos;
    bool lastUpdateOk = false;

    //printf("Lim[%zu] %lu --> %lu\n",threadNum, localDesLimit, simulation->globState->globalHits.globalHits.hit.nbDesorbed);

    double timeStart = omp_get_wtime();
    do {
        setSimState(getSimStatus());
        size_t desorptions = localDesLimit;//(localDesLimit > 0 && localDesLimit > particle->tmpState.globalHits.globalHits.hit.nbDesorbed) ? localDesLimit - particle->tmpState.globalHits.globalHits.hit.nbDesorbed : 0;
        //printf("Pre[%zu] %lu + %lu / %lu\n",threadNum, desorptions, particle->tmpState.globalHits.globalHits.hit.nbDesorbed, localDesLimit);
        simEos = runSimulation(desorptions);      // Run during 1 sec
        //printf("Pos[%zu][%d] %lu + %lu / %lu\n",threadNum, simEos, desorptions, particle->tmpState.globalHits.globalHits.hit.nbDesorbed, localDesLimit);

        double timeEnd = omp_get_wtime();

        if (procInfo->currentSubProc == threadNum || timeEnd-timeStart > 60) { // update after 60s of no update or when thread is called
            if(simulation->model.otfParams.desorptionLimit > 0){
                if(localDesLimit > particle->tmpState.globalHits.globalHits.hit.nbDesorbed)
                    localDesLimit -= particle->tmpState.globalHits.globalHits.hit.nbDesorbed;
                else localDesLimit = 0;
            }

            size_t timeOut = lastUpdateOk ? 0 : 100; //ms
            lastUpdateOk = particle->UpdateHits(simulation->globState,
                                                timeOut); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
            procInfo->NextSubProc();
            timeStart = omp_get_wtime();
        }
        else{
            lastUpdateOk = false;
        }
        //printf("[%zu] PUP: %lu , %lu , %lu\n",threadNum, desorptions,localDesLimit, particle->tmpState.globalHits.globalHits.hit.nbDesorbed);
        eos = simEos || (procInfo->masterCmd != COMMAND_START) || (procInfo->subProcInfo[threadNum].slaveState == PROCESS_ERROR);
    } while (!eos);

    if (!lastUpdateOk) {
        //printf("[%zu] Updating on finish!\n",threadNum);
        setSimState("Final update...");
        particle->UpdateHits(simulation->globState,
                             20000); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).)
    }
    return simEos;
}

void SimThread::setSimState(char *msg) const {
    snprintf(procInfo->subProcInfo[threadNum].statusString, 128, "%s", msg);
}

void SimThread::setSimState(const std::string& msg) const {
    snprintf(procInfo->subProcInfo[threadNum].statusString, 128, "%s", msg.c_str());
}

[[nodiscard]] char *SimThread::getSimStatus() const {
    static char ret[128];
    size_t count = particle->totalDesorbed + particle->tmpState.globalHits.globalHits.hit.nbDesorbed;;

    size_t max = 0;
    if (simulation->model.otfParams.nbProcess)
        max = (simulation->model.otfParams.desorptionLimit / simulation->model.otfParams.nbProcess)
                + ((this->threadNum < simulation->model.otfParams.desorptionLimit % simulation->model.otfParams.nbProcess) ? 1 : 0);

    if (max != 0) {
        double percent = (double) (count) * 100.0 / (double) (max);
        sprintf(ret, "(%s) MC %zd/%zd (%.1f%%)", simulation->model.sh.name.c_str(), count, max, percent);
    } else {
        sprintf(ret, "(%s) MC %zd", simulation->model.sh.name.c_str(), count);
    }
    return ret;
}

int SimThread::runSimulation(size_t desorptions) {
    // 1s step
    size_t nbStep = (stepsPerSec <= 0.0) ? 250 : std::ceil(stepsPerSec + 0.5);

    {
        char msg[128];
        snprintf(msg, 128, "%s [%zu event/s]", getSimStatus(), nbStep);
        setSimState(msg);
    }

    // Check end of simulation
    bool goOn = true;
    size_t remainingDes = 0;

    if (particle->model->otfParams.desorptionLimit > 0) {
        if (desorptions <= particle->tmpState.globalHits.globalHits.hit.nbDesorbed){
            //lastHitFacet = nullptr; // reset full particle status or go on from where we left
            goOn = false;
        }
        else {
            //if(particle->tmpState.globalHits.globalHits.hit.nbDesorbed <= (particle->model->otfParams.desorptionLimit - simulation->globState->globalHits.globalHits.hit.nbDesorbed)/ particle->model->otfParams.nbProcess)
                remainingDes = desorptions - particle->tmpState.globalHits.globalHits.hit.nbDesorbed;
        }
    }
    //auto start_time = std::chrono::high_resolution_clock::now();
    if(goOn) {
        double start_time = omp_get_wtime();
        goOn = particle->SimulationMCStep(nbStep, threadNum, remainingDes);
        double end_time = omp_get_wtime();

    //auto end_time = std::chrono::high_resolution_clock::now();


        if (goOn) // don't update on end, this will give a false ratio (SimMCStep could return actual steps instead of plain "false"
        {
            const double elapsedTimeMs = (end_time - start_time); //std::chrono::duration<float, std::ratio<1, 1>>(end_time - start_time).count();
            if (elapsedTimeMs != 0.0)
                stepsPerSec = (1.0 * nbStep) / (elapsedTimeMs); // every 1.0 second
            else
                stepsPerSec = (100.0 * nbStep); // in case of fast initial run
        }
    }
#if defined(_DEBUG)
    //printf("Running: stepPerSec = %lf [%lu]\n", stepsPerSec, threadNum);
#endif

    return !goOn;
}

SimulationController::SimulationController(size_t parentPID, size_t procIdx, size_t nbThreads,
                                           std::vector<SimulationUnit *> *simulationInstance, ProcComm *pInfo) {
    this->prIdx = procIdx;
    this->parentPID = parentPID;
    if (nbThreads == 0)
        this->nbThreads = omp_get_max_threads();
    else
        this->nbThreads = nbThreads;

    simulation = simulationInstance; // TODO: Find a nicer way to manager derived simulationunit for Molflow and Synrad
    procInfo = pInfo; // Set from outside to link with general structure

    loadOK = false;

    // resetControls()
    stepsPerSec = 0.0;
    endState = false;
    lastHitUpdateOK = true;

    SetRuntimeInfo();

    //DEBUG_PRINT("Controller created (%zd bytes)\n", sizeof(SHCONTROL));
    SetReady(false);
}

SimulationController::~SimulationController() = default;

SimulationController::SimulationController(SimulationController &&o) noexcept {
    simulation = o.simulation;
    procInfo = o.procInfo;

    o.simulation = nullptr;
    o.procInfo = nullptr;

    stepsPerSec = o.stepsPerSec;
    endState = o.endState;
    lastHitUpdateOK = o.lastHitUpdateOK;

    prIdx = o.prIdx;
    parentPID = o.parentPID;
    nbThreads = o.nbThreads;
    loadOK = o.loadOK;

    simThreads.reserve(nbThreads);
    for (size_t t = 0; t < nbThreads; t++) {
        simThreads.emplace_back(
                SimThread(procInfo, simulation->at(0), t));
        simThreads.back().particle = simulation->at(0)->GetParticle(t);
    }
}

int SimulationController::resetControls() {
    lastHitUpdateOK = true;
    endState = false;

    stepsPerSec = 0.0;

    return 0;
}

int SimulationController::StartSimulation() {
    try {
        for (auto &sim : *simulation)
            sim->SanityCheckGeom();
    }
    catch (std::runtime_error &e) {
        return 1;
    }

    SetState(PROCESS_RUN, GetSimuStatus());
    //if (!currentParticle.lastHitFacet) StartFromSource();
    //return (currentParticle.lastHitFacet != nullptr);

    return 0;
}

int SimulationController::RunSimulation() {
    abort();
}

int SimulationController::SetRuntimeInfo() {

    // Update runtime information
    //procInfo->procId = 0; // TODO: There is no more procId
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    //procInfo->procId = _getpid();
#else
    //procInfo->procId = ::getpid();
#endif //  WIN

    for (auto &pInfo : procInfo->subProcInfo)
        GetProcInfo(pInfo.procId, &pInfo.runtimeInfo);

    return 0;
}

int SimulationController::ClearCommand() {

    procInfo->masterCmd = COMMAND_NONE;
    procInfo->cmdParam = 0;
    procInfo->cmdParam2 = 0;
    SetState(PROCESS_READY,GetSimuStatus());

    return 0;
}

int SimulationController::SetState(size_t state, const char *status, bool changeState, bool changeStatus) {

    if (changeState) {
        DEBUG_PRINT("\n setstate %zd \n", state);
        //master->procInformation[prIdx].masterCmd = state;
        for (auto &pInfo : procInfo->subProcInfo) {
            pInfo.slaveState = state;
        }
    }
    if (changeStatus) {
        for (auto &pInfo : procInfo->subProcInfo) {
            strncpy(pInfo.statusString, status, 127);
            pInfo.statusString[127] = '\0';
        }
    }

    return 0;
}

int SimulationController::SetState(size_t state, const std::vector<std::string> &status, bool changeState, bool changeStatus) {

    if (changeState) {
        DEBUG_PRINT("\n setstate %zd \n", state);
        //master->procInformation[prIdx].masterCmd = state;
        for (auto &pInfo : procInfo->subProcInfo) {
            pInfo.slaveState = state;
        }
    }
    if (changeStatus) {
        if(procInfo->subProcInfo.size() != status.size()){
            for (auto &pInfo : procInfo->subProcInfo) {
                strncpy(pInfo.statusString, "invalid state", 127);
                pInfo.statusString[127] = '\0';
            }
        }
        else {
            size_t pInd = 0;
            for (auto &pInfo : procInfo->subProcInfo) {
                strncpy(pInfo.statusString, status[pInd++].c_str(), 127);
                pInfo.statusString[127] = '\0';
            }
        }
    }

    return 0;
}

std::vector<std::string> SimulationController::GetSimuStatus() {

    std::vector<std::string> ret_vec(nbThreads);
    if (simulation->empty()) {
        ret_vec.assign(nbThreads, "[NONE]");
    }
    else{
        size_t threadId = 0;
        auto* sim = simulation->at(0);
        for(size_t p = 0; p < nbThreads; ++p) {
            auto* particle = (simulation->at(0))->GetParticle(p);
            if(particle == nullptr) break;
            if(!particle->tmpState.initialized){
                ret_vec[threadId]= "[NONE]";
            }
            else {
                size_t count = 0;
                count = particle->totalDesorbed + particle->tmpState.globalHits.globalHits.hit.nbDesorbed;
                size_t max = 0;
                if (sim->model.otfParams.nbProcess)
                    max = sim->model.otfParams.desorptionLimit / sim->model.otfParams.nbProcess + ((threadId < sim->model.otfParams.desorptionLimit % sim->model.otfParams.nbProcess) ? 1 : 0);

                char tmp[128];
                if (max != 0) {
                    double percent = (double) (count) * 100.0 / (double) (max);
                    sprintf(tmp, "(%s) MC %zd/%zd (%.1f%%)", sim->model.sh.name.c_str(), count, max, percent);
                } else {
                    sprintf(tmp, "(%s) MC %zd", sim->model.sh.name.c_str(), count);
                }
                ret_vec[threadId] = tmp;
            }
            ++threadId;
        }
    }
    return ret_vec;
}

void SimulationController::SetErrorSub(const char *message) {
    printf("Error: %s\n", message);
    SetState(PROCESS_ERROR, message);
}

void SimulationController::SetStatus(char *status) {
    for (auto &pInfo : procInfo->subProcInfo) {
        strncpy(pInfo.statusString, status, 127);
        pInfo.statusString[127] = '\0';
    }
}

void SimulationController::SetReady(const bool loadOk) {

    if (loadOk)
        SetState(PROCESS_READY, this->GetSimuStatus());
    else
        SetState(PROCESS_READY, "(No geometry)");

    ClearCommand();
}

// Return Error code if subproc info is out of sync
size_t SimulationController::GetLocalState() const {
    size_t locState = procInfo->subProcInfo[0].slaveState;
    for (auto &pInfo : procInfo->subProcInfo) {
        if (locState != pInfo.slaveState)
            return PROCESS_ERROR;
    }
    return locState;
}

// Main loop
int SimulationController::controlledLoop(int argc, char **argv) {
    endState = false;
    bool loadOk = false;
    while (!endState) {
        bool eos = false;
        switch (procInfo->masterCmd) {

            case COMMAND_LOAD: {
                DEBUG_PRINT("[%d] COMMAND: LOAD (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                SetState(PROCESS_STARTING, "Loading simulation");

                simulation->at(0)->SetNParticle(nbThreads);
                if(1){//if(nbThreads != simThreads.size()) {
                    simThreads.clear();
                    simThreads.reserve(nbThreads);
                    for (size_t t = 0; t < nbThreads; t++) {
                        simThreads.emplace_back(
                                SimThread(procInfo, simulation->at(0), t));
                        simThreads.back().particle = simulation->at(0)->GetParticle(t);
                    }
                }

                loadOk = Load();
                if (loadOk) {
                    //desorptionLimit = procInfo->cmdParam2; // 0 for endless
                    SetRuntimeInfo();
                }
                SetReady(loadOk);

                break;
            }
            case COMMAND_UPDATEPARAMS: {
                SetState(PROCESS_WAIT, GetSimuStatus());
                DEBUG_PRINT("[%d] COMMAND: UPDATEPARAMS (%zd,%zd)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if (UpdateParams()) {
                    SetReady(loadOk);
                    //SetState(procInfo->cmdParam2, GetSimuStatus());
                } else {
                    SetState(PROCESS_ERROR, "Could not update parameters");
                }
                break;
            }
            case COMMAND_RELEASEDPLOG: {
                SetState(PROCESS_WAIT, GetSimuStatus());
                DEBUG_PRINT("[%d] COMMAND: RELEASEDPLOG (%zd,%zd)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                SetReady(loadOk);
                break;
            }
            case COMMAND_START: {
                // Check end of simulation

                if ((*simulation)[0]->model.otfParams.desorptionLimit > 0) {
                    if ((*simulation)[0]->totalDesorbed >=
                        (*simulation)[0]->model.otfParams.desorptionLimit /
                        (*simulation)[0]->model.otfParams.nbProcess) {
                        ClearCommand();
                        SetState(PROCESS_DONE, GetSimuStatus());
                    }
                }

                for(auto& thread : simThreads){
                    thread.particle->model= &(*simulation)[0]->model;
                }

                if (GetLocalState() != PROCESS_RUN) {
                    DEBUG_PRINT("[%d] COMMAND: START (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                    SetState(PROCESS_RUN, GetSimuStatus());
                }
                bool lastUpdateOk = true;
                if (loadOk) {
                    size_t updateThread = 0;

                    // Calculate remaining work
                    size_t desPerThread = 0;
                    size_t remainder = 0;
                    if(simulation->at(0)->model.otfParams.desorptionLimit > 0){
                        if(simulation->at(0)->model.otfParams.desorptionLimit > (simulation->at(0)->globState->globalHits.globalHits.hit.nbDesorbed)) {
                            size_t limitDes_global = simulation->at(0)->model.otfParams.desorptionLimit;
                            desPerThread = limitDes_global / nbThreads;
                            remainder = limitDes_global % nbThreads;
                        }
                    }
                    for(auto& thread : simThreads){
                        size_t localDes = desPerThread - thread.particle->totalDesorbed;
                        thread.localDesLimit = (thread.threadNum < remainder) ? localDes + 1 : localDes;
                    }

                    int simuEnd = 0; // bool atomic is not supported by MSVC OMP implementation
#pragma omp parallel num_threads(nbThreads) default(none) firstprivate(/*stepsPerSec,*/ lastUpdateOk, eos) shared(/*procInfo,*/ updateThread, simuEnd/*, simulation,simThreads*/)
                    {
                        eos = simThreads[omp_get_thread_num()].runLoop();

#pragma omp atomic
                        simuEnd |= eos;
                    }

                    if (simuEnd) {
                        if (GetLocalState() != PROCESS_ERROR) {
                            // Max desorption reached
                            ClearCommand();
                            SetState(PROCESS_DONE, GetSimuStatus());
                            DEBUG_PRINT("[%d] COMMAND: PROCESS_DONE (Max reached)\n", prIdx);
                        }
                    }
                    break;
                } else {
                    SetErrorSub("No geometry loaded");
                    ClearCommand();
                }
                break;
            }
            case COMMAND_PAUSE: {
                DEBUG_PRINT("[%d] COMMAND: PAUSE (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if (!lastHitUpdateOK) {
                    // Last update not successful, retry with a longer timeout
                    if ((GetLocalState() != PROCESS_ERROR)) {
                        SetState(PROCESS_STARTING, "Updating hits...", false, true);
                        //lastHitUpdateOK = (*simulation)[0]->UpdateHits(prIdx, 60000);
                        SetState(PROCESS_STARTING, GetSimuStatus(), false, true);
                    }
                }
                SetReady(loadOk);
                break;
            }
            case COMMAND_RESET: {
                DEBUG_PRINT("[%d] COMMAND: RESET (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                SetState(PROCESS_STARTING, "Resetting local cache...", false, true);
                resetControls();
                for (auto &sim : *simulation)
                    sim->ResetSimulation();
                SetReady(loadOk);
                break;
            }
            case COMMAND_EXIT: {
                DEBUG_PRINT("[%d] COMMAND: EXIT (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                endState = true;
                break;
            }
            case COMMAND_CLOSE: {
                DEBUG_PRINT("[%d] COMMAND: CLOSE (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                for (auto &sim : *simulation)
                    sim->ClearSimulation();
                loadOk = false;
                SetReady(loadOk);
                break;
            }
            case PROCESS_RUN: {
                eos = RunSimulation();      // Run during 1 sec
                if ((GetLocalState() != PROCESS_ERROR)) {
                    //lastHitUpdateOK = (*simulation)[0]->UpdateHits(prIdx,20); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
                }
                if (eos) {
                    if (GetLocalState() != PROCESS_ERROR) {
                        // Max desorption reached
                        ClearCommand();
                        SetState(PROCESS_DONE, GetSimuStatus());
                        DEBUG_PRINT("[%d] COMMAND: PROCESS_DONE (Max reached)\n", prIdx);
                    }
                }
                break;
            }
            default: {
                ProcessSleep(WAITTIME);
                break;
            }
        }
    }
    SetState(PROCESS_KILLED, "Process terminated peacefully");
    return 0;
}

bool SimulationController::Load() {
    SetState(PROCESS_STARTING, "Loading simulation");
    bool loadError = false;
    // Load geometry

    for (auto & sim : *simulation) {
        if (sim->model.vertices3.empty()) {
            char err[512];
            sprintf(err, "Loaded empty 'geometry' (%zd Bytes)", procInfo->cmdParam);
            SetErrorSub(err);
            loadError = true;
            return loadError;
        }
    }

#pragma omp parallel for default(none) shared(loadError)
    for (int i = 0; i < simulation->size(); ++i) {
        if (simulation->at(i)->LoadSimulation(&simulation->at(0)->model, procInfo->subProcInfo[i].statusString)) {
#pragma omp critical
            loadError = true;
        }
    }
    return !loadError;
}

bool SimulationController::UpdateParams() {

    // Load geometry

    //bool result = simulation->UpdateOntheflySimuParams(loader);
    for (auto &sim : *simulation) {
        if (sim->model.otfParams.enableLogging) {
            printf("Logging with size limit %zd\n",
                   sizeof(size_t) + sim->model.otfParams.logLimit * sizeof(ParticleLoggerItem));
        }
        sim->ReinitializeParticleLog();
    }
    return true;
}