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

#ifndef MOLFLOW_PROJ_GUIDEFINES_H
#define MOLFLOW_PROJ_GUIDEFINES_H

#include <vector>
#include "GLApp/GLChart/GLChartConst.h"

namespace ColorSchemes{
    // Default color scheme (softer classic Molflow colors)
    static std::vector<GLColor> defaultCol = {
            GLColor(255,000,055), //red
            GLColor(000,000,255), //blue
            GLColor(000,204,051), //green
            GLColor(000,000,000), //black
            GLColor(255,153,051), //orange
            GLColor(153,204,255), //light blue
            GLColor(153,000,102), //violet
            GLColor(255,230,005) //yellow
    };

    // Paul Tol
    static std::vector<GLColor> colorBrewer = {
            GLColor(228,26,28), //red
            GLColor(55,126,184), //blue
            GLColor(77,175,74), //green
            GLColor(152,78,163), //purple
            GLColor(255,127,0), //orange
            GLColor(255,255,51), //yellow
            GLColor(166,86,40), //brown
            GLColor(247,129,191), //pink
            GLColor(153,153,153) //grey
    };

    // Colorblind: Paul Tol
    static std::vector<GLColor> paulTol = {
            GLColor(238,119,51), //orange
            GLColor(0,119,187), //blue
            GLColor(51,187,238), //cyan
            GLColor(238,51,119), //magenta
            GLColor(204,51,17), //red
            GLColor(0,153,136), //teal
            GLColor(187,187,187) // grey
    };

    // Colorblind: Okabe and Ito
    static std::vector<GLColor> okabeIto = {
            GLColor(230,159,0), //orange
            GLColor(86,180,233), //sky blue
            GLColor(0,158,115), //blueish green
            GLColor(240,228,66), //yellow
            GLColor(0,114,178), //blue
            GLColor(213,94,0), //vermillion
            GLColor(204,121,167), //reddish purple
            GLColor(0,0,0) //black
    };
}
#endif //MOLFLOW_PROJ_GUIDEFINES_H
