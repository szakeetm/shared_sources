//
// Created by pascal on 4/30/21.
//

#ifndef MOLFLOW_PROJ_CONSOLELOGGER_H
#define MOLFLOW_PROJ_CONSOLELOGGER_H

#define verbosity 3

namespace Log {
    bool master = true;
    template<typename... P>
    void console_msg(int level, const char * message, const P&... fmt){
        if (verbosity >= level) {
            printf(message, fmt...);
        }
    }

    template<typename... P>
    void console_msg_master(int level, const char * message, const P&... fmt){
        if (master && verbosity >= level) {

            printf(message, fmt...);
        }
    }
}
#endif //MOLFLOW_PROJ_CONSOLELOGGER_H
