//
// Created by pbahr on 15/04/2020.
//


#include <thread>
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <process.h>
#elif not(defined(__MACOSX__) || defined(__APPLE__))
#include <cstring> //memset on unix
#endif

#include "SimulationManager.h"
//#include "Buffer_shared.h" // TODO: Move SHCONTROL to seperate file or SMP.h
#include "SMP.h"
#include "ProcessControl.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cereal/archives/binary.hpp>
#include <../src/Simulation.h>

SimulationManager::SimulationManager(std::string appName , std::string dpName) {
    isRunning = false;
    useCPU = false;
    nbCores = 0;

    useGPU = false;

    useRemote = false;

    dpHit = nullptr;
    dpLog = nullptr;

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    uint32_t pid = _getpid();
    const char* dpPrefix = "";
#else
    uint32_t pid = ::getpid();
    const char *dpPrefix = "/"; // creates semaphore as /dev/sem/%s_sema
#endif

    sprintf(this->appName,"%s", appName.c_str());
    sprintf(this->ctrlDpName,"%s", std::string(dpPrefix+dpName+"CTRL"+std::to_string(pid)).c_str());
    sprintf(this->loadDpName,"%s", std::string(dpPrefix+dpName+"LOAD"+std::to_string(pid)).c_str());
    sprintf(this->hitsDpName,"%s", std::string(dpPrefix+dpName+"HITS"+std::to_string(pid)).c_str());
    sprintf(this->logDpName,"%s", std::string(dpPrefix+dpName+"LOG"+std::to_string(pid)).c_str());

    allProcsDone = false;
}

SimulationManager::~SimulationManager() {
    CLOSEDP(dpHit);
    CLOSEDP(dpLog);
}

int SimulationManager::refreshProcStatus() {
    int nbDead = 0;
    for(auto proc = simHandles.begin(); proc != simHandles.end() ; ){
        if(!(*proc).first.joinable()){
            auto myHandle = (*proc).first.native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
            TerminateThread(myHandle, 1);
#else
            //Linux
            pthread_cancel(myHandle);
#endif
            proc = this->simHandles.erase(proc);
            ++nbDead;
        }
        else{
            ++proc;
        }
    }
    return nbDead;
}

int SimulationManager::LoadInput(const std::string& fileName) {
    std::ifstream inputFile(fileName);
    inputFile.seekg(0, std::ios::end);
    size_t size = inputFile.tellg();
    std::string buffer(size, ' ');
    inputFile.seekg(0);
    inputFile.read(&buffer[0], size);

    try {
        ShareWithSimUnits((BYTE *) buffer.c_str(), buffer.size(), LoadType::LOADGEOM);
    }
    catch (std::runtime_error& e) {
        throw e;
    }
    return 0;
}

int SimulationManager::ResetStatsAndHits() {

    return 0;
}

int SimulationManager::ReloadLogBuffer(size_t logSize, bool ignoreSubs) {//Send simulation mode changes to subprocesses without reloading the whole geometry
    if (simHandles.empty())
        throw std::logic_error("No active simulation handles!");

    if(!ignoreSubs) {
        //if (ExecuteAndWait(COMMAND_RELEASEDPLOG, isRunning ? PROCESS_RUN : PROCESS_READY,isRunning ? PROCESS_RUN : PROCESS_READY)) {
        if (ExecuteAndWait(COMMAND_RELEASEDPLOG, PROCESS_READY,isRunning ? PROCESS_RUN : PROCESS_READY)) {
            throw std::runtime_error(MakeSubProcError("Subprocesses didn't release dpLog handle"));
        }
    }
    //To do: only close if parameters changed
    if (dpLog && logSize == dpLog->size) {
        ClearLogBuffer(); //Fills values with 0
    } else {
        CloseLogDP();
        if (logSize)
            if(CreateLogDP(logSize)) {
                throw std::runtime_error(
                        "Failed to create 'dpLog' dataport.\nMost probably out of memory.\nReduce number of logged particles in Particle Logger.");
            }
    }

    return 0;
}

int SimulationManager::ReloadHitBuffer(size_t hitSize) {
    if(dpHit && hitSize == dpHit->size){
        // Just reset the buffer
        ResetHits();
    }
    else{
        CloseHitsDP();
        if (CreateHitsDP(hitSize)) {
            throw std::runtime_error("Failed to create 'hits' dataport: out of memory.");
        }
    }

    return 0;
}

/*!
 * @brief Starts the simulation on all available simulation units
 * @return 0=start successful, 1=PROCESS_DONE state entered
 */
int SimulationManager::StartSimulation() {
    refreshProcStatus();
    if (simHandles.empty())
        throw std::logic_error("No active simulation handles!");

    if (ExecuteAndWait(COMMAND_START, PROCESS_RUN, 0, 0)){ // TODO: 0=MC_MODE, AC_MODE should be seperated completely
        throw std::runtime_error(MakeSubProcError("Subprocesses could not start the simulation"));
    }
    isRunning = true;
    if(allProcsDone){
        return 1;
    }
    return 0;
}

int SimulationManager::StopSimulation() {
    refreshProcStatus();
    if (simHandles.empty())
        return 1;

    if (ExecuteAndWait(COMMAND_PAUSE, PROCESS_READY, 0, 0))
        throw std::runtime_error(MakeSubProcError("Subprocesses could not stop the simulation"));
    isRunning = false;
    return 0;
}

/*!
 * @brief Convenience function that stops a running simulation and starts a paused simulation
 * @return 0=success, 1=else
 * @todo add benchmark
 */
bool SimulationManager::StartStopSimulation(){
    if (isRunning) {

        // Stop
        //InnerStop(appTime);
        try {
            StopSimulation();
            //Update(appTime);
        }

        catch (std::runtime_error &e) {
            throw e;
        }
    } else {

        // Start
        try {
            //if (needsReload) RealReload(); //Synchronize subprocesses to main process

            StartSimulation();
        }
        catch (std::runtime_error &e) {
            throw e;
        }

        // Particular case when simulation ends before getting RUN state
        if (allProcsDone) {
            //Update(appTime);
            //GLMessageBox::Display("Max desorption reached", "Information (Start)", GLDLG_OK, GLDLG_ICONINFO);
        }

    }
    return isRunning; // return previous state
}


int SimulationManager::TerminateSimHandles() {

    return 0;
}

int SimulationManager::CreateCPUHandle(uint16_t iProc) {
    uint32_t processId;

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    processId = _getpid();
#else
    processId = ::getpid();
#endif //  WIN

    simUnits.emplace_back(Simulation{nbThreads});
    procInformation.emplace_back(SubProcInfo{});
    simController.emplace_back(SimulationController{"molflow", processId, iProc, &simUnits.back(), &procInformation.back()});
    simHandles.emplace_back(
            /*StartProc(arguments, STARTPROC_NOWIN),*/
            std::thread(&SimulationController::controlledLoop,&simController.back(),NULL,nullptr),
            SimType::simCPU);
    auto myHandle = simHandles.back().first.native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
    SetThreadPriority(myHandle, THREAD_PRIORITY_NORMAL);
#else
    int policy;
    struct sched_param param{};

    pthread_getschedparam(myHandle, &policy, &param);
    param.sched_priority = sched_get_priority_max(policy);
    pthread_setschedparam(myHandle, policy, &param);
    //Check! Some documentation says it's always 0
#endif

    return 0;
}

// return 1=error
int SimulationManager::CreateGPUHandle() {
    return 1;
}

// return 1=error
int SimulationManager::CreateRemoteHandle() {
    return 1;
}

//TODO: This is Molflow only
int SimulationManager::CreateLogDP(size_t logDpSize) {
    //size_t logDpSize = sizeof(size_t) + ontheflyParams.logLimit * sizeof(ParticleLoggerItem);
    dpLog = CreateDataport(logDpName, logDpSize);
    if (!dpLog) {
        //progressDlg->SetVisible(false);
        //SAFE_DELETE(progressDlg);
        return 1;
        //throw Error("Failed to create 'dpLog' dataport.\nMost probably out of memory.\nReduce number of logged particles in Particle Logger.");
    }

    return 0;
}

int SimulationManager::CreateHitsDP(size_t hitSize) {
    //size_t hitSize = geom->GetHitsSize(&moments);
    dpHit = CreateDataport(hitsDpName, hitSize);
    //ClearHits(true);
    if (!dpHit) {
        return 1;
        //throw Error("Failed to create 'hits' dataport: out of memory.");
    }

    // ClearHits
    ResetHits(); //Also clears hits, leaks
    return 0;
}

/*!
 * @brief Creates Simulation Units and waits for their ready status
 * @return 0=all SimUnits are ready, else = ret Units are active, but not all could be launched
 */
int SimulationManager::InitSimUnits() {

    if(useCPU){
        // Launch nbCores subprocesses
        auto nbActiveProcesses = simHandles.size();
        for(int iProc = 0; iProc < nbCores; ++iProc) {
            if(CreateCPUHandle(iProc + nbActiveProcesses)) // abort initialization when creation fails
                return simHandles.size();
            //procInformation.push_back(simController.back().procInfo);
        }
    }
    if(useGPU){
        CreateGPUHandle();
        //procInformation.push_back(simController.back().procInfo);
    }
    if(useRemote){
        CreateRemoteHandle();
        //procInformation.push_back(simController.back().procInfo);
    }

    return WaitForProcStatus(PROCESS_READY);
}

