

#pragma once

#include <cstddef> //size_t
#include <vector>
#include <mutex>
#include <list>
#include <map>
#include <optional>

enum ControllerState {
    Loading,
    Resetting,
    Pausing,
    ParamUpdating,
    Ready,
    InError,
    Exit,
    Starting,
    Initializing
};

enum ThreadState : int { //type specified to allow forward declare
	Idle, //Not running
	ThreadError, //Running + error (temporary, will exit -> Idle)
    Running,
    LimitReached
};

static std::map<ControllerState, std::string> controllerStateStrings = {
	{ControllerState::Loading,"Loading simu"},
	{ControllerState::Resetting,"Resetting simu"},
	{ControllerState::Pausing,"Stopping"},
	{ControllerState::ParamUpdating,"Updating otf.params"},
	{ControllerState::InError,"Error"},
    {ControllerState::Ready,"Ready"},
    {ControllerState::Exit,"Exited"},
    {ControllerState::Starting,"Starting"},
    {ControllerState::Initializing,"Initializing"}
};

static std::map<ThreadState, std::string> threadStateStrings = {
    {ThreadState::Running,"Running"},
    {ThreadState::Idle,"Idle"},
    {ThreadState::ThreadError,"Error"},
    {ThreadState::LimitReached,"Limit Reached"}
};

enum SimCommand {
    None,
    Load,
    Run,
    Pause,
    Reset,
    Kill,
    UpdateParams,
    MarkIdle
};

static std::map<SimCommand, std::string> simCommandStrings = {
    {SimCommand::None,"No command"},
    {SimCommand::Load,"Command: Load"},
    {SimCommand::Run,"Command: Run"},
    {SimCommand::Pause,"Command: Pause"},
    {SimCommand::Reset,"Command: Reset"},
    {SimCommand::Kill,"Command: Exit"},
    {SimCommand::UpdateParams,"Update params"},
    {SimCommand::MarkIdle,"Mark threads Idle"}
};

struct PROCESS_INFO{

    double cpu_time; // CPU time         (in second)
    size_t  mem_use;  // Memory usage     in byte (Windows) or kByte (Unix)
    size_t  mem_peak; // Max Memory usage (in byte)
    size_t counterSize; //GlobalSimuState size
};

struct ThreadInfo {
    size_t threadId=0;
    ThreadState threadState=ThreadState::Idle;
    std::string threadStatus;
    PROCESS_INFO runtimeInfo;
};

class LoadStatus_abstract;

struct ProcCommData {
    SimCommand masterCmd = SimCommand::None;
    size_t cmdParam = 0;
    size_t cmdParam2 = 0;
    std::string controllerStatus; //Allows to display fine-grained status in LoadStatus/Global Settings
    ControllerState controllerState = ControllerState::Initializing;

    std::vector<ThreadInfo> threadInfos;
    std::mutex procDataMutex; // To avoid writing to it while GUI refreshes LoadStatus window

    // Custom assignment operator
    ProcCommData& operator=(const ProcCommData& other) {
        if (this == &other) {
            return *this; // Check for self-assignment
        }

        // Copy all members except the mutex
        masterCmd = other.masterCmd;
        cmdParam = other.cmdParam;
        cmdParam2 = other.cmdParam2;
        controllerStatus = other.controllerStatus;
        controllerState = other.controllerState;
        threadInfos = other.threadInfos;

        // No need to copy the mutex, it's not copyable

        return *this;
    }

    void UpdateCounterSizes(const std::vector<size_t>& counterSizes);
    void UpdateControllerStatus(const std::optional<ControllerState>& state, const std::optional<std::string>& status, LoadStatus_abstract* loadStatus = nullptr);
};

//! Process Communication class for handling inter process/thread communication
struct ProcComm : ProcCommData {

    std::list<size_t> hitUpdateQueue; //For round-robin access. When a process in front is "processed", it's moved to back
    //std::mutex activeProcsMutex;

    // Custom assignment operator
    ProcComm& operator=(const ProcComm& other) {
        if (this == &other) {
            return *this; // Check for self-assignment
        }
        ProcCommData::operator=(other); // Call the base class's assignment operator
        return *this;
    }

    ProcComm() = default;
    ProcComm(size_t nbProcs) {
        Resize(nbProcs);
    };

    void Resize(size_t nbProcs) { //Called by constructor and by simulation manager's CreateCPUHandle()
        threadInfos.resize(nbProcs);
        InitHitUpdateQueue();
    };

    void PlaceFrontToBack(); //Called by simulation controller (SimThreadHandle::runloop) when thread-local hits are added to master hits

    void RemoveFromHitUpdateQueue(size_t id); //Called by simulation controller (SimThreadHandle::runloop) when end condition is met (and exit from loop), before final hit update

    void InitHitUpdateQueue(); //Called by constructor and on resize
    
};

//An abstract class that can display the status of subprocesses and issue an abort command
class LoadStatus_abstract {
public:
    virtual void Update() = 0; //Notify that the state has changed
    virtual void EnableStopButton() = 0;
    ProcCommData procStateCache; //Updated
    bool abortRequested = false;
};