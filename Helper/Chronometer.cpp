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

void Chronometer::Stop() {
    // Check for "real stop": isActive [true->false]
    if(isActive) {
        isActive = false;
        stopTime = clock_type::now();
        std::chrono::duration<double> duration = std::chrono::duration_cast<time_ratio>(
                stopTime - startTime); // gets converted to sec with ns precision like this
        elapsedOnStop += duration.count();
    }
}

double Chronometer::Elapsed() {
    if (isActive) {
        std::chrono::duration<double> duration = std::chrono::duration_cast<time_ratio>(clock_type::now() - startTime);
        return (elapsedOnStop + duration.count()); // ns to s
    } else {
        return elapsedOnStop;
    }
}

double Chronometer::ElapsedMs() {
    if (isActive) {
        std::chrono::duration<double> duration = std::chrono::duration_cast<time_ratio>(clock_type::now() - startTime);
        return (elapsedOnStop + duration.count()) * 1000.0; // ns to ms
    } else {
        //std::chrono::duration<double> duration = std::chrono::duration_cast<time_ratio>(stopTime - startTime);
        return elapsedOnStop * 1000.0;
    }
}

void Chronometer::ReInit() {
    isActive = false;
    startTime = clock_type::now();
    stopTime = startTime;
    elapsedOnStop = 0.0;
}