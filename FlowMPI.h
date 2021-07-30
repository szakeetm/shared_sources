//
// Created by pascal on 4/29/21.
//

#ifndef MOLFLOW_PROJ_FLOWMPI_H
#define MOLFLOW_PROJ_FLOWMPI_H

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
    void mpi_transfer_simu();
    void mpi_receive_states(SimulationModel& model, GlobalSimuState& globState);

    template<class T>
    int MPI_Send_serialized(const T &data, int dest, int tag, MPI_Comm comm) {
        std::ostringstream state_stream;
        {
            cereal::BinaryOutputArchive archive(state_stream);
            archive(data);
        }
        const auto serialized = state_stream.str();
        //printf("[%d] Attempt to send state to %d (in %lu / %lu bytes).\n", dest, 0, state_stream.str().size(), strlen(state_stream.str().c_str()));
        return MPI_Send(serialized.data(), (int) serialized.size(), MPI::BYTE, dest, tag, comm);
    }

    template<class T>
    int MPI_Recv_serialized(T &data, int source, int tag, MPI_Comm comm, MPI_Status *status) {
        //Get number of bytes in incoming message
        MPI_Probe(source, tag, comm, status);
        int number_bytes;
        MPI_Get_count(status, MPI::BYTE, &number_bytes);
        //printf("Trying to receive %d bytes from %d.\n", number_bytes, source);
        //Allocate a buffer of appropriate size
        std::vector<char> incoming(number_bytes);

        //Receive the data
        auto ret = MPI_Recv(incoming.data(), number_bytes, MPI::BYTE, source, tag, comm, status);
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