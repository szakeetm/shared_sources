

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

extern int AppSettings::verbosity;
extern int MFMPI::world_rank;

namespace Log {

    template<typename... P>
    void console_error(const char * message, const P&... fmt){
        fmt::print(stderr, message, fmt...);
    }

    template<typename... P>
    void console_msg(int level, const char * message, const P&... fmt){
        if (AppSettings::verbosity >= level) {
            if(AppSettings::outputLevel) printf("%*c", AppSettings::outputLevel, ' ');
            fmt::print(message, fmt...);
            fflush(stdout);
        }
    }

    template<typename... P>
    void console_msg_master(int level, const char * message, const P&... fmt){
        if (!MFMPI::world_rank && AppSettings::verbosity >= level) {
            if(AppSettings::outputLevel) printf("%*c", AppSettings::outputLevel, ' ');
            fmt::print(message, fmt...);
            fflush(stdout);
        }
    }

    // First output message, then increase front spacing
    template<typename... P>
    void console_header(int level, const char * message, const P&... fmt){
        console_msg_master(level, message, fmt...);
        AppSettings::outputLevel++;
    }

    // First decrease front spacing, then output message
    template<typename... P>
    void console_footer(int level, const char * message, const P&... fmt){
        AppSettings::outputLevel--;
        console_msg_master(level, message, fmt...);
    }
}

#endif //MOLFLOW_PROJ_CONSOLELOGGER_H

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif