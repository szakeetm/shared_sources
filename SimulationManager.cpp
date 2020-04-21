//
// Created by pbahr on 15/04/2020.
//

#include <string>
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <process.h>
#endif
#include "SimulationManager.h"
#include "Buffer_shared.h" // TODO: Move SHCONTROL to seperate file or SMP.h
#include "SMP.h"

SimulationManager::SimulationManager() {
    useCPU = false;
    nbCores = 0;

    useGPU = false;

    useRemote = false;

    dpControl = nullptr;
    dpHit = nullptr;
    dpLog = nullptr;
    dpLoader = nullptr;

    char dpName[5];
#if defined(MOLFLOW)
    sprintf(appName,"molflow");
    sprintf(dpName,"MFLW");
#elif defined(SYNRAD)
    sprintf(appName,"synrad");
    sprintf(dpName,"SNRD");
#endif

    uint32_t  pid;
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    pid = _getpid();
    const char* dpPrefix = "MFLW";
#else
    pid = ::getpid();
        const char* dpPrefix = "/MFLW"; // creates semaphore as /dev/sem/%s_sema
#endif
    sprintf(ctrlDpName,"%sCTRL%u",dpPrefix,pid);
    sprintf(loadDpName,"%sLOAD%u",dpPrefix,pid);
    sprintf(hitsDpName,"%sHITS%u",dpPrefix,pid);
    sprintf(logDpName, "%sLOG%u",dpPrefix,pid);

    allProcsDone = false;
}

SimulationManager::~SimulationManager() {
    CLOSEDP(dpControl);
    CLOSEDP(dpLoader);
    CLOSEDP(dpHit);
    CLOSEDP(dpLog);
}

int SimulationManager::LoadInput() {

    return 0;
}

int SimulationManager::ResetStatsAndHits() {

    return 0;
}

int SimulationManager::ChangeSimuParams() {

    return 0;
}

int SimulationManager::StartSimulation() {

    return 0;
}

int SimulationManager::StopSimulation() {

    return 0;
}

int SimulationManager::TerminateSimHandles() {

    return 0;
}

int SimulationManager::FetchResults() {

    return 0;
}

int SimulationManager::CreateCPUHandle(uint16_t iProc) {
    char cmdLine[512];
    uint32_t processId;

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    processId = _getpid();
#else
    processId = ::getpid();
#endif //  WIN

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    sprintf(cmdLine,"%sSub.exe %d %hu",appName,processId,iProc);
#else
    char **arguments;
    arguments = new char*[2];
    for(int arg=0;arg<2;arg++)
        arguments[arg] = new char[10];
    sprintf(cmdLine,"./%sSub",appName);
    sprintf(arguments[0],"%d",processId);
    sprintf(arguments[1],"%hu",iProc);
    //sprintf(argumente[2],"%s",'\0');
    //argumente[2] = NULL;
#endif

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    simHandles.emplace_back(
            StartProc(cmdLine, STARTPROC_NORMAL, nullptr),
            SimType::simCPU);
#else
    simHandles.emplace_back(
            StartProc(cmdLine, STARTPROC_NORMAL, static_cast<char **>(arguments)),
            SimType::simCPU);
#endif

    // Wait a bit
    ProcessSleep(25);

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

int SimulationManager::CreateLoaderDP(size_t loaderSize) {

    //std::string loaderString = SerializeForLoader().str();
    //size_t loadSize = loaderSize.size();
    dpLoader = CreateDataport(loadDpName, loaderSize);
    if (!dpLoader)
        return 1;
        //throw Error("Failed to create 'loader' dataport.\nMost probably out of memory.\nReduce number of subprocesses or texture size.");

    //progressDlg->SetMessage("Accessing dataport...");
//    AccessDataportTimed(dpLoader, (DWORD)(3000 + nbCores*loaderSize / 10000));
//    //progressDlg->SetMessage("Assembling geometry to pass...");
//
//    std::copy(loaderSize.begin(), loaderSize.end(), (BYTE*)dpLoader->buff);
//
//    //progressDlg->SetMessage("Releasing dataport...");
//    ReleaseDataport(dpLoader);
    return 0;
}

int SimulationManager::CreateControlDP() {
    if( !dpControl )
        dpControl = CreateDataport(ctrlDpName,sizeof(SHCONTROL));
    if( !dpControl )
        return 1;
        //throw Error("Failed to create 'control' dataport");
    AccessDataport(dpControl);
    memset(dpControl->buff,0,sizeof(SHCONTROL));
    ReleaseDataport(dpControl);

    return 0;
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
        CLOSEDP(dpLoader);
        return 1;
        //throw Error("Failed to create 'hits' dataport: out of memory.");
    }

    // ClearHits
    AccessDataport(dpHit);
    memset(dpHit->buff, 0, hitSize); //Also clears hits, leaks
    ReleaseDataport(dpHit);
    return 0;
}

/*!
 * @brief Creates Simulation Units and waits for their ready status
 * @return 0=all SimUnits are ready, 1=else
 */
