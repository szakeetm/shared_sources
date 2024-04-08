#ifndef MOLFLOW_PROJ_RTHELPER_H
#define MOLFLOW_PROJ_RTHELPER_H

struct SimulationFacetTempVar {
    // Temporary var (used in Intersect for collision)
    SimulationFacetTempVar(){
        colDistTranspPass=1.0E99;
        colU = 0.0;
        colV = 0.0;
        isHit=false;
    }
    SimulationFacetTempVar(double d, double u, double v, bool hit){
        colDistTranspPass=d;
        colU = u;
        colV = v;
        isHit=hit;
    }
    double colDistTranspPass;
    double colU;
    double colV;
    bool   isHit;
};

#endif //MOLFLOW_PROJ_RTHELPER_H
