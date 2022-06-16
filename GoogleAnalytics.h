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

#ifndef MOLFLOW_PROJ_GOOGLEANALYTICS_H
#define MOLFLOW_PROJ_GOOGLEANALYTICS_H

#define PUBLIC_WEBSITE "https://molflow.web.cern.ch/"
#define DOWNLOAD_PAGE "https://molflow.web.cern.ch/content/downloads"
#define GA_PROJECT_ID "UA-86802533-2"

#if defined(MOLFLOW)
#define REMOTE_FEED "https://gitlab.cern.ch/molflow_synrad/molflow-updater/-/raw/master/autoupdate_molflow.xml"
#define BRANCH_NAME "molflow_public"
#elif defined(SYNRAD)
#define REMOTE_FEED "https://gitlab.cern.ch/molflow_synrad/molflow-updater/-/raw/master/autoupdate_synrad.xml"
#define BRANCH_NAME "synrad_public"
#endif //BRANCH_NAME

#if defined(__LINUX_FEDORA) // set by cmake
#define BRANCH_OS_SUFFIX "_linux_fedora"
#define OS_ID "fedora"
#elif defined(__LINUX_DEBIAN) // set by cmake
#define BRANCH_OS_SUFFIX "_linux_debian"
#define OS_ID "debian"
#elif defined(WIN32) || defined(_WIN32) || (defined(__CYGWIN__) && defined(__x86_64__)) || defined(__MINGW32__)
#define BRANCH_OS_SUFFIX "_win"
#define OS_ID "win_x86"
#elif defined(WIN64) || defined(_WIN64) || (defined(__CYGWIN__) && defined(__X86__)) || defined(__MINGW64__)
#define BRANCH_OS_SUFFIX "_win"
#define OS_ID "win_x64"
#elif (defined(__MACOSX__) || defined(__APPLE__)) && defined(__ARM_ARCH)
#define BRANCH_OS_SUFFIX "_mac_arm"
#define OS_ID "mac_arm"
#elif defined(__MACOSX__) || defined(__APPLE__)
#define BRANCH_OS_SUFFIX "_mac"
#define OS_ID "mac_intel"
#endif //BRANCH_OS_SUFFIX

#ifndef BRANCH_OS_SUFFIX
#error "No supported OS found for app updater config."
#endif

#endif //MOLFLOW_PROJ_GOOGLEANALYTICS_H