//
// Created by Pascal Baehr on 28.04.20.
//

#include "SimulationManager.h"
#include "SMP.h"
#include "Buffer_shared.h"
#include <fstream>
#include <filesystem>
#include <Parameter.h>
#include <IO/LoaderXML.h>
#include <IO/WriterXML.h>
#include "GeometrySimu.h"
#include "Initializer.h"
#include "Helper/MathTools.h"
#include <omp.h>
#include <sstream>

static constexpr const char* molflowCliLogo = R"(
  __  __     _  __ _             ___ _    ___
 |  \/  |___| |/ _| |_____ __ __/ __| |  |_ _|
 | |\/| / _ \ |  _| / _ \ V  V / (__| |__ | |
 |_|  |_\___/_|_| |_\___/\_/\_/ \___|____|___|
    )";



struct MolflowData{
    // Geometry
    WorkerParams wp;
    GeomProperties sh;
    OntheflySimulationParams ontheflyParams;

    std::vector<Vector3d>   vertices3;        // Vertices
    std::vector<SuperStructure> structures; //They contain the facets
    std::vector<std::vector<std::pair<double, double>>> CDFs; //cumulative distribution function for each temperature
    std::vector<std::vector<std::pair<double, double>>> IDs; //integrated distribution function for each time-dependent desorption type
    std::vector<double> temperatures; //keeping track of all temperatures that have a CDF already generated
    std::vector<double> moments;      //time values (seconds) when a simulation state is measured
    std::vector<size_t> desorptionParameterIDs; //time-dependent parameters which are used as desorptions, therefore need to be integrated
    std::vector<Parameter> parameters; //Time-dependent parameters
};

