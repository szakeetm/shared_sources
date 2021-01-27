//
// Created by pbahr on 27/01/2021.
//

#ifndef MOLFLOW_PROJ_CHRONOMETER_H
#define MOLFLOW_PROJ_CHRONOMETER_H

#include <chrono>

using clock_type = std::chrono::steady_clock;
using time_ratio = std::chrono::seconds;

class Chronometer {
public:
    Chronometer();
    void ReInit();
    void Start();
    void Stop();
    double Elapsed();

protected:
    std::chrono::time_point<clock_type> startTime;
    std::chrono::time_point<clock_type> stopTime;
    bool isActive;
};


#endif //MOLFLOW_PROJ_CHRONOMETER_H
