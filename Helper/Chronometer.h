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

#ifndef MOLFLOW_PROJ_CHRONOMETER_H
#define MOLFLOW_PROJ_CHRONOMETER_H

#include <chrono>

using clock_type = std::chrono::steady_clock;
using time_ratio_s = std::chrono::seconds;
using time_ratio = std::chrono::nanoseconds;
using time_type = std::chrono::time_point<clock_type>;

// time class working in ns precision, returning suitable sec/msec values
class Chronometer {
public:
    Chronometer(); //! constructor for initial state: stopped, no elapsed time
    void ReInit(); //! turns off and resets the timer
    void Start(); //! start the timer and save starting time
    void Stop(); //! stops the timer and save elapsed time
    double Elapsed(); //! return elapsed time in seconds
    double ElapsedMs(); //! return elapsed time in ms
    double SecondsSinceLastAutosave();
    void UpdateLastAutoSave();
    double SecondsSinceLastStatprint();
    void UpdateLastStatprintTime();

protected:
    std::chrono::time_point<clock_type> startTime;
    std::chrono::time_point<clock_type> stopTime;
    std::chrono::time_point<clock_type> lastAutoSave;
    std::chrono::time_point<clock_type> lastStatPrint;
    double elapsedOnStop; // elapsed time in seconds
public:
    bool isActive;
};


#endif //MOLFLOW_PROJ_CHRONOMETER_H
