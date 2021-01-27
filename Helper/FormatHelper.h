//
// Created by pbahr on 27/01/2021.
//

#ifndef MOLFLOW_PROJ_FORMATHELPER_H
#define MOLFLOW_PROJ_FORMATHELPER_H

namespace Util {
    char *formatInt(size_t v, const char *unit);

    char *formatPs(double v, const char *unit);

    char *formatSize(size_t size);

    char *formatTime(float t);
}

#endif //MOLFLOW_PROJ_FORMATHELPER_H