int main(int argc, char** argv) {
    std::cout << molflowCliLogo << std::endl;

    SimulationManager simManager("molflow","MFLW");
    SimulationModel model{};
    GlobalSimuState globState{};
    Initializer::init(argc, argv, &simManager, &model, &globState);
    size_t oldHitsNb = globState.globalHits.globalHits.hit.nbMCHit;
    size_t oldDesNb = globState.globalHits.globalHits.hit.nbDesorbed;

    // Skip desorptions if limit was already reached
    if(!Settings::desLimit.empty())
    {
        size_t listSize = Settings::desLimit.size();
        for(size_t l = 0; l < listSize; ++l) {
            if (oldDesNb > Settings::desLimit.front()){
                printf("Skipping desorption limit: %llu\n",Settings::desLimit.front());
                Settings::desLimit.pop_front();
            }
            else{
                printf("Starting with desorption limit: %llu from %zu\n",Settings::desLimit.front(), oldDesNb);
                model.otfParams.desorptionLimit = Settings::desLimit.front();
                simManager.ForwardOtfParams(&model.otfParams);
                break;
            }
        }
        if(Settings::desLimit.empty()){
            printf("All given desorption limits have been reached. Consider resetting the simulation results from the input file (--reset): Starting desorption %zu\n", oldDesNb);
            exit(0);
        }
    }

    // Create copy of input file for autosave
    std::string autoSave;
    if(Settings::autoSaveDuration > 0)
    {
        autoSave = std::filesystem::path(Settings::req_real_file).filename().string();

        std::string autoSavePrefix = "autosave_";
        if(autoSave.size() > autoSavePrefix.size() && std::search(autoSave.begin(), autoSave.begin()+autoSavePrefix.size(), autoSavePrefix.begin(), autoSavePrefix.end()) == autoSave.begin())
        {
            autoSave = std::filesystem::path(Settings::req_real_file).filename().string();
            Settings::req_real_file = autoSave.substr( autoSavePrefix.size(), autoSave.size() - autoSavePrefix.size());
            std::cout << "Using autosave file " << autoSave << " for "<<Settings::req_real_file<<'\n';
        }
        else {
            // create autosavefile from copy of original
            std::stringstream autosaveFile;
            autosaveFile << autoSavePrefix<< autoSave;
            autoSave = autosaveFile.str();

            try {
                std::filesystem::copy_file(Settings::req_real_file, autoSave,
                                           std::filesystem::copy_options::overwrite_existing);
            } catch (std::filesystem::filesystem_error &e) {
                std::cout << "Could not copy file: " << e.what() << '\n';
            }
        }
    }

    //simManager.ReloadHitBuffer();
    try {
        simManager.StartSimulation();
    }
    catch (std::runtime_error& e) {
        std::cout << "ERROR: Starting simulation: " << e.what() << std::endl;
        exit(0);
    }
    /* Establish a handler for SIGALRM signals. */
    double timeNow = GetSysTimeMs();

    std::cout << "Commencing simulation for " << Settings::simDuration << " seconds from "<< globState.globalHits.globalHits.hit.nbDesorbed << " desorptions." << std::endl;
    //ProcessSleep(1000*Settings::simDuration);
    double timeStart = omp_get_wtime();
    double timeEnd = (Settings::simDuration > 0) ? timeStart + 1.0 * Settings::simDuration : std::numeric_limits<double>::max();

    bool endCondition = false;
    //std::cout << "." << std::flush << '\b';
    do {
        ProcessSleep(1000);
        /*ProcessSleep(1000);
        //usleep(1000000);
        std::cout << "." << std::flush;
        ProcessSleep(1000);
        std::cout << "\b.." << std::flush;
        ProcessSleep(1000);
        std::cout << "\b\b..." << std::flush;
        ProcessSleep(1000);
        std::cout << "\b\b\b...." << std::flush;
        ProcessSleep(1000);
        std::cout << "\b\b\b\b....." << std::flush<< "\b\b\b\b\b";
        */

        timeNow = omp_get_wtime();
        if(model.otfParams.desorptionLimit != 0)
            endCondition = globState.globalHits.globalHits.hit.nbDesorbed/* - oldDesNb*/ >= model.otfParams.desorptionLimit;
        //std::cout << "["<<timeNow-timeStart<<"s] "<< globState.globalHits.globalHits.hit.nbMCHit - oldHitsNb<< " : " << (double)(globState.globalHits.globalHits.hit.nbMCHit - oldHitsNb) / ((timeNow-timeStart) > 1e-8 ? (timeNow-timeStart) : 1.0) << std::endl;
        //printf("Des: %zu - %zu > %llu\n", globState.globalHits.globalHits.hit.nbDesorbed, oldDesNb, model.otfParams.desorptionLimit);
        //printf("Hit: %zu - %zu\n", globState.globalHits.globalHits.hit.nbMCHit, oldHitsNb);
        //printf("Trans Prob: %lu -> %e\n", globState.globalHits.globalHits.hit.nbDesorbed,globState.facetStates[1].momentResults[0].hits.hit.nbAbsEquiv / globState.globalHits.globalHits.hit.nbDesorbed);

        //printf("%d && %d\n", timeNow < timeEnd, endCondition);
        if(endCondition){
            printf("--- Trans Prob: %lu -> %e\n", globState.globalHits.globalHits.hit.nbDesorbed, globState.facetStates[1].momentResults[0].hits.hit.nbAbsEquiv / globState.globalHits.globalHits.hit.nbDesorbed);
            std::stringstream outFile;
            outFile << "out_" << model.otfParams.desorptionLimit <<".xml";
            try {
                std::filesystem::copy_file(Settings::req_real_file, outFile.str(), std::filesystem::copy_options::overwrite_existing);
            } catch(std::filesystem::filesystem_error& e) {
                std::cout << "Could not copy file: " << e.what() << '\n';
            }

            //FlowIO::WriterXML::SaveGeometry(outFile.str(), &model);
            FlowIO::WriterXML::SaveSimulationState(outFile.str(), &model, globState);
            // if there is a next des limit, handle that
            if(!Settings::desLimit.empty()) {
                model.otfParams.desorptionLimit = Settings::desLimit.front();
                Settings::desLimit.pop_front();
                simManager.ForwardOtfParams(&model.otfParams);
                endCondition = false;
                std::cout << " Handling next des limit " << model.otfParams.desorptionLimit << std::endl;
                try {
                    ProcessSleep(1000);
                    simManager.StartSimulation();
                }
                catch (std::runtime_error& e) {
                    std::cerr << "ERROR: Starting simulation: " << e.what() << std::endl;
                    endCondition = true;
                }
            }
        }
        else if(Settings::autoSaveDuration && (uint64_t)(timeNow-timeStart)%Settings::autoSaveDuration==0){ // autosave every x seconds
            printf("[%.0lfs] Creating auto save file %s\n", timeNow-timeStart, autoSave.c_str());
            FlowIO::WriterXML::SaveSimulationState(autoSave, &model, globState);
        }
        else if(!Settings::autoSaveDuration && (uint64_t)(timeNow-timeStart)%60==0){
            printf("[%.0lfs] time remaining -- [%lf] %lu : %e Hit/s\n", timeEnd-timeNow, timeNow-timeStart, globState.globalHits.globalHits.hit.nbMCHit - oldHitsNb, (double)(globState.globalHits.globalHits.hit.nbMCHit-oldHitsNb)/(timeNow-timeStart));
        }
    } while(timeNow < timeEnd && !endCondition);
    std::cout << "Simulation finished!" << std::endl << std::flush;

    // Stop and copy results
    simManager.StopSimulation();

    std::cout << "Hit["<<timeNow-timeStart<<"s] "<< globState.globalHits.globalHits.hit.nbMCHit - oldHitsNb
              << " : " << (double)(globState.globalHits.globalHits.hit.nbMCHit - oldHitsNb) / ((timeNow-timeStart) > 1e-8 ? (timeNow-timeStart) : 1.0) << "Hit/s" << std::endl;
    std::cout << "Des["<<timeNow-timeStart<<"s] "<< globState.globalHits.globalHits.hit.nbDesorbed - oldDesNb
              << " : " << (double)(globState.globalHits.globalHits.hit.nbDesorbed - oldDesNb) / ((timeNow-timeStart) > 1e-8 ? (timeNow-timeStart) : 1.0) << "Des/s" << std::endl;

    simManager.KillAllSimUnits();
    // Export results
    FlowIO::WriterXML::SaveSimulationState(Settings::req_real_file, &model, globState);

    return 0;
}