/*!
 * @brief Wait until all SimulationUnits are in procStatus or reach another endstate (error, done)
 * @param procStatus Process Status that should be waited for
 * @return 0 if wait is successful
 */
int SimulationManager::WaitForProcStatus(const uint8_t procStatus) {
    // Wait for completion
    bool finished = false;
    bool error = false;
    int waitTime = 0;
    int timeOutAt = 10000; // 10 sec
    allProcsDone = true;

    do {

        finished = true;

        for (size_t i = 0; i < simHandles.size(); i++) {
            auto procState = procInformation[i].slaveState;
            finished = finished & (procState==procStatus || procState==PROCESS_ERROR || procState==PROCESS_DONE);
            if( procState==PROCESS_ERROR ) {
                error = true;
            }
            else if(procState == PROCESS_STARTING){
                timeOutAt = 20000; // if task properly started, increase allowed wait time
            }
            allProcsDone = allProcsDone & (procState == PROCESS_DONE);
        }

        if (!finished) {
            ProcessSleep(250);
            waitTime += 250;
        }
    } while (!finished && waitTime<timeOutAt);

    return waitTime>=timeOutAt || error; // 0 = finished, 1 = timeout
}

int SimulationManager::ForwardCommand(const int command, const size_t param, const size_t param2) {
    // Send command

    for(size_t i=0;i<simHandles.size();i++) {
        auto procState = procInformation[i].slaveState;
        if(procState == PROCESS_READY
           || procState == PROCESS_RUN
              || procState==PROCESS_ERROR || procState==PROCESS_DONE) { // check if it'' ready before sending a new command
            procInformation[i].slaveState = PROCESS_STARTING;
            procInformation[i].oldState = procInformation[i].masterCmd; // use to solve old state
            procInformation[i].masterCmd = command;
            procInformation[i].cmdParam = param;
            procInformation[i].cmdParam2 = param2;
        }
    }

    return 0;
}

/*!
 * @brief Shortcut function combining ForwardCommand() and WaitForProcStatus() into a single call
 * @param command execution command for every subprocess
 * @param procStatus status that every subprocess has to reach
 * @param param additional command parameter
 * @return 0=success, 1=fail
 */
int SimulationManager::ExecuteAndWait(const int command, const uint8_t procStatus, const size_t param,
                                      const size_t param2) {
    if(!ForwardCommand(command, param, param2)) { // execute
        if (!WaitForProcStatus(procStatus)) { // and wait
            return 0;
        }
    }
    return 1;
}

int SimulationManager::KillAllSimUnits() {
    if( !simHandles.empty() ) {
        if(ExecuteAndWait(COMMAND_EXIT, PROCESS_KILLED)){ // execute
            // Force kill

            for(size_t i=0;i<simHandles.size();i++) {
                if (procInformation[i].slaveState != PROCESS_KILLED){
                    auto nativeHandle = simHandles[i].first.native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
                    //Windows
				    TerminateThread(nativeHandle, 1);
#else
                    //Linux
                    pthread_cancel(nativeHandle);
#endif
                    //assume that the process doesn't exist, so remove it from our management structure
                    simHandles.erase((simHandles.begin()+i));
                    throw std::runtime_error(MakeSubProcError("Could not terminate sub processes")); // proc couldn't be killed!?

                }
            }
        }

        for(size_t i=0;i<simHandles.size();i++) {
            if (procInformation[i].slaveState == PROCESS_KILLED) {
                simHandles[i].first.join();
            }
            else{
                auto nativeHandle = simHandles[i].first.native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
                //Windows
                TerminateThread(nativeHandle, 1);
#else
                //Linux
                pthread_cancel(nativeHandle);
#endif
            }
        }
        simHandles.clear();
    }
    nbCores = 0;
    return 0;
}

int SimulationManager::ClearLogBuffer() {
    if (!dpLog) {
        return 1;
    }
    AccessDataport(dpLog);
    memset(dpLog->buff, 0, dpLog->size); //Also clears hits, leaks
    ReleaseDataport(dpLog);
    return 0;
}

int SimulationManager::ResetSimulations() {
    if (ExecuteAndWait(COMMAND_CLOSE, PROCESS_READY, 0, 0))
        throw std::runtime_error(MakeSubProcError("Subprocesses could not restart"));
    return 0;
}

int SimulationManager::ResetHits() {
    if (ExecuteAndWait(COMMAND_RESET, PROCESS_READY, 0, 0))
        throw std::runtime_error(MakeSubProcError("Subprocesses could not reset hits"));
    return 0;
}

int SimulationManager::CloseLogDP() {
    CLOSEDP(dpLog);
    return 0;
}

int SimulationManager::CloseHitsDP() {
    CLOSEDP(dpHit);
    return 0;
}

