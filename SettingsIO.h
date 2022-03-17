//
// Created by pascal on 5/17/21.
//

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
    extern bool autogenerateTest;

    int prepareIO();
    int initDirectories();
    int initFromZip();
    void cleanup_files();
}


#endif //MOLFLOW_PROJ_SETTINGSIO_H
