//
// Created by pascal on 4/30/21.
//

#ifndef MOLFLOW_PROJ_CONSOLELOGGER_H
#define MOLFLOW_PROJ_CONSOLELOGGER_H

#define verbosity 0

namespace Log {

    template<typename... P>
    void console_error(const char * message, const P&... fmt){
        fprintf(stderr, message, fmt...);
    }

    template<typename... P>
    void console_msg(int level, const char * message, const P&... fmt){
        if (verbosity >= level) {
            printf(message, fmt...);
        }
    }

    template<typename... P>
    void console_msg_master(int level, const char * message, const P&... fmt){
        if (1 && verbosity >= level) {

            printf(message, fmt...);
        }
    }
}
#endif //MOLFLOW_PROJ_CONSOLELOGGER_H
