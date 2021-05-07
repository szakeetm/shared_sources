//
// Created by pascal on 4/22/21.
//

#ifndef MOLFLOW_PROJ_RTHELPER_H
#define MOLFLOW_PROJ_RTHELPER_H

struct SubProcessFacetTempVar {
    // Temporary var (used in Intersect for collision)
    SubProcessFacetTempVar(){
        colDistTranspPass=1.0E99;
        colU = 0.0;
        colV = 0.0;
        isHit=false;
    }
    double colDistTranspPass;
    double colU;
    double colV;
    bool   isHit;
};

#endif //MOLFLOW_PROJ_RTHELPER_H
