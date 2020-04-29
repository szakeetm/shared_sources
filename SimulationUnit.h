//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONUNIT_H
#define MOLFLOW_PROJ_SIMULATIONUNIT_H

//#include "Simulation.h"

class SimulationUnit {
    int main(int argc, char *argv[]);

    virtual int LoadGeometry() = 0;

    virtual int UpdateParams() = 0;

    virtual int StartSim() = 0;

    virtual int StopSim() = 0;

    virtual int ResetSim() = 0;

    virtual int TerminateSim() = 0;

public:
    SimulationUnit(std::string appName , std::string dpName, size_t parentPID){
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
        const char* dpPrefix = "";
#else
        const char *dpPrefix = "/"; // creates semaphore as /dev/sem/%s_sema
#endif
        sprintf(this->appName,"%s", appName.c_str());
        sprintf(this->ctrlDpName,"%s", std::string(dpPrefix+dpName+"CTRL"+std::to_string(parentPID)).c_str());
        sprintf(this->loadDpName,"%s", std::string(dpPrefix+dpName+"LOAD"+std::to_string(parentPID)).c_str());
        sprintf(this->hitsDpName,"%s", std::string(dpPrefix+dpName+"HITS"+std::to_string(parentPID)).c_str());
        sprintf(this->logDpName,"%s", std::string(dpPrefix+dpName+"LOG"+std::to_string(parentPID)).c_str());
    };
protected:
    char appName[16];
    char ctrlDpName[32];
    char loadDpName[32];
    char hitsDpName[32];
    char logDpName[32];
};

int SimulationUnit::main(int argc, char *argv[]) {
    bool eos = false;

    if(argc!=3) {
        printf("Usage: molflowSub peerId index\n");
        return 1;
    }

    hostProcessId=atoi(argv[1]);
    prIdx = atoi(argv[2]);

    {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
        const char* dpPrefix = "MFLW";
#else
        const char* dpPrefix = "/MFLW"; // creates semaphore as /dev/sem/%s_sema
#endif
        sprintf(ctrlDpName,"%sCTRL%s",dpPrefix,argv[1]);
        sprintf(loadDpName,"%sLOAD%s",dpPrefix,argv[1]);
        sprintf(hitsDpName,"%sHITS%s",dpPrefix,argv[1]);
        sprintf(logDpName, "%sLOG%s",dpPrefix,argv[1]);
    }
    dpControl = OpenDataport(ctrlDpName,sizeof(SHCONTROL));
    if( !dpControl ) {
        printf("Cannot connect to %s\n",ctrlDpName);
        return 1;
    }

    printf("Connected to %s (%zd bytes), molflowSub.exe #%d\n",ctrlDpName,sizeof(SHCONTROL),prIdx);

    InitSimulation(); //Creates sHandle instance

    // Sub process ready
    SetReady();

    // Main loop
    while( !endState ) {
        GetState();
        switch(prState) {

            case COMMAND_LOAD:
                printf("[%d] COMMAND: LOAD (%zd,%zu)\n", prIdx, prParam,prParam2);
                Load();
                if( sHandle->loadOK ) {
                    //sHandle->desorptionLimit = prParam2; // 0 for endless
                    SetReady();
                }
                break;

            case COMMAND_LOADAC:
                printf("[%d] COMMAND: LOADAC (%zd)\n", prIdx,prParam);
                LoadAC();
                break;

            case COMMAND_UPDATEPARAMS:
                printf("[%d] COMMAND: UPDATEPARAMS (%zd,%zd)\n", prIdx, prParam, prParam2);
                if (UpdateParams()) {
                    SetState(prParam2, GetSimuStatus());
                }
                break;

            case COMMAND_RELEASEDPLOG:
                printf("[%d] COMMAND: RELEASEDPLOG (%zd,%zd)\n", prIdx, prParam, prParam2);
                CLOSEDPSUB(dpLog);
                SetState(prParam, GetSimuStatus());
                break;

            case COMMAND_START:
                printf("[%d] COMMAND: START (%zd,%zu)\n", prIdx,prParam,prParam2);
                if( sHandle->loadOK ) {
                    if( StartSimulation(prParam) )
                        SetState(PROCESS_RUN,GetSimuStatus());
                    else {
                        if( GetLocalState()!=PROCESS_ERROR )
                            SetState(PROCESS_DONE,GetSimuStatus());
                    }
                } else
                    SetErrorSub("No geometry loaded");
                break;

            case COMMAND_PAUSE:
                printf("[%d] COMMAND: PAUSE (%zd,%zu)\n", prIdx,prParam,prParam2);
                if( !sHandle->lastHitUpdateOK ) {
                    // Last update not successful, retry with a longer timeout
                    if (dpHit && (GetLocalState() != PROCESS_ERROR)) UpdateHits(dpHit,dpLog,prIdx,60000);
                }
                SetReady();
                break;

            case COMMAND_RESET:
                printf("[%d] COMMAND: RESET (%zd,%zu)\n", prIdx,prParam,prParam2);
                ResetSimulation();
                SetReady();
                break;

            case COMMAND_EXIT:
                printf("[%d] COMMAND: EXIT (%zd,%zu)\n", prIdx,prParam,prParam2);
                endState = true;
                break;

            case COMMAND_CLOSE:
                printf("[%d] COMMAND: CLOSE (%zd,%zu)\n", prIdx,prParam,prParam2);
                ClearSimulation();
                CLOSEDPSUB(dpHit);
                CLOSEDPSUB(dpLog);
                SetReady();
                break;

            case COMMAND_STEPAC:
                // Debug command
                printf("[%d] COMMAND: STEPAC (%zd,%zu)\n", prIdx,prParam,prParam2);
                if( sHandle->loadOK ) {
                    if( StartSimulation(prParam) ) {
                        SetState(PROCESS_RUN,GetSimuStatus());
                        SimulationACStep(1);
                        if(dpHit) UpdateHits(dpHit,dpLog,prIdx,20);
                        SetReady();
                    } else {
                        if( GetLocalState()!=PROCESS_ERROR )
                            SetState(PROCESS_DONE,GetSimuStatus());
                    }
                } else
                    SetErrorSub("No geometry loaded");
                break;

            case PROCESS_RUN:
                SetStatus(GetSimuStatus()); //update hits only
                eos = SimulationRun();      // Run during 1 sec
                if (dpHit && (GetLocalState() != PROCESS_ERROR)) {
                    UpdateHits(dpHit, dpLog, prIdx,20); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
                }
                if(eos) {
                    if( GetLocalState()!=PROCESS_ERROR ) {
                        // Max desorption reached
                        SetState(PROCESS_DONE,GetSimuStatus());
                        printf("[%d] COMMAND: PROCESS_DONE (Max reached)\n",prIdx);
                    }
                }
                break;

            default:

                ProcessSleep(WAITTIME);
                break;
        }
    }
    // Release and clear memory
    SetState(PROCESS_KILLED,"");
    delete sHandle;
    //CLOSEDP(dpControl);
    //CLOSEDP(dpHit);
    //Why bother closing dataports? Windows will release handles automatically.
    return 0;
}

#endif //MOLFLOW_PROJ_SIMULATIONUNIT_H
