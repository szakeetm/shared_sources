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
#include <Helper/Chronometer.h>

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

    SimulationManager simManager();
    SimulationModel model{};
    GlobalSimuState globState{};
    Initializer::init(argc, argv, &simManager, &model, &globState);
    size_t oldHitsNb = globState.globalHits.globalHits.hit.nbMCHit;
    size_t oldDesNb = globState.globalHits.globalHits.hit.nbDesorbed;

    // Get autosave file name
    std::string autoSave = Initializer::getAutosaveFile();

    //simManager.ReloadHitBuffer();
    try {
        simManager.StartSimulation();
    }
    catch (std::runtime_error& e) {
        std::cerr << "[ERROR] Starting simulation: " << e.what() << std::endl;
        return 1;
    }



    std::cout << "Commencing simulation for " << Settings::simDuration << " seconds from "<< globState.globalHits.globalHits.hit.nbDesorbed << " desorptions." << std::endl;

    Chronometer simTimer;
    simTimer.Start();
    double elapsedTime;

    bool endCondition = false;
    do {
        ProcessSleep(1000);

        elapsedTime = simTimer.Elapsed();
        if(model.otfParams.desorptionLimit != 0)
            endCondition = globState.globalHits.globalHits.hit.nbDesorbed/* - oldDesNb*/ >= model.otfParams.desorptionLimit;

        if(endCondition){
            printf("--- Trans Prob: %lu -> %e\n", globState.globalHits.globalHits.hit.nbDesorbed, globState.facetStates[1].momentResults[0].hits.hit.nbAbsEquiv / globState.globalHits.globalHits.hit.nbDesorbed);
            std::stringstream outFile;
            outFile << "out_" << model.otfParams.desorptionLimit <<".xml";
            try {
                std::filesystem::copy_file(Settings::inputFile, outFile.str(), std::filesystem::copy_options::overwrite_existing);
            } catch(std::filesystem::filesystem_error& e) {
                std::cout << "Could not copy file: " << e.what() << '\n';
            }

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
        else if(Settings::autoSaveDuration && (uint64_t)(elapsedTime)%Settings::autoSaveDuration==0){ // autosave every x seconds
            printf("[%.0lfs] Creating auto save file %s\n", elapsedTime, autoSave.c_str());
            FlowIO::WriterXML::SaveSimulationState(autoSave, &model, globState);
        }
        else if(!Settings::autoSaveDuration && (uint64_t)(elapsedTime)%60==0){
            if(Settings::simDuration > 0){
                printf("[%.0lfs / %lf] %llu Hit : %e Hit/s\n", Settings::simDuration-elapsedTime, elapsedTime, globState.globalHits.globalHits.hit.nbMCHit - oldHitsNb, (double)(globState.globalHits.globalHits.hit.nbMCHit-oldHitsNb)/(elapsedTime));
            }
        }

        // Check for potential time end
        if(Settings::simDuration > 0) {
            endCondition |= elapsedTime >= Settings::simDuration;
        }
    } while(!endCondition);
    simTimer.Stop();

    // Terminate simulation
    simManager.StopSimulation();
    simManager.KillAllSimUnits();

    std::cout << "Simulation finished!" << std::endl << std::flush;
    if(elapsedTime > 1e-4) {
        // Global result print --> TODO: ()
        std::cout << "[" << elapsedTime << "s] Hit " << globState.globalHits.globalHits.hit.nbMCHit - oldHitsNb
                  << " : " << (double) (globState.globalHits.globalHits.hit.nbMCHit - oldHitsNb) /
                              (elapsedTime) << "Hit/s" << std::endl;
        std::cout << "[" << elapsedTime << "s] Des " << globState.globalHits.globalHits.hit.nbDesorbed - oldDesNb
                  << " : " << (double) (globState.globalHits.globalHits.hit.nbDesorbed - oldDesNb) /
                              (elapsedTime) << "Des/s" << std::endl;
    }

    // Export results
    if(Settings::inputFile != Settings::outputFile){
        // Copy full file description first, in case outputFile is different
        std::filesystem::copy_file(Settings::inputFile, Settings::outputFile,
                                   std::filesystem::copy_options::overwrite_existing);
    }
    FlowIO::WriterXML::SaveSimulationState(Settings::outputFile, &model, globState);

    return 0;
}