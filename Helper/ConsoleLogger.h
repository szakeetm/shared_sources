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

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

#ifndef MOLFLOW_PROJ_CONSOLELOGGER_H
#define MOLFLOW_PROJ_CONSOLELOGGER_H

#include "AppSettings.h"
#include "FlowMPI.h"
#include <fmt/core.h>

extern int Settings::verbosity;
extern int MFMPI::world_rank;

namespace Log {

    template<typename... P>
    void console_error(const char * message, const P&... fmt){
        fmt::print(stderr, message, fmt...);
    }

    template<typename... P>
    void console_msg(int level, const char * message, const P&... fmt){
        if (Settings::verbosity >= level) {
            if(Settings::outputLevel) printf("%*c", Settings::outputLevel, ' ');
            fmt::print(message, fmt...);
            fflush(stdout);
        }
    }

    template<typename... P>
    void console_msg_master(int level, const char * message, const P&... fmt){
        if (!MFMPI::world_rank && Settings::verbosity >= level) {
            if(Settings::outputLevel) printf("%*c", Settings::outputLevel, ' ');
            fmt::print(message, fmt...);
            fflush(stdout);
        }
    }

    // First output message, then increase front spacing
    template<typename... P>
    void console_header(int level, const char * message, const P&... fmt){
        console_msg_master(level, message, fmt...);
        Settings::outputLevel++;
    }

    // First decrease front spacing, then output message
    template<typename... P>
    void console_footer(int level, const char * message, const P&... fmt){
        Settings::outputLevel--;
        console_msg_master(level, message, fmt...);
    }
}

#endif //MOLFLOW_PROJ_CONSOLELOGGER_H

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif