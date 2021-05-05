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

#include "FlowMPI.h"

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

#if defined(WIN32) || defined(__APPLE__)
    setlocale(LC_ALL, "C");
#else
    std::setlocale(LC_ALL, "C");
#endif

#if defined(USE_MPI)
    MFMPI::mpi_initialize();
#endif

    Log::console_msg_master(0, "%s\n", molflowCliLogo);

    SimulationManager simManager{};
    simManager.interactiveMode = true;
    SimulationModel model{};
    GlobalSimuState globState{};


    if(Initializer::initFromArgv(argc, argv, &simManager, &model)){
#if defined(USE_MPI)
        MPI_Finalize();
#endif
        return 41;
    }

#if defined(USE_MPI)
    MFMPI::mpi_transfer_simu();
#endif

    if(Initializer::initFromFile(&simManager, &model, &globState)){
#if defined(USE_MPI)
        MPI_Finalize();
#endif
        return 42;
    }
    size_t oldHitsNb = globState.globalHits.globalHits.nbMCHit;
    size_t oldDesNb = globState.globalHits.globalHits.nbDesorbed;

    // Get autosave file name
    std::string autoSave = Initializer::getAutosaveFile();


    //simManager.ReloadHitBuffer();
    //simManager.IncreasePriority();
    Log::console_msg_master(1,"[%s] Commencing simulation for %lu seconds from %lu desorptions.\n", Util::getTimepointString().c_str(), Settings::simDuration, globState.globalHits.globalHits.nbDesorbed);

    try {
        simManager.StartSimulation();
    }
    catch (std::runtime_error& e) {
        Log::console_error("Starting simulation: %s\n",e.what());
#if defined(USE_MPI)
        MPI_Finalize();
#endif
        return 43;
    }

    Chronometer simTimer;
    simTimer.Start();
    double elapsedTime = 0.0;

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
                Log::console_error("Warning: Could not create file: %s\n", e.what());
            }

            // if there is a next des limit, handle that
            if(!Settings::desLimit.empty()) {
                model.otfParams.desorptionLimit = Settings::desLimit.front();
                Settings::desLimit.pop_front();
                simManager.ForwardOtfParams(&model.otfParams);
                endCondition = false;
                Log::console_msg_master(1, " Handling next des limit %z\n", model.otfParams.desorptionLimit);

                try {
                    ProcessSleep(1000);
                    simManager.StartSimulation();
                }
                catch (std::runtime_error& e) {
                    Log::console_error("ERROR: Starting simulation: %s\n", e.what());
                    endCondition = true;
                }
            }
        }
        else if(Settings::autoSaveDuration && (uint64_t)(elapsedTime)%Settings::autoSaveDuration==0){ // autosave every x seconds
            Log::console_msg_master(1,"[%.0lfs] Creating auto save file %s\n", elapsedTime, autoSave.c_str());
            FlowIO::WriterXML writer;
            writer.SaveSimulationState(autoSave, &model, globState);
        }
        else if(!Settings::autoSaveDuration && (uint64_t)(elapsedTime)%60==0){
            if(Settings::simDuration > 0){
                Log::console_msg_master(1,"[%.0lfs / %lf] %llu Hit : %e Hit/s\n", (double) Settings::simDuration - elapsedTime, elapsedTime, globState.globalHits.globalHits.nbMCHit - oldHitsNb, (double)(globState.globalHits.globalHits.nbMCHit-oldHitsNb)/(elapsedTime));
            }
        }

        // Check for potential time end
        if(Settings::simDuration > 0) {
            endCondition |= (elapsedTime >= (double) Settings::simDuration);
        }
    } while(!endCondition);
    simTimer.Stop();
    elapsedTime = simTimer.Elapsed();

    // Terminate simulation
    simManager.StopSimulation();
    simManager.KillAllSimUnits();
    Log::console_msg(1,"[%d][%s] Simulation finished!\n", MFMPI::world_rank, Util::getTimepointString().c_str());

#ifdef USE_MPI
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);
    Log::console_msg_master(1, "\n%-6s %-14s %-20s %-20s %-20s %-20s %-20s %-20s\n",
                            "Node#", "Time",
                            "#Hits (run)", "#Hits (total)","Hit/sec",
                            "#Des (run)", "#Des (total)","Des/sec");
    Log::console_msg_master(1, "%s\n",std::string(6+14+20+20+20+20+20+20,'-').c_str());
    if(!MFMPI::world_rank) fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    //TODO: Send output to master node
    if(elapsedTime > 1e-4) {
        // Global result print --> TODO: ()
        Log::console_msg(1,"%-6d %-14.2lf %-20zu %-20zu %-20.2lf %-20zu %-20zu %-20.2lf\n",
               MFMPI::world_rank, elapsedTime,
               globState.globalHits.globalHits.nbMCHit - oldHitsNb, globState.globalHits.globalHits.nbMCHit,
               (double) (globState.globalHits.globalHits.nbMCHit - oldHitsNb) /
               (elapsedTime),
               globState.globalHits.globalHits.nbDesorbed - oldDesNb, globState.globalHits.globalHits.nbDesorbed,
                         (double) (globState.globalHits.globalHits.nbDesorbed - oldDesNb) /
                         (elapsedTime));
    }

#if defined(USE_MPI)
    MPI_Barrier(MPI_COMM_WORLD);
    MFMPI::mpi_receive_states(model, globState);
    if(MFMPI::world_rank != 0){
        if(std::filesystem::exists(autoSave)){
            std::filesystem::remove(autoSave);
        }
    }
    // Finalize the MPI environment.
    MPI_Finalize();
    if(MFMPI::world_rank != 0){
        return 0;
    }
    Log::console_msg_master(1, "%s\n",std::string(6+14+20+20+20+20+20+20,'=').c_str());
#endif //USE_MPI

    if(elapsedTime > 1e-4) {
        Log::console_msg(1,"%-6s %-14.2lf %-20zu %-20zu %-20.2lf %-20zu %-20zu %-20.2lf\n",
                         "x", elapsedTime,
                         globState.globalHits.globalHits.nbMCHit - oldHitsNb, globState.globalHits.globalHits.nbMCHit,
                         (double) (globState.globalHits.globalHits.nbMCHit - oldHitsNb) /
                         (elapsedTime),
                         globState.globalHits.globalHits.nbDesorbed - oldDesNb, globState.globalHits.globalHits.nbDesorbed,
                         (double) (globState.globalHits.globalHits.nbDesorbed - oldDesNb) /
                         (elapsedTime));
    }

    if(MFMPI::world_rank == 0){
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
    }

    // Cleanup
    // a) tmp folder if it is not our output folder
    if(std::filesystem::path(Settings::outputPath).relative_path().compare(std::filesystem::path("tmp"))
       && std::filesystem::path(Settings::outputPath).parent_path().compare(std::filesystem::path("tmp"))){
        //Settings::tmpfile_dir
        std::filesystem::remove_all("tmp");
    }

    return 0;
}