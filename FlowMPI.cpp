/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/

#include <Initializer.h>
#include <Helper/ConsoleLogger.h>
#include <ZipLib/ZipFile.h>
#include "FlowMPI.h"
//#include "Simulation/SynradSimGeom.h"
#include "SettingsIO.h"


#include <fstream>
#include <filesystem>
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

        int number_bytes = 0;
        std::string contents;

        MPI_Barrier(MPI_COMM_WORLD);
        if(MFMPI::world_rank == 0){
            std::ifstream infile;
            infile.open(SettingsIO::inputFile, std::ios::binary);
            //std::string contents;
            contents.assign(std::istreambuf_iterator<char>(infile),
                            std::istreambuf_iterator<char>());
            int nByte = 0;
            MPI_Type_size(MPI_BYTE, &nByte);
            //MPI_Send(contents.c_str(), nByte * contents.size(), MPI_BYTE, i, 0, MPI_COMM_WORLD);
            number_bytes = nByte * contents.size();
            Log::console_msg(4,"Sharing file size with nodes {}.\n", number_bytes);
            MPI_Bcast(&number_bytes, 1, MPI_INT, 0, MPI_COMM_WORLD);
        }
        else {
            MPI_Bcast(&number_bytes, 1, MPI_INT, 0, MPI_COMM_WORLD);
        }

        if(number_bytes == 0) {
            Log::console_msg(2,"No bytes received for file transfer!\n");
            return;
        }

        Log::console_msg_master(4,"Attempt to write file to nodes.\n");
        char *file_buffer;
        if(MFMPI::world_rank != 0){
            file_buffer = (char *) malloc(number_bytes);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        if(MFMPI::world_rank == 0){
            MPI_Bcast((char*)contents.c_str(), number_bytes, MPI_BYTE, 0, MPI_COMM_WORLD);
        }
        else {
            MPI_Bcast(file_buffer, number_bytes, MPI_BYTE, 0, MPI_COMM_WORLD);
        }

        //MPI_Bcast(data, num_elements, MPI_INT, 0, MPI_COMM_WORLD);
        if(MFMPI::world_rank != 0){
            free(file_buffer);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        return;

        for (int i = 1; i < MFMPI::world_size; i++) {
            MPI_Barrier(MPI_COMM_WORLD);
            Log::console_msg_master(5,"File transfer loop {} / {}.\n", i, MFMPI::world_size);
            if (MFMPI::world_rank == 0) {
                bool hasFile[1]{false};
                MPI_Status status;
                // Receive at most MAX_NUMBERS from process zero
                MPI_Recv(hasFile, 1, MPI_CXX_BOOL, i, 0, MPI_COMM_WORLD,
                         &status);

                if (!hasFile[0]) {
                    Log::console_msg(4, "Attempt to transfer file {} to node {}.\n", SettingsIO::inputFile.c_str(), i);

                    std::ifstream infile;
                    infile.open(SettingsIO::inputFile, std::ios::binary);
                    std::string contents;
                    contents.assign(std::istreambuf_iterator<char>(infile),
                                    std::istreambuf_iterator<char>());
                    Log::console_msg(4,"Attempt to write file to node {}.\n", i);
                    int nByte = 0;
                    MPI_Type_size(MPI_BYTE, &nByte);
                    MPI_Send(contents.c_str(), nByte * contents.size(), MPI_BYTE, i, 0, MPI_COMM_WORLD);
                }

            } else if (MFMPI::world_rank == i) {
                bool hasFile[]{false/*!SettingsIO::inputFile.empty()*/};
                MPI_Send(hasFile, 1, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD);

                if (!hasFile[0]) {
                    Log::console_msg(4,"[{}] Attempt to receive file from master.\n", MFMPI::world_rank);
                    MPI_Status status;
                    // Probe for an incoming message from process zero
                    MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
                    // When probe returns, the status object has the size and other
                    // attributes of the incoming message. Get the message size
                    int number_bytes = 0;
                    MPI_Get_count(&status, MPI_BYTE, &number_bytes);
                    // Allocate a buffer to hold the incoming numbers
                    char *file_buffer = (char *) malloc(sizeof(char) * number_bytes);
                    // Now receive the message with the allocated buffer
                    Log::console_msg(4,"Trying to receive {} numbers from master.\n",
                           number_bytes);
                    MPI_Recv(file_buffer, number_bytes, MPI_BYTE, 0, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    // use fallback dir
                    SettingsIO::outputPath = "tmp"+std::to_string(MFMPI::world_rank)+"/";
                    try {
                        std::filesystem::create_directory(SettingsIO::outputPath);
                    }
                    catch (const std::exception &e){
                        SettingsIO::outputPath = "./";
                        Log::console_error("Couldn't create fallback directory [ {} ], falling back to binary folder instead for output files\n", SettingsIO::outputPath.c_str());
                    }

                    auto outputName = SettingsIO::outputPath+"workfile"+std::filesystem::path(SettingsIO::inputFile).extension().string();
                    std::ofstream outfile(outputName);
                    outfile.write(file_buffer, number_bytes);
                    outfile.close();
                    free(file_buffer);

                    SettingsIO::inputFile = outputName;
                }
            }
        }
    }

    void mpi_receive_states(std::shared_ptr<SimulationModel>& model, GlobalSimuState& globState) {
        // First prepare receive structure
        MPI_Status status;
        GlobalSimuState tmpState{};
        tmpState.Resize(*model);

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