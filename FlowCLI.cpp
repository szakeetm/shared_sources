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
#include <mpi.h>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>
#include <bitset>


static constexpr const char* molflowCliLogo = R"(
  __  __     _  __ _             ___ _    ___
 |  \/  |___| |/ _| |_____ __ __/ __| |  |_ _|
 | |\/| / _ \ |  _| / _ \ V  V / (__| |__ | |
 |_|  |_\___/_|_| |_\___/\_/\_/ \___|____|___|
    )";

template<class T>
int MPI_Send_serialized(const T &data, int dest, int tag, MPI_Comm comm){
    std::ostringstream state_stream;
    {
        cereal::BinaryOutputArchive archive( state_stream );
        archive( data );
    }
    const auto serialized = state_stream.str();
    //printf("[%d] Attempt to send state to %d (in %lu / %lu bytes).\n", dest, 0, state_stream.str().size(), strlen(state_stream.str().c_str()));
    return MPI_Send(serialized.data(), (int)serialized.size(), MPI::BYTE, dest, tag, MPI_COMM_WORLD);
}

template<class T>
int MPI_Recv_serialized(T &data, int source, int tag, MPI_Comm comm, MPI_Status *status){
    //Get number of bytes in incoming message
    MPI_Probe(source, tag, MPI_COMM_WORLD, status);
    int number_bytes;
    MPI_Get_count(status, MPI::BYTE, &number_bytes);
    //printf("Trying to receive %d bytes from %d.\n", number_bytes, source);
    //Allocate a buffer of appropriate size
    std::vector<char> incoming(number_bytes);

    //Receive the data
    auto ret = MPI_Recv(incoming.data(), number_bytes, MPI::BYTE, source, tag, MPI_COMM_WORLD, status);
    std::stringstream state_stream;
    state_stream.write(incoming.data(), number_bytes);

    //Unpack the data
    {
        cereal::BinaryInputArchive archive(state_stream);
        archive(data);
    }

    return ret;
}

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

#if defined(WIN32) || defined(__APPLE__)
    setlocale(LC_ALL, "C");
#else
    std::setlocale(LC_ALL, "C");
