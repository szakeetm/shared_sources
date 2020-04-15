//
// Created by pbahr on 15/04/2020.
//

#include "SimulationManager.h"


SimulationManager::SimulationManager() {
    useCPU = false;
    nbCores = false;

    useGPU = false;

    useRemote = false;
}

SimulationManager::~SimulationManager() {

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

int SimulationManager::CreateCPUHandle() {
    return 0;
}

int SimulationManager::CreateGPUHandle() {
    return 0;
}

int SimulationManager::CreateRemoteHandle() {
    return 0;
}
