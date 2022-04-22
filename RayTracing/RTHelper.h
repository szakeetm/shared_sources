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
    SubProcessFacetTempVar(double d, double u, double v, bool hit){
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
