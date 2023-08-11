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

#include "FormatHelper.h"
#include <cstdio>

namespace Util {

// Name: formatInt()
// Desc: Format an integer in K,M,G,..
    char *formatInt(size_t v, const char *unit) {

        double x = (double) v;

        static char ret[64];
        if (x < 1E3) {
            sprintf(ret, "%g %s", (double) x, unit);
        } else if (x < 1E6) {
            sprintf(ret, "%.1f K%s", x / 1E3, unit);
        } else if (x < 1E9) {
            sprintf(ret, "%.2f M%s", x / 1E6, unit);
        } else if (x < 1E12) {
            sprintf(ret, "%.2f G%s", x / 1E9, unit);
        } else {
            sprintf(ret, "%.2f T%s", x / 1E12, unit);
        }

        return ret;
    }

// Name: formatPs()
// Desc: Format a double in K,M,G,.. per sec
    char *formatPs(double v, const char *unit) {

        static char ret[64];
        if (v < 1000.0) {
            sprintf(ret, "%.1f %s/s", v, unit);
        } else if (v < 1000000.0) {
            sprintf(ret, "%.1f K%s/s", v / 1000.0, unit);
        } else if (v < 1000000000.0) {
            sprintf(ret, "%.1f M%s/s", v / 1000000.0, unit);
        } else {
            sprintf(ret, "%.1f G%s/s", v / 1000000000.0, unit);
        }

        return ret;
    }

// Name: formatSize()
// Desc: Format a double in K,M,G,.. per sec
    char *formatSize(size_t size) {

        static char ret[64];
        if (size < 1024UL) {
            sprintf(ret, "%zd Bytes", size);
        } else if (size < 1048576UL) {
            sprintf(ret, "%.1f KB", (double) size / 1024.0);
        } else if (size < 1073741824UL) {
            sprintf(ret, "%.1f MB", (double) size / 1048576.0);
        } else {
            sprintf(ret, "%.1f GB", (double) size / 1073741824.0);
        }

        return ret;
    }

// Name: formatTime()
// Desc: Format time in HH:MM:SS
    char *formatTime(float t) {
        static char ret[64];
        int nbSec = (int) (t + 0.5f);
        sprintf(ret, "%02d:%02d:%02d", nbSec / 3600, (nbSec % 3600) / 60, nbSec % 60);
        return ret;
    }

    char *formatTime(double t) {
        static char ret[64];
        int nbSec = (int) (t + 0.5f);
        sprintf(ret, "%02d:%02d:%02d", nbSec / 3600, (nbSec % 3600) / 60, nbSec % 60);
        return ret;
    }
}