int SimulationManager::InitSimUnits() {

    // First create Control DP, so subprocs can communicate
    if(CreateControlDP())
        return 1;

    if(useCPU){
        // Launch nbCores subprocesses
        auto nbActiveProcesses = simHandles.size();
        for(int iProc = 0; iProc < nbCores; ++iProc) {
            if(CreateCPUHandle(iProc + nbActiveProcesses)) // abort initialization when creation fails
                return 1;
        }
    }
    if(useGPU){
        CreateGPUHandle();
    }
    if(useRemote){
        CreateRemoteHandle();
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
        AccessDataport(dpControl);
        SHCONTROL *shMaster = (SHCONTROL *)dpControl->buff;

        for (size_t i = 0; i < simHandles.size(); i++) {

            finished = finished & (shMaster->states[i]==procStatus || shMaster->states[i]==PROCESS_ERROR || shMaster->states[i]==PROCESS_DONE);
            if( shMaster->states[i]==PROCESS_ERROR ) {
                error = true;
            }
            allProcsDone = allProcsDone & (shMaster->states[i] == PROCESS_DONE);
        }
        ReleaseDataport(dpControl);

        if (!finished) {
            ProcessSleep(250);
            waitTime += 250;
        }
    } while (!finished && waitTime<timeOutAt);

    return waitTime>=timeOutAt || error; // 0 = finished, 1 = timeout
}

int SimulationManager::ForwardCommand(const int command, const size_t param) const {
    if(!dpControl) {
        return 1;
    }

    // Send command
    AccessDataport(dpControl);
    SHCONTROL *shMaster = (SHCONTROL *)dpControl->buff;
    for(size_t i=0;i<simHandles.size();i++) {
        shMaster->states[i]=command;
        shMaster->cmdParam[i]=param;
    }
    ReleaseDataport(dpControl);

    return 0;
}

/*!
 * @brief Shortcut function combining ForwardCommand() and WaitForProcStatus() into a single call
 * @param command execution command for the subprocesses
 * @param procStatus status of subprocesses that has to be reached
 * @param param additional command parameter
 * @return 0=success, 1=fail
 */
int SimulationManager::ExecuteAndWait(const int command, const uint8_t procStatus, const size_t param) {
    if(!ForwardCommand(command, param)) { // execute
        if (!WaitForProcStatus(procStatus)) { // and wait
            return 0;
        }
    }
    return 1;
}

int SimulationManager::KillAllSimUnits() {
    if( dpControl && simHandles.size()>0 ) {
        if(!ForwardCommand(COMMAND_EXIT)){ // execute
            if(!WaitForProcStatus(PROCESS_KILLED)){ // and wait
                // Force kill
                AccessDataport(dpControl);
                SHCONTROL *shMaster = (SHCONTROL *)dpControl->buff;
                for(size_t i=0;i<simHandles.size();i++) {
                    if (shMaster->states[i] != PROCESS_KILLED){
                        if(!KillProc(simHandles[i].first))
                            return 1; // proc couldn't be killed!?
                    }
                }
                ReleaseDataport(dpControl);

                simHandles.clear();
            }
        }
        else return 1;
    }
    nbCores = 0;
    return 0;
}

int SimulationManager::ClearHitsBuffer() {
    if (!dpHit) {
        return 1;
    }
    AccessDataport(dpHit);
    memset(dpHit->buff, 0, dpHit->size); //Also clears hits, leaks
    ReleaseDataport(dpHit);
    return 0;
}

int SimulationManager::CloseLoaderDP() {
    CLOSEDP(dpLoader);
    return 0;
}

int SimulationManager::CloseControlDP() {
    CLOSEDP(dpControl);
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

int SimulationManager::GetProcStatus(std::vector<ProcInfo>& procInfoList) {
    AccessDataport(dpControl);
    SHCONTROL *shMaster = (SHCONTROL *)dpControl->buff;
    for(int i = 0; i<simHandles.size();++i){
        ProcInfo pInfo;
        pInfo.procId = simHandles[i].first;
        pInfo.statusId = shMaster->states[i];
        pInfo.statusString = shMaster->statusStr[i];
        pInfo.cmdParam = shMaster->cmdParam[i];
        pInfo.cmdParam2 = shMaster->cmdParam2[i];
        procInfoList.emplace_back(pInfo);
    }

    ReleaseDataport(dpControl);
    return 0;
}

int SimulationManager::GetProcStatus(size_t *states, std::vector<std::string>& statusStrings) {
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
    return 0;
}

BYTE *SimulationManager::GetLockedHitBuffer() {
    if (dpHit && AccessDataport(dpHit)) {
        // A copy might result in too much memory requirement depending on the geometry
        /*BYTE* bufferCopy;
        bufferCopy = new BYTE[dpHit->size];
        std::copy((BYTE*)dpHit->buff,(BYTE*)dpHit->buff + dpHit->size,bufferCopy);
        ReleaseDataport(dpHit);*/
        return (BYTE*)dpHit->buff;
    }
    return nullptr;
}

int SimulationManager::UnlockHitBuffer() {
    if (dpHit && ReleaseDataport(dpHit)) {
        return 0;
    }
    return 1;
}

int SimulationManager::UploadToLoader(void *data, size_t size) {
    if (dpLoader && AccessDataport(dpLoader)) {
        std::copy((BYTE*)data,(BYTE*)data + size,(BYTE*)dpLoader->buff);
        ReleaseDataport(dpLoader);
        return 0;
    }

    return 1;
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
