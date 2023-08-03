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

#pragma once

#include <cstddef> //size_t
#include <vector>
#include <mutex>
#include <list>
#include <map>

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

enum ThreadState {
	Idle, //Not running
	HitUpdate, //Running + updating hits
	ThreadError, //Running + error (temporary, will exit -> Idle)
    Running
};

static std::map<ControllerState, std::string> controllerStateStrings = {
	{ControllerState::Loading,"Loading simu"},
	{ControllerState::Resetting,"Resetting simu"},
	{ControllerState::Pausing,"Stopping"},
	{ControllerState::ParamUpdating,"Updating otf.params"},
	{ControllerState::InError,"Error"},
    {ControllerState::Ready,"Ready"},
    {ControllerState::Exit,"Finished"},
    {ControllerState::Starting,"Starting"},
    {ControllerState::Initializing,"Initializing"}
};

static std::map<ThreadState, std::string> threadStateStrings = {
    {ThreadState::HitUpdate,"HitUpdate"},
    {ThreadState::Running,"Running"},
    {ThreadState::Idle,"Idle"},
    {ThreadState::ThreadError,"Error"}
};

enum SimCommand {
    None,
    Load,
    Run,
    Pause,
    Reset,
    Kill,
    UpdateParams
};

static std::map<SimCommand, std::string> simCommandStrings = {
    {SimCommand::None,"No command"},
    {SimCommand::Load,"Command: Load"},
    {SimCommand::Run,"Command: Run"},
    {SimCommand::Pause,"Command: Pause"},
    {SimCommand::Reset,"Command: Reset"},
    {SimCommand::Kill,"Command: Exit"},
    {SimCommand::UpdateParams,"Update params"}
};

struct PROCESS_INFO{

    double cpu_time; // CPU time         (in second)
    size_t  mem_use;  // Memory usage     (in byte)
    size_t  mem_peak; // MAx Memory usage (in byte)

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
        threadInfos = other.threadInfos;

        // No need to copy the mutex, it's not copyable

        return *this;
    }

    void UpdateControllerStatus(const std::string& status, LoadStatus_abstract* loadStatus = nullptr);
    void UpdateControllerStatus(const ControllerState state, const std::string& status, LoadStatus_abstract* loadStatus = nullptr);
};

//! Process Communication class for handling inter process/thread communication
struct ProcComm : ProcCommData {

    std::list<size_t> activeProcs; //For round-robin access. When a process in front is "processed", it's moved to back
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
        InitActiveProcList();
    };

    void PlaceFrontToBack(); //Called by simulation controller (SimThreadHandle::runloop) when thread-local hits are added to master hits

    void RemoveAsActive(size_t id); //Called by simulation controller (SimThreadHandle::runloop) when end condition is met (and exit from loop), before final hit update

    void InitActiveProcList(); //Called by constructor and on resize
    
};

//An abstract class that can display the status of subprocesses and issue an abort command
class LoadStatus_abstract {
public:
    virtual void Update() = 0; //Notify that the state has changed
    ProcCommData procStateCache; //Updated
    bool abortRequested = false;
};