//
// Created by Pascal Baehr on 28.04.20.
//

#include <cereal/archives/xml.hpp>
#include "SimulationManager.h"
#include "SMP.h"
#include "CLI11/CLI11.hpp"
#include "Buffer_shared.h"
#include <signal.h>
#include <Parameter.h>
#include <cereal/archives/binary.hpp>
#include "GeometrySimu.h"

class FlowFormatter : public CLI::Formatter {
public:
    std::string make_usage(const CLI::App *app, std::string name) const override {
        return "Usage: ./"
        +std::filesystem::path(name).filename().string()
        +" [options]";
    }
};

class MolflowData{
public:
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

int parse_input(std::string serializedFile){
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
    inputArchive(md.temperatures);
    inputArchive(md.moments);
    inputArchive(md.desorptionParameterIDs);

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
}
/* This flag controls termination of the main loop. */
volatile sig_atomic_t keep_going = 1;

/* The signal handler just clears the flag and re-enables itself. */
void catch_alarm (int sig)
{
    keep_going = 0;
    signal (sig, catch_alarm);
}

int main(int argc, char** argv) {
    std::cout << R"(
  __  __     _  __ _             ___ _    ___
 |  \/  |___| |/ _| |_____ __ __/ __| |  |_ _|
 | |\/| / _ \ |  _| / _ \ V  V / (__| |__ | |
 |_|  |_\___/_|_| |_\___/\_/\_/ \___|____|___|
    )" << std::endl;

    SimulationManager simManager("molflow","MFLW");
    CLI::App app{"Molflow+/Synrad+ Simulation Management"};
    app.formatter(std::make_shared<FlowFormatter>());

    // Define options
    double p = 0;
    app.add_option("-p,--procs", p, "# CPU cores");
    uint64_t simDuration = 10;
    app.add_option("-t,--time", simDuration, "Simulation duration in seconds");
    std::string req_real_file;
    app.add_option("-f,--file", req_real_file, "Require an existing file")
            ->required()
            ->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    std::cout << "Parameter value: " << p << std::endl;

    simManager.nbCores = p;
    simManager.useCPU = false;
    simManager.nbCores = 1;
    simManager.useGPU = true;
    if(simManager.InitSimUnits())
        std::cout << "Error: Initialising subprocesses: " << simManager.simHandles.size() << std::endl;


    std::cout << "Active cores: " << simManager.simHandles.size() << std::endl;

    std::cout << "Parsing file: " << req_real_file << std::endl;
    parse_input(req_real_file);
    std::cout << "Forwarding serialization: " << req_real_file << std::endl;
    try {
        simManager.LoadInput(req_real_file);
    }
    catch (std::runtime_error& e) {
        std::cout << "ERROR: Loading Input: " << e.what() << std::endl;
        exit(0);
    }
    //simManager.ReloadHitBuffer();
    try {
        simManager.StartSimulation();
    }
    catch (std::runtime_error& e) {
        std::cout << "ERROR: Starting simulation: " << e.what() << std::endl;
        exit(0);
    }

#if not defined(WIN32)
    /* Establish a handler for SIGALRM signals. */
    signal (SIGALRM, catch_alarm);

    alarm(simDuration);
    std::cout << "Commencing simulation for " << simDuration << " seconds."<<std::endl;

    std::cout << "." << std::flush << '\b';
    do {
        usleep(1000000);
        std::cout << "." << std::flush;
        usleep(1000000);
        std::cout << "\b.." << std::flush;
        usleep(1000000);
        std::cout << "\b\b..." << std::flush;
        usleep(1000000);
        std::cout << "\b\b\b...." << std::flush;
        usleep(1000000);
        std::cout << "\b\b\b\b....." << std::flush<< "\b\b\b\b\b";
    } while(keep_going);
    std::cout << "Simulation finished!" << std::endl << std::flush;
#endif
    simManager.StopSimulation();
    simManager.KillAllSimUnits();
    // Export results
    // GlobalHitBuffer *gHits = (GlobalHitBuffer *)simManager.GetLockedHitBuffer();
    //simManager.UnlockHitBuffer();

    return 0;
}