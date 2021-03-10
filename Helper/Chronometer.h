//
// Created by pbahr on 27/01/2021.
//

#ifndef MOLFLOW_PROJ_CHRONOMETER_H
#define MOLFLOW_PROJ_CHRONOMETER_H

#include <chrono>

using clock_type = std::chrono::steady_clock;
using time_ratio = std::chrono::seconds;
using time_type = std::chrono::time_point<clock_type>;

class Chronometer {
public:
    Chronometer();
    void ReInit();
    void Start();
    void Stop();
    double Elapsed(); // return elapsed time in seconds
    double StartTime(); // return start time in seconds

protected:
    std::chrono::time_point<clock_type> startTime;
    std::chrono::time_point<clock_type> stopTime;
    double elapsedOnStop;
public:
    bool isActive;
};


#endif //MOLFLOW_PROJ_CHRONOMETER_H
