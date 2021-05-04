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
#include <sstream>
#include <Helper/Chronometer.h>
#include <Helper/StringHelper.h>
#include <Helper/ConsoleLogger.h>

static constexpr const char* molflowCliLogo = R"(
  __  __     _  __ _
 |  \/  |___| |/ _| |_____ __ __
 | |\/| / _ \ |  _| / _ \ V  V /
 |_|  |_\___/_|_| |_\___/\_/\_/
    )";

/*static constexpr const char* molflowCliLogo = R"(
 __
 \ \      __  __     _  __ _
  \ \    |  \/  |___| |/ _| |_____ __ __
  / /    | |\/| / _ \ |  _| / _ \ V  V /
 /_/_____|_|  |_\___/_|_| |_\___/\_/\_/
   |_____|
    )";*/

int main(int argc, char** argv) {
    std::cout << molflowCliLogo << std::endl;

    SimulationManager simManager{};
    simManager.interactiveMode = false;
    SimulationModel model{};
    GlobalSimuState globState{};
    Initializer::init(argc, argv, &simManager, &model, &globState);
    size_t oldHitsNb = globState.globalHits.globalHits.nbMCHit;
    size_t oldDesNb = globState.globalHits.globalHits.nbDesorbed;

    // Get autosave file name
    std::string autoSave = Initializer::getAutosaveFile();


    //simManager.ReloadHitBuffer();
    //simManager.IncreasePriority();
    try {
        simManager.StartSimulation();
    }
    catch (std::runtime_error& e) {
        std::cerr << "[ERROR] Starting simulation: " << e.what() << std::endl;
        return 1;
    }

    printf("[%s] Commencing simulation for %lu seconds from %lu desorptions.\n", Util::getTimepointString().c_str(), Settings::simDuration, globState.globalHits.globalHits.nbDesorbed);
    Log::console_msg_master(1,"[%s] Commencing simulation for %lu seconds from %lu desorptions.\n", Util::getTimepointString().c_str(), Settings::simDuration, globState.globalHits.globalHits.nbDesorbed);

    Chronometer simTimer;
    simTimer.Start();
    double elapsedTime;

    bool endCondition = false;
    do {
        ProcessSleep(1000);

        elapsedTime = simTimer.Elapsed();
        if(model.otfParams.desorptionLimit != 0)
            endCondition = globState.globalHits.globalHits.nbDesorbed/* - oldDesNb*/ >= model.otfParams.desorptionLimit;

        if(endCondition){
            std::stringstream outFile;
            outFile << Settings::outputPath << "/" <<"desorped_" << model.otfParams.desorptionLimit << "_" <<
                 std::filesystem::path(Settings::outputFile).filename().string();
            try {
                std::filesystem::copy_file(Settings::inputFile, outFile.str(), std::filesystem::copy_options::overwrite_existing);
                FlowIO::WriterXML writer;
                writer.SaveSimulationState(outFile.str(), &model, globState);
            } catch(std::filesystem::filesystem_error& e) {
                std::cout << "Could not create file: " << e.what() << '\n';
            }

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
            FlowIO::WriterXML writer;
            writer.SaveSimulationState(autoSave, &model, globState);
        }
        else if(!Settings::autoSaveDuration && (uint64_t)(elapsedTime)%60==0){
            if(Settings::simDuration > 0){
                printf("[%.0lfs / %lf] %llu Hit : %e Hit/s\n", Settings::simDuration-elapsedTime, elapsedTime, globState.globalHits.globalHits.nbMCHit - oldHitsNb, (double)(globState.globalHits.globalHits.nbMCHit-oldHitsNb)/(elapsedTime));
            }
        }

        // Check for potential time end
        if(Settings::simDuration > 0) {
            endCondition |= elapsedTime >= Settings::simDuration;
        }
    } while(!endCondition);
    simTimer.Stop();
    elapsedTime = simTimer.Elapsed();

    // Terminate simulation
    simManager.StopSimulation();
    simManager.KillAllSimUnits();
    printf("[%s][%.0lfs] Simulation finished!\n", Util::getTimepointString().c_str(), elapsedTime);
    if(elapsedTime > 1e-4) {
        // Global result print --> TODO: ()
        std::cout << "[" << elapsedTime << "s] Hit " << globState.globalHits.globalHits.nbMCHit - oldHitsNb
                << " (" << globState.globalHits.globalHits.nbMCHit << ") : " << (double) (globState.globalHits.globalHits.nbMCHit - oldHitsNb) /
                              (elapsedTime) << "Hit/s" << std::endl;
        std::cout << "[" << elapsedTime << "s] Des " << globState.globalHits.globalHits.nbDesorbed - oldDesNb
                << " (" << globState.globalHits.globalHits.nbDesorbed << ") : " << (double) (globState.globalHits.globalHits.nbDesorbed - oldDesNb) /
                              (elapsedTime) << "Des/s" << std::endl;
    }

    // Export results
    //  a) Use existing autosave as base
    //  b) Create copy of input file(TODO: yet w/o sweep)
    // and simply update simulation results
    if(std::filesystem::exists(autoSave)){
        std::filesystem::rename(autoSave, Settings::outputPath+"/"+Settings::outputFile);
    }
    else if(Settings::inputFile != Settings::outputFile){     // TODO: Difficult to check when with complex paths
        // Copy full file description first, in case outputFile is different
        std::filesystem::copy_file(Settings::inputFile, Settings::outputPath+"/"+Settings::outputFile,
                                   std::filesystem::copy_options::overwrite_existing);
    }
    FlowIO::WriterXML writer;
    pugi::xml_document newDoc;
    newDoc.load_file((Settings::outputPath+"/"+Settings::outputFile).c_str());
    writer.SaveGeometry(newDoc, &model, false, true);
    writer.SaveSimulationState(Settings::outputPath+"/"+Settings::outputFile, &model, globState);

    // Cleanup
    // a) tmp folder if it is not our output folder
    if(std::filesystem::path(Settings::outputPath).relative_path().compare(std::filesystem::path("tmp"))
        && std::filesystem::path(Settings::outputPath).parent_path().compare(std::filesystem::path("tmp"))){
        //Settings::tmpfile_dir)
        std::cout << "NOT TMP DIR: " << Settings::outputPath << std::endl;
        std::filesystem::remove_all("tmp");
    }

    return 0;
}