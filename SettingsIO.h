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

#ifndef MOLFLOW_PROJ_SETTINGSIO_H
#define MOLFLOW_PROJ_SETTINGSIO_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include "GeometryTypes.h" // selecitons

namespace SettingsIO {
    extern std::string workFile;
    extern std::string workPath;
    extern std::string inputFile;
    extern std::string inputPath;
    extern std::string outputFile;
    extern std::string outputPath;
    extern std::vector<std::string> extraFiles;
    extern std::map<std::string, std::vector<std::string>> cachedLines;
    extern std::vector<std::vector<std::string>> formulas;
    extern std::vector<SelectionGroup> selections;

    extern bool overwrite;
    extern bool isArchive;
    extern bool outputFacetDetails;
    extern bool outputFacetQuantities;
    extern double autogenerateTest;

    int prepareIO();
    int initDirectories();
    int initFromZip();
    void cleanup_files();
}


#endif //MOLFLOW_PROJ_SETTINGSIO_H
