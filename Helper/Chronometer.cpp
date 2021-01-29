//
// Created by pbahr on 27/01/2021.
//

#include "Chronometer.h"

Chronometer::Chronometer() {
    isActive = false;
    elapsedOnStop = 0.0;
}

void Chronometer::Start() {
    isActive = true;
    startTime = clock_type::now();
}

void Chronometer::Stop(){
    isActive = false;
    stopTime = clock_type::now();
    elapsedOnStop += std::chrono::duration_cast<time_ratio>(stopTime - startTime).count();;
}

double Chronometer::Elapsed(){
    if(isActive) {
        std::chrono::duration<double> duration = std::chrono::duration_cast<time_ratio>(clock_type::now() - startTime);
        return elapsedOnStop + duration.count();
    }
    else {
        //std::chrono::duration<double> duration = std::chrono::duration_cast<time_ratio>(stopTime - startTime);
        return elapsedOnStop;
    }
}

double Chronometer::StartTime(){
    return std::chrono::time_point_cast<time_ratio>(startTime).time_since_epoch().count();
}

void Chronometer::ReInit(){
    isActive = false;
    startTime = clock_type::now();
    stopTime = startTime;
    elapsedOnStop = 0.0;
}