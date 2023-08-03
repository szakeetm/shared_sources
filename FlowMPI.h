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

#ifndef MOLFLOW_PROJ_FLOWMPI_H
#define MOLFLOW_PROJ_FLOWMPI_H

#include "SettingsIO.h"

#if defined(USE_MPI)
#include <mpi.h>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>
#include <bitset>
#endif

class SimulationModel;
class GlobalSimuState;

namespace MFMPI {
    // globals
    extern int world_rank;
    extern int world_size;

#if defined(USE_MPI)
    void mpi_initialize();
    void mpi_transfer_simu(SettingsIO::CLIArguments& parsedArgs);
    void mpi_receive_states(std::shared_ptr<SimulationModel> model, GlobalSimuState& globalState);

    template<class T>
    int MPI_Send_serialized(const T &data, int dest, int tag, MPI_Comm comm) {
        std::ostringstream state_stream;
        {
            cereal::BinaryOutputArchive archive(state_stream);
            archive(data);
        }
        const auto serialized = state_stream.str();
        //printf("[%d] Attempt to send state to %d (in %lu / %lu bytes).\n", dest, 0, state_stream.str().size(), strlen(state_stream.str().c_str()));
        return MPI_Send(serialized.data(), (int) serialized.size(), MPI_BYTE, dest, tag, comm);
    }

    template<class T>
    int MPI_Recv_serialized(T &data, int source, int tag, MPI_Comm comm, MPI_Status *status) {
        //Get number of bytes in incoming message
        MPI_Probe(source, tag, comm, status);
        int number_bytes;
        MPI_Get_count(status, MPI_BYTE, &number_bytes);
        //printf("Trying to receive %d bytes from %d.\n", number_bytes, source);
        //Allocate a buffer of appropriate size
        std::vector<char> incoming(number_bytes);

        //Receive the data
        auto ret = MPI_Recv(incoming.data(), number_bytes, MPI_BYTE, source, tag, comm, status);
        std::stringstream state_stream;
        state_stream.write(incoming.data(), number_bytes);

        //Unpack the data
        {
            cereal::BinaryInputArchive archive(state_stream);
            archive(data);
        }

        return ret;
    }

#endif // USE_MPI
};
#endif //MOLFLOW_PROJ_FLOWMPI_H