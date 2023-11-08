#if defined(MOLFLOW)
#define APPLICATION_NAME "molflow"
#define REMOTE_FEED "https://gitlab.cern.ch/molflow_synrad/molflow-updater/-/raw/master/dummy/autoupdate_molflow_testing.xml"
#define BRANCH_NAME "molflow_public"
#elif defined(SYNRAD)
#define APPLICATION_NAME "synrad"
#define REMOTE_FEED "https://gitlab.cern.ch/molflow_synrad/molflow-updater/-/raw/master/autoupdate_synrad.xml"
#define BRANCH_NAME "synrad_public"
#endif //BRANCH_NAME