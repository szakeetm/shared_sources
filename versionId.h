#pragma once
#include <string>

#ifdef MOLFLOW
//Hard-coded identifiers, update these on new release and rebuild solution
//---------------------------------------------------
static const std::string appName = "Molflow";
static const int appVersionId = 2680; //Compared with available updates. Global variable, so rebuild whole solution if changed.
static const std::string appVersionName = "2.6.80";
//---------------------------------------------------
#ifdef _DEBUG
static const std::string appTitle = "MolFlow+ debug version (Compiled " __DATE__ " " __TIME__ ")";
#else
static const std::string appTitle = "Molflow+ " + appVersionName + " (" __DATE__ ")";
#endif
#endif

#ifdef SYNRAD
//Hard-coded identifiers, update these on new release and rebuild solution
//---------------------------------------------------
static const std::string appName = "Synrad";
static const int appVersionId = 2680; //Compared with available updates. Global variable, so rebuild whole solution if changed.
static const std::string appVersionName = "2.6.80";
//---------------------------------------------------
#ifdef _DEBUG
static const std::string appTitle = "Synrad+ debug version (Compiled " __DATE__ " " __TIME__ ")";
#else
static const std::string appTitle = "Synrad+ " + appVersionName + " (" __DATE__ ")";
#endif
#endif
