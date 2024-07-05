#pragma once

#include "ImguiWindowBase.h"
class MolFlow;

class ImSidebar : public ImWindow
{
public:
	void Draw();
	void Init(Interface* mApp_);
	void OnShow() override;
protected:
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
	double PumpingSpeedFromSticking(double sticking, double area, double temperature);
	double StickingFromPumpingSpeed(double pumpingSpeed, double area, double temperature);
	void DrawSectionDebug();
	void DrawSectionViewerSettings();
	void DrawSectionSelectedFacet();
	void DrawSectionSimulation();
	void DrawFacetTable();
	std::string title;
#ifdef MOLFLOW
	MolFlow* molApp = nullptr;
#endif
};