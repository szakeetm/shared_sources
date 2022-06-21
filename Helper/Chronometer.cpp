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