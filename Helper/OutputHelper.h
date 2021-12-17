//
// Created by pbahr on 27/01/2021.
//

#ifndef MOLFLOW_PROJ_OUTPUTHELPER_H
#define MOLFLOW_PROJ_OUTPUTHELPER_H

#ifdef DEBUG
#define DEBUG 1
#endif

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#if defined(DEBUG) && DEBUG > 3
#define DEBUG_PRINT(fmt, ...) printf("DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#elif defined(DEBUG) && DEBUG > 0
#define DEBUG_PRINT(fmt, ...) printf("DEBUG: %s(): " fmt, __func__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) /* Don't do anything in release builds */
#endif
#else // not Windows
#if defined(DEBUG) && DEBUG > 3
#define DEBUG_INT(fmt, ...) printf("DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define DEBUG_PRINT(...) DEBUG_INT(__VA_ARGS__)
#elif defined(DEBUG) && DEBUG > 0
#define DEBUG_INT(fmt, ...) printf("DEBUG: %s(): " fmt, __func__, __VA_ARGS__)
#define DEBUG_PRINT(...) DEBUG_INT(__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) /* Don't do anything in release builds */
#endif
#endif // Windows

namespace Util {

}

#endif //MOLFLOW_PROJ_OUTPUTHELPER_H
