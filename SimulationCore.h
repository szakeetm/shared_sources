//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONCORE_H
#define MOLFLOW_PROJ_SIMULATIONCORE_H

class SimulationCore {
    int main();
    virtual int LoadGeometry() = 0;
    virtual int UpdateParams() = 0;
    virtual int StartSim() = 0;
    virtual int StopSim() = 0;
    virtual int ResetSim() = 0;
    virtual int TerminateSim() = 0;
};

#endif //MOLFLOW_PROJ_SIMULATIONCORE_H
