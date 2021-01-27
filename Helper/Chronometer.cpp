//
// Created by pbahr on 27/01/2021.
//

#include "Chronometer.h"

Chronometer::Chronometer() {
    isActive = false;
}

void Chronometer::Start() {
    isActive = true;
    startTime = clock_type::now();
}

void Chronometer::Stop(){
    isActive = false;
    stopTime = clock_type::now();
}

double Chronometer::Elapsed(){
    if(isActive) {
        std::chrono::duration<double> duration = std::chrono::duration_cast<time_ratio>(clock_type::now() - startTime);
        return duration.count();
    }
    else {
        std::chrono::duration<double> duration = std::chrono::duration_cast<time_ratio>(stopTime - startTime);
        return duration.count();
    }
}

void Chronometer::ReInit(){
    isActive = false;
    startTime = clock_type::now();
    stopTime = startTime;
}