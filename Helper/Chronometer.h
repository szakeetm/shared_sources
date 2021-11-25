//
// Created by pbahr on 27/01/2021.
//

#ifndef MOLFLOW_PROJ_CHRONOMETER_H
#define MOLFLOW_PROJ_CHRONOMETER_H

#include <chrono>
#include <tuple> // benchmark

using clock_type = std::chrono::steady_clock;
using time_ratio_s = std::chrono::seconds;
using time_ratio = std::chrono::nanoseconds;
using time_type = std::chrono::time_point<clock_type>;

// time class working in ns precision, returning suitable sec/msec values
class Chronometer {
public:
    Chronometer(bool start = false); //! constructor for initial state: stopped, no elapsed time
    void Start(); //! start the timer and save starting time
    void Stop(); //! stops the timer and save elapsed time
    void ReInit(); //! turns off and resets the timer
    void ReStart(); //! stays or turns on and resets the timer

    double Elapsed(); //! return elapsed time in seconds
    double ElapsedMs(); //! return elapsed time in ms

protected:
    std::chrono::time_point<clock_type> startTime;
    std::chrono::time_point<clock_type> stopTime;
    double elapsedOnStop; // elapsed time in seconds
public:
    bool isActive;
};

struct CummulativeBenchmark{
    std::pair<double, size_t> pair{};

    void AddTime(double time){
        pair.first += time;
        ++pair.second;
    }

    double GetRatio(){
        if(pair.second > 0)
            return pair.first / static_cast<double>(pair.second);
        else
            return 0.0;
    }
};

#endif //MOLFLOW_PROJ_CHRONOMETER_H
