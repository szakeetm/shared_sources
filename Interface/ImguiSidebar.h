

#ifndef MOLFLOW_PROJ_IMGUISIDEBAR_H
#define MOLFLOW_PROJ_IMGUISIDEBAR_H

class InterfaceGeometry;
#if defined(MOLFLOW)
class MolFlow;
class ImGuiSidebar {
public:
	void ShowAppSidebar(bool *p_open, MolFlow *mApp, InterfaceGeometry *interfGeom);
protected:
	double PumpingSpeedFromSticking(double sticking, double area, double temperature, MolFlow* mApp);
	double StickingFromPumpingSpeed(double pumpingSpeed, double area, double temperature, MolFlow* mApp);
};
#else
class SynRad;
void ShowAppSidebar(bool *p_open, SynRad *mApp, InterfaceGeometry *interfGeom);
#endif

#endif //MOLFLOW_PROJ_IMGUISIDEBAR_H
