//
// Created by pbahr on 5/5/21.
//

#include <Initializer.h>
#include <fstream>
#include <filesystem>
#include <Helper/ConsoleLogger.h>
#include <ziplib/ZipFile.h>
#include "FlowMPI.h"
#include "GeometrySimu.h"

namespace MFMPI {
    // globals
    int world_rank{0};
    int world_size{1};

#if defined(USE_MPI)
    void mpi_initialize() {
        // Initialize the MPI environment
        MPI_Init(nullptr, nullptr);

        // Get the number of processes
        MPI_Comm_size(MPI_COMM_WORLD, &MFMPI::world_size);
        // Get the rank of the process
        MPI_Comm_rank(MPI_COMM_WORLD, &MFMPI::world_rank);

        // Get the name of the processor
        char processor_name[MPI_MAX_PROCESSOR_NAME];
        int name_len;
        MPI_Get_processor_name(processor_name, &name_len);

        MPI_Barrier(MPI_COMM_WORLD);
    };

    void mpi_transfer_simu() {
        // Check if all worlds have the inputfile
        // forward if not, maybe change to broadcast
        for (int i = 1; i < MFMPI::world_size; i++) {
            MPI_Barrier(MPI_COMM_WORLD);
            Log::console_msg_master(5,"File transfer loop %d / %d.\n", i, MFMPI::world_size);
            if (MFMPI::world_rank == 0) {
                bool hasFile[1]{false};
                MPI_Status status;
                // Receive at most MAX_NUMBERS from process zero
                MPI_Recv(hasFile, 1, MPI::BOOL, i, 0, MPI_COMM_WORLD,
                         &status);

                if (!hasFile[0]) {
                    Log::console_msg(4, "Attempt to transfer file %s to node %d.\n", Settings::inputFile.c_str(), i);

                    std::ifstream infile;
                    infile.open(Settings::inputFile, std::ios::binary);
                    std::string contents;
                    contents.assign(std::istreambuf_iterator<char>(infile),
                                    std::istreambuf_iterator<char>());
                    Log::console_msg(4,"Attempt to write file to node %d.\n", i);
                    MPI_Send(contents.c_str(), MPI::BYTE.Get_size() * contents.size(), MPI::BYTE, i, 0, MPI_COMM_WORLD);
                }

            } else if (MFMPI::world_rank == i) {
                bool hasFile[]{false/*!Settings::inputFile.empty()*/};
                MPI_Send(hasFile, 1, MPI::BOOL, 0, 0, MPI_COMM_WORLD);

                if (!hasFile[0]) {
                    Log::console_msg(4,"[%d] Attempt to receive file from master.\n", MFMPI::world_rank);
                    MPI_Status status;
                    // Probe for an incoming message from process zero
                    MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
                    // When probe returns, the status object has the size and other
                    // attributes of the incoming message. Get the message size
                    int number_bytes = 0;
                    MPI_Get_count(&status, MPI::BYTE, &number_bytes);
                    // Allocate a buffer to hold the incoming numbers
                    char *file_buffer = (char *) malloc(sizeof(char) * number_bytes);
                    // Now receive the message with the allocated buffer
                    Log::console_msg(4,"Trying to receive %d numbers from master.\n",
                           number_bytes);
                    MPI_Recv(file_buffer, number_bytes, MPI::BYTE, 0, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    // use fallback dir
                    Settings::outputPath = "tmp"+std::to_string(MFMPI::world_rank)+"/";
                    try {
                        std::filesystem::create_directory(Settings::outputPath);
                    }
                    catch (std::exception& e){
                        Settings::outputPath = "./";
                        Log::console_error("Couldn't create fallback directory [ %s ], falling back to binary folder instead for output files\n", Settings::outputPath.c_str());
                    }

                    auto outputName = Settings::outputPath+"workfile"+std::filesystem::path(Settings::inputFile).extension().string();
                    std::ofstream outfile(outputName);
                    outfile.write(file_buffer, number_bytes);
                    outfile.close();
                    free(file_buffer);

                    Settings::inputFile = outputName;
                }
            }
        }
    }

    void mpi_receive_states(SimulationModel& model, GlobalSimuState& globState) {
        // First prepare receive structure
        MPI_Status status;
        GlobalSimuState tmpState{};
        tmpState.Resize(model);

        // Retrieve global simu state from other ranks
        // Binary Fan-in pattern
        MPI_Barrier(MPI_COMM_WORLD);
        for (int k = 0; k < std::ceil(std::log2(MFMPI::world_size)); k++) {
            MPI_Barrier(MPI_COMM_WORLD);
            for (int i = 0; i < MFMPI::world_size; i++) {
                MPI_Barrier(MPI_COMM_WORLD);
                if (MFMPI::world_rank == i || MFMPI::world_rank == i + std::pow(2, k)) {
                    std::bitset<32> procTurn(i);
                    if (procTurn.test(k)) { // is kth bit set in i ?
                        continue;
                    } else if (i + std::pow(2, k) < MFMPI::world_size) {
                        // _i <- _i + _(i+2^k)
                        if (MFMPI::world_rank == i) {
                            //printf("..Receive simu state loop %d from %d (.. %d).\n", i, i + (int) std::pow(2, k),(int) std::ceil(std::log2(world_size)));
                            MPI_Recv_serialized(tmpState,(int) (i + std::pow(2, k)), 0, MPI_COMM_WORLD, &status);
                            globState += tmpState;
                        } else if (MFMPI::world_rank == i + std::pow(2, k)) {
                            //printf("..Send simu state loop %d to %d (.. %d).\n", i + (int)std::pow(2,k), i,(int)std::ceil(std::log2(world_size)));
                            MPI_Send_serialized(globState, i,0, MPI_COMM_WORLD);
                        }
                    }
                }
            }
        }
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

#endif // USE_MPI
};