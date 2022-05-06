//
// Created by pbahr on 8/2/21.
//

#ifndef MOLFLOW_PROJ_IMGUIGLOBALSETTINGS_H
#define MOLFLOW_PROJ_IMGUIGLOBALSETTINGS_H

#if defined(MOLFLOW)
class MolFlow;
void ShowGlobalSettings(MolFlow *mApp, bool *show_global_settings, bool &nbProcChanged, bool &recalcOutg,
                        bool &changeDesLimit, int &nbProc) ;
#else
class SynRad;
void ShowGlobalSettings(SynRad *mApp, bool *show_global_settings, bool &nbProcChanged, bool &recalcOutg,
                        bool &changeDesLimit, int &nbProc) ;
#endif

#endif //MOLFLOW_PROJ_IMGUIGLOBALSETTINGS_H
