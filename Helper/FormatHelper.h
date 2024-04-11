

#ifndef MOLFLOW_PROJ_FORMATHELPER_H
#define MOLFLOW_PROJ_FORMATHELPER_H

#include <cstddef>
namespace Util {
    char *formatInt(size_t v, const char *unit);

    char *formatPs(double v, const char *unit);

    char *formatSize(size_t size);

    char *formatTime(float t);
    char *formatTime(double t);
}

#endif //MOLFLOW_PROJ_FORMATHELPER_H
