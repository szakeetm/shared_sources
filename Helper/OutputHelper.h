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

#ifndef MOLFLOW_PROJ_OUTPUTHELPER_H
#define MOLFLOW_PROJ_OUTPUTHELPER_H

#ifdef DEBUG
#define DEBUG 1
#endif

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#if defined(DEBUG) && DEBUG > 3
#define DEBUG_INT(fmt, ...) printf("DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define DEBUG_PRINT(...) DEBUG_INT(__VA_ARGS__)
#elif defined(DEBUG) && DEBUG > 0
#define DEBUG_INT(fmt, ...) printf("DEBUG: %s(): " fmt, __func__, __VA_ARGS__)
#define DEBUG_PRINT(...) DEBUG_INT(__VA_ARGS__)
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
