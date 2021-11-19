//
// Created by pbahr on 15/06/2020.
//

#ifndef MOLFLOW_PROJ_GOOGLEANALYTICS_H
#define MOLFLOW_PROJ_GOOGLEANALYTICS_H

#define PUBLIC_WEBSITE "https://molflow.web.cern.ch/"
#define DOWNLOAD_PAGE "https://molflow.web.cern.ch/content/downloads"
#define GA_PROJECT_ID "UA-86802533-2"

#if defined(MOLFLOW)
#define REMOTE_FEED "https://gitlab.cern.ch/molflow_synrad/molflow-updater/-/raw/master/autoupdate_molflow.xml"
#define BRANCH_NAME "molflow_public"
#define APP_NAME "molflow"
#elif defined(SYNRAD)
#define REMOTE_FEED "https://gitlab.cern.ch/molflow_synrad/molflow-updater/-/raw/master/autoupdate_synrad.xml"
#define BRANCH_NAME "synrad_public"
#define APP_NAME "synrad"
#endif //BRANCH_NAME

#if defined(__LINUX_FEDORA) // set by cmake
#define BRANCH_OS_SUFFIX "_linux_fedora"
#define OS_ID "fedora"
#elif defined(__LINUX_DEBIAN) // set by cmake
#define BRANCH_OS_SUFFIX "_linux_debian"
#define OS_ID "debian"
#elif defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define BRANCH_OS_SUFFIX "_win"
#define OS_ID "win"
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