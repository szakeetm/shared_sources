//
// Created by pascal on 5/17/21.
//

#ifndef MOLFLOW_PROJ_SETTINGSIO_H
#define MOLFLOW_PROJ_SETTINGSIO_H

#include <string>
#include <list>
#include <vector>

namespace SettingsIO {
    extern std::string workFile;
    extern std::string workPath;
    extern std::string inputFile;
    extern std::string inputPath;
    extern std::string outputFile;
    extern std::string outputPath;
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
