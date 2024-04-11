

#pragma once

#include <string>
#include <list>
#include <vector>
#include <map>
#include "Interface/GeometryTypes.h" // selections

namespace SettingsIO {

    struct CLIArguments {
        size_t nbThreads = 0;
        uint64_t simDuration = 0;
        uint64_t desLimit = 0;
        uint64_t statprintInterval = 60;
        uint64_t autoSaveInterval = 600; // default: autosave every 600s=10min
        bool loadAutosave = false;
        
        bool resetOnStart = false;
        std::string paramFile;
        std::vector<std::string> paramChanges;
        bool noProgress = false;  //If true, doesn't print percentage updates for CLI progressbars

        bool overwrite = false; //! whether overwriting the input file is allowed
        bool outputFacetDetails = false; //write facet details at end of simulation
        bool outputFacetQuantities = false; //write derived facet quantities at end of simulation

        std::string workFile; //! Uncompressed file that is used for the simulation
        std::string workPath; //! Output path for various simulation files (e.g. autosave), can be different from outputPath e.g. for cluster simulations
        std::string inputFile; //! Uncompressed or compressed file that is used for the simulation
        std::string outputFile; //! Output file name, can contain path, in which case it also sets outputPath
        std::string outputPath; //! Output path for output file
    };

    void prepareIO(CLIArguments& parsedArgs);
    int initDirectories(CLIArguments& parsedArgs);
    int initFromZip(CLIArguments& parsedArgs);
    void cleanup_files(const std::string& outPath, const std::string& workPath);
}