#endif

    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    if(world_rank == 0)
        std::cout << molflowCliLogo << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);
    // Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);

    SimulationManager simManager{};
    simManager.interactiveMode = true;
    SimulationModel model{};
    GlobalSimuState globState{};


    if(Initializer::initFromArgv(argc, argv, &simManager, &model)){
        exit(41);
    }

    {
        // Check if all worlds have the inputfile
        for (int i = 1; i < world_size; i++) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (world_rank == 0) {
                printf("File transfer loop %d / %d.\n", i, world_size);
            }
            if (world_rank == 0) {
                bool hasFile[1] {false};
                MPI_Status status;
                // Receive at most MAX_NUMBERS from process zero
                MPI_Recv(hasFile, 1, MPI::BOOL, i, 0, MPI_COMM_WORLD,
                         &status);

                if(!hasFile[0]){
                    printf("Attempt to transfer file to %d.\n", i);

                    std::ifstream infile;
                    infile.open(Settings::inputFile, std::ios::binary);
                    std::string contents;
                    contents.assign(std::istreambuf_iterator<char>(infile),
                                    std::istreambuf_iterator<char>());
                    printf("Attempt to write file to %d.\n", i);
                    MPI_Send(contents.c_str(), contents.size()*MPI::BYTE.Get_size(), MPI::BYTE, i, 0, MPI_COMM_WORLD);
                }

            } else if(world_rank == i){
                bool hasFile[] {false/*!Settings::inputFile.empty()*/};
                MPI_Send(hasFile, 1, MPI::BOOL, 0, 0, MPI_COMM_WORLD);

                if(!hasFile[0]){
                    printf("[%d] Attempt to receive file from 0.\n", world_rank);
                    MPI_Status status;
                    // Probe for an incoming message from process zero
                    MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
                    // When probe returns, the status object has the size and other
                    // attributes of the incoming message. Get the message size
                    int number_bytes = 0;
                    MPI_Get_count(&status, MPI::BYTE, &number_bytes);
                    // Allocate a buffer to hold the incoming numbers
                    char* file_buffer = (char*)malloc(sizeof(char) * number_bytes);
                    // Now receive the message with the allocated buffer
                    printf("Trying to receive %d numbers from 0.\n",
                           number_bytes);
                    MPI_Recv(file_buffer, number_bytes, MPI::BYTE, 0, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    /*printf("1 dynamically received %d numbers from 0.\n",
                           number_amount);*/
                    std::ofstream outfile("workfile.xml");
                    outfile << file_buffer;
                    free(file_buffer);

                    Settings::inputFile = "workfile.xml";
                }
            }
        }
    }

    if(Initializer::initFromFile(argc, argv, &simManager, &model, &globState)){
        exit(42);
    }
    size_t oldHitsNb = globState.globalHits.globalHits.nbMCHit;
    size_t oldDesNb = globState.globalHits.globalHits.nbDesorbed;

    // Get autosave file name
    std::string autoSave = Initializer::getAutosaveFile();

    std::cout << "Commencing simulation for " << Settings::simDuration << " seconds from "<< globState.globalHits.globalHits.nbDesorbed << " desorptions." << std::endl;

    //simManager.ReloadHitBuffer();
    //simManager.IncreasePriority();
    try {
        simManager.StartSimulation();
    }
    catch (std::runtime_error& e) {
        std::cerr << "[ERROR] Starting simulation: " << e.what() << std::endl;
        return 1;
    }

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
            printf("--- Trans Prob: %zu -> %e\n", globState.globalHits.globalHits.nbDesorbed, globState.facetStates[1].momentResults[0].hits.nbAbsEquiv / globState.globalHits.globalHits.nbDesorbed);
            std::stringstream outFile;
            outFile << "out_" << model.otfParams.desorptionLimit <<".xml";
            try {
                std::filesystem::copy_file(Settings::inputFile, outFile.str(), std::filesystem::copy_options::overwrite_existing);
            } catch(std::filesystem::filesystem_error& e) {
                std::cout << "Could not copy file: " << e.what() << '\n';
            }

            FlowIO::WriterXML writer;
            writer.SaveSimulationState(outFile.str(), &model, globState);
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

    // Terminate simulation
    simManager.StopSimulation();
    simManager.KillAllSimUnits();

    std::cout << "Simulation finished!" << std::endl << std::flush;
    if(elapsedTime > 1e-4) {
        // Global result print --> TODO: ()
        std::cout << "[" << elapsedTime << "s] Hit " << globState.globalHits.globalHits.nbMCHit - oldHitsNb
                << " (" << globState.globalHits.globalHits.nbMCHit << ") : " << (double) (globState.globalHits.globalHits.nbMCHit - oldHitsNb) /
                              (elapsedTime) << "Hit/s\n";
        std::cout << "[" << elapsedTime << "s] Des " << globState.globalHits.globalHits.nbDesorbed - oldDesNb
                << " (" << globState.globalHits.globalHits.nbDesorbed << ") : " << (double) (globState.globalHits.globalHits.nbDesorbed - oldDesNb) /
                              (elapsedTime) << "Des/s\n";
    }

    {
        // First prepare receive structure
        MPI_Status status;
        GlobalSimuState tmpState{};
        tmpState.Resize(model);

        // Retrieve global simu state from other ranks
        // Binary Fan-in pattern
        MPI_Barrier(MPI_COMM_WORLD);
        for (int k = 0; k < std::ceil(std::log2(world_size)); k++) {
            if(world_rank==0)
                printf("\n");
            MPI_Barrier(MPI_COMM_WORLD);
            for (int i = 0; i < world_size; i++) {
                MPI_Barrier(MPI_COMM_WORLD);
                if (world_rank == i || world_rank == i + std::pow(2,k)) {
                    std::bitset<32> procTurn(i);
                    if(procTurn.test(k)) { // is kth bit set in i ?
                        continue;
                    }
                    else if(i + std::pow(2,k) < world_size) {
                        // _i <- _i + _(i+2^k)
                        if (world_rank == i) {
                            //printf("..Receive simu state loop %d from %d (.. %d).\n", i, i + (int) std::pow(2, k),(int) std::ceil(std::log2(world_size)));
                            MPI_Recv_serialized(tmpState, (int)(i + std::pow(2,k)), 0, MPI_COMM_WORLD, &status);
                            globState += tmpState;
                        }
                        else if (world_rank == i + std::pow(2,k)) {
                            //printf("..Send simu state loop %d to %d (.. %d).\n", i + (int)std::pow(2,k), i,(int)std::ceil(std::log2(world_size)));
                            MPI_Send_serialized(globState, i, 0, MPI_COMM_WORLD);
                        }
                    }
                }
            }
        }
    }

    // Finalize the MPI environment.
    MPI_Finalize();
    if(world_rank != 0){
        return 0;
    }

    printf("Total after sum up hits: %zu\n", globState.globalHits.globalHits.nbMCHit);
    if(elapsedTime > 1e-4) {
        // Global result print --> TODO: ()
        std::cout << "[" << elapsedTime << "s] Hit " << globState.globalHits.globalHits.nbMCHit - oldHitsNb
                  << " (" << globState.globalHits.globalHits.nbMCHit << ") : " << (double) (globState.globalHits.globalHits.nbMCHit - oldHitsNb) /
                                                                                  (elapsedTime) << "Hit/s\n";
        std::cout << "[" << elapsedTime << "s] Des " << globState.globalHits.globalHits.nbDesorbed - oldDesNb
                  << " (" << globState.globalHits.globalHits.nbDesorbed << ") : " << (double) (globState.globalHits.globalHits.nbDesorbed - oldDesNb) /
                                                                                     (elapsedTime) << "Des/s\n";
    }

    /*{
        // First prepare receive structure
        if (world_rank == 0) {
            MPI_Status status;
            GlobalSimuState tmpState{};
            tmpState.Resize(model);
        }

        // Retrieve global simu state from other ranks
        for (int i = 1; i < world_size; i++) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (world_rank == 0) {
                printf("Receive simu state loop %d / %d.\n", i, world_size);
            }
            if (world_rank == 0) {
                MPI_Status status;
                GlobalSimuState tmpState{};
                tmpState.Resize(model);
                MPI_Recv_serialized(tmpState, i, 0, MPI_COMM_WORLD, &status);

                printf("Adding tmpState from %d with %zu hits to %zu hits.\n", i, tmpState.globalHits.globalHits.nbMCHit, globState.globalHits.globalHits.nbMCHit);
                globState += tmpState;
            } else if(world_rank == i){
                MPI_Send_serialized(globState, 0, 0, MPI_COMM_WORLD);
            }
        }
    }*/

    /*if(world_rank == 0){
        // Export results
        char outname[256]{};
        sprintf(outname, "out%d_%s", world_rank, std::filesystem::path(Settings::outputFile).filename().c_str());
        if(Settings::inputFile != outname){
            // Copy full file description first, in case outputFile is different
            printf("[%d] attempting to copy file %s to %s\n", world_rank, std::filesystem::relative(Settings::inputFile).c_str(), outname);
            std::filesystem::copy_file( std::filesystem::relative(Settings::inputFile), outname,
                                        std::filesystem::copy_options::overwrite_existing);
        }
        FlowIO::WriterXML writer;
        writer.SaveSimulationState(outname, &model, globState);
    }*/

    // Finalize the MPI environment.
    //MPI_Finalize();
    return 0;
    exit(42);

    return 0;
}