int SimulationManager::GetProcStatus(std::vector<SubProcInfo>& procInfoList) {
    if(simHandles.empty())
        return 1;
    
    for(int i = 0; i<simHandles.size();++i){
        procInfoList.emplace_back(SubProcInfo {procInformation[i]});
    }

    return 0;
}

int SimulationManager::GetProcStatus(size_t *states, std::vector<std::string>& statusStrings) {

    for (size_t i = 0; i < procInformation.size(); i++) {
        //states[i] = shMaster->procInformation[i].masterCmd;
        states[i] = procInformation[i].slaveState;
        char tmp[128];
        strncpy(tmp, procInformation[i].statusString, 127);
        tmp[127] = 0;
        statusStrings[i] = tmp;
    }
    return 0;
}

bool SimulationManager::GetLockedHitBuffer() {
    return true;
}

int SimulationManager::UnlockHitBuffer() {
    return 0;
}

int SimulationManager::UploadToHitBuffer(void *data, size_t size) {
    if (dpHit && AccessDataport(dpHit)) {
        std::copy((BYTE*)data,(BYTE*)data + size,(BYTE*)dpHit->buff);
        ReleaseDataport(dpHit);
        return 0;
    }

    return 1;
}

BYTE *SimulationManager::GetLockedLogBuffer() {
    if (dpLog && AccessDataport(dpLog)) {
        return (BYTE*)dpLog->buff;
    }
    return nullptr;
}

int SimulationManager::UnlockLogBuffer() {
    if (dpLog && ReleaseDataport(dpLog)) {
        return 0;
    }
    return 1;
}

/*!
 * @brief Return error information or current running state in case of a hangup
 * @return char array containing proc status (and error message/s)
 */
const char *SimulationManager::GetErrorDetails() {

    std::vector<SubProcInfo> procInfo;
    GetProcStatus(procInfo);

    static char err[1024];
    strcpy(err, "");

    for (size_t i = 0; i < procInfo.size(); i++) {
        char tmp[512];
        size_t state = procInfo[i].slaveState;
        if (state == PROCESS_ERROR) {
            sprintf(tmp, "[#%zd] Process [PID %zu] %s: %s\n", i, procInfo[i].procId, prStates[state],
                    procInfo[i].statusString);
            strncat(err, tmp, 512);
        } else {
            sprintf(tmp, "[#%zd] Process [PID %zu] %s\n", i, procInfo[i].procId, prStates[state]);
        }
    }
    return err;
}

std::string SimulationManager::MakeSubProcError(const char *message) {
    std::string errString;
    if (!message){
        errString.append("Bad response from sub process(es):\n");
        errString.append(GetErrorDetails());
    }
    else{
        errString.append(message);
        errString.append(":\n");
        errString.append(GetErrorDetails());
    }
    return errString;
}


int SimulationManager::ShareWithSimUnits(void *data, size_t size, LoadType loadType) {
    if(loadType < LoadType::NLOADERTYPES){
        /*if(CreateLoaderDP(size))
            return 1;
        if(UploadToLoader(data, size))
            return 1;*/
    }


    switch (loadType) {
        case LoadType::LOADGEOM:{
            if (ExecuteAndWait(COMMAND_LOAD, PROCESS_READY, size, 0)) {
                //CloseLoaderDP();
                std::string errString = "Failed to send geometry to sub process:\n";
                errString.append(GetErrorDetails());
                throw std::runtime_error(errString);
            }
            //CloseLoaderDP();
            break;
        }
        case LoadType::LOADPARAM:{

            //if (ExecuteAndWait(COMMAND_UPDATEPARAMS, isRunning ? PROCESS_RUN : PROCESS_READY, size, isRunning ? PROCESS_RUN : PROCESS_READY)) {
            if (ExecuteAndWait(COMMAND_UPDATEPARAMS, PROCESS_READY, size, isRunning ? PROCESS_RUN : PROCESS_READY)) {
                //CloseLoaderDP();
                std::string errString = "Failed to send params to sub process:\n";
                errString.append(GetErrorDetails());
                throw std::runtime_error(errString);
            }
            //CloseLoaderDP();
            if(isRunning) { // restart
                StartSimulation();
            }
            break;
        }
        case LoadType::LOADAC:{
            if (ExecuteAndWait(COMMAND_LOADAC, PROCESS_RUNAC, size, 0)) {
                //CloseLoaderDP();
                std::string errString = "Failed to send AC geometry to sub process:\n";
                errString.append(GetErrorDetails());
                throw std::runtime_error(errString);
            }
            //CloseLoaderDP();
            break;
        }
        case LoadType::LOADHITS:{
            if(UploadToHitBuffer(data, size))
                throw std::runtime_error(MakeSubProcError("Failed to access hit buffer"));
            break;
        }
        default:{
            // Unspecified load type
            return 1;
        }
    }

    return 0;
}