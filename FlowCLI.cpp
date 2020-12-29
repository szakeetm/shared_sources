//
// Created by Pascal Baehr on 28.04.20.
//

#include <cereal/archives/xml.hpp>
#include "SimulationManager.h"
#include "SMP.h"
#include "Buffer_shared.h"
#include <signal.h>
#include <fstream>
#include <Parameter.h>
#include <cereal/archives/binary.hpp>
#include <LoaderXML.h>
#include <WriterXML.h>
#include "GeometrySimu.h"
#include "Initializer.h"
#include "Helper/MathTools.h"
#include <omp.h>

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

/*int parse_input(std::string serializedFile){
    std::ifstream inputFile(serializedFile);
    inputFile.seekg(0, std::ios::end);
    size_t size = inputFile.tellg();
    std::string buffer(size, ' ');
    inputFile.seekg(0);
    inputFile.read(&buffer[0], size);
    
    std::stringstream inputStream;
    inputStream << buffer;
    cereal::BinaryInputArchive inputArchive(inputStream);

    MolflowData md;
    //Worker params
    inputArchive(md.wp);
    inputArchive(md.ontheflyParams);
    inputArchive(md.CDFs);
    inputArchive(md.IDs);
    inputArchive(md.parameters);
    //inputArchive(md.temperatures);
    inputArchive(md.moments);
    //inputArchive(md.desorptionParameterIDs);

    md.structures.resize(md.sh.nbSuper); //Create structures
    //Facets
    for (size_t i = 0; i < md.sh.nbFacet; i++) { //Necessary because facets is not (yet) a vector in the interface
        SubprocessFacet f;
        inputArchive(
                f.sh,
                f.indices,
                f.vertices2,
                f.outgassingMap,
                f.angleMap.pdf,
                f.textureCellIncrements
        );

        if (f.sh.superIdx == -1) { //Facet in all structures
            for (auto& s : md.structures) {
                s.facets.push_back(f);
            }
        }
        else {
            md.structures[f.sh.superIdx].facets.push_back(f); //Assign to structure
        }
    }

    std::cout << "Parsed geometry " << md.sh.name << std::endl;
    std::cout << " -> " << md.sh.nbFacet <<" facets" << std::endl;
    std::cout << " -> " << md.sh.nbVertex <<" vertices" << std::endl;

    return 0;
}*/
/* This flag controls termination of the main loop. */
volatile sig_atomic_t keep_going = 1;

/* The signal handler just clears the flag and re-enables itself. */
void catch_alarm (int sig)
{
    keep_going = 0;
    signal (sig, catch_alarm);
}

int main(int argc, char** argv) {
    std::cout << molflowCliLogo << std::endl;

    SimulationManager simManager("molflow","MFLW");
    SimulationModel model{};
    GlobalSimuState globState{};
    Initializer::init(argc, argv, &simManager, &model, &globState);
    size_t oldHitsNb = globState.globalHits.globalHits.hit.nbMCHit;
    size_t oldDesNb = globState.globalHits.globalHits.hit.nbDesorbed;

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

    std::cout << "Commencing simulation for " << Settings::simDuration << " seconds." << std::endl;
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
        //printf("Des: %zu - %zu > %llu\n", globState.globalHits.globalHits.hit.nbDesorbed, oldDesNb, Settings::desLimit);
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
                std::cout << "Could not copy sandbox/abc: " << e.what() << '\n';
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
    } while(timeNow < timeEnd && !endCondition);
    std::cout << "Simulation finished!" << std::endl << std::flush;

    // Stop and copy results
    simManager.StopSimulation();
    /*for(const auto& subHandle : simManager.simUnits){
        const size_t sub_pid = 0;
        //GlobalSimuState* localState = simManager.FetchResults(sub_pid);
        const GlobalSimuState& localState = *subHandle->globState;
        std::cout << "["<<sub_pid<<"] "<< globState.globalHits.globalHits.hit.nbMCHit + localState.globalHits.globalHits.hit.nbMCHit
            << " : " << globState.globalHits.globalHits.hit.nbMCHit << " += " << localState.globalHits.globalHits.hit.nbMCHit <<std::endl;
        globState.globalHits.globalHits += localState.globalHits.globalHits;
        globState.globalHistograms += localState.globalHistograms;
        globState.facetStates += localState.facetStates;
        //delete localState;
    }*/
    std::cout << "Hit["<<timeNow-timeStart<<"s] "<< globState.globalHits.globalHits.hit.nbMCHit - oldHitsNb
              << " : " << (double)(globState.globalHits.globalHits.hit.nbMCHit - oldHitsNb) / ((timeNow-timeStart) > 1e-8 ? (timeNow-timeStart) : 1.0) << std::endl;
    std::cout << "Des["<<timeNow-timeStart<<"s] "<< globState.globalHits.globalHits.hit.nbDesorbed - oldDesNb
              << " : " << (double)(globState.globalHits.globalHits.hit.nbDesorbed - oldDesNb) / ((timeNow-timeStart) > 1e-8 ? (timeNow-timeStart) : 1.0) << std::endl;

    simManager.KillAllSimUnits();
    // Export results
    //BYTE *buffer = simManager.GetLockedHitBuffer();
    //FlowIO::WriterXML writer;
    //writer.SaveSimulationState(Settings::req_real_file, &model, buffer);
    FlowIO::WriterXML::SaveSimulationState(Settings::req_real_file, &model, globState);
    //simManager.UnlockHitBuffer();

    return 0;
}