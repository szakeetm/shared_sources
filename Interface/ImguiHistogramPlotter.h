#pragma once
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "ImguiWindowBase.h"
#include "Geometry_shared.h"

#ifdef MOLFLOW
#define IM_HISTOGRAM_TABS 3
#else
#define IM_HISTOGRAM_TABS 2
#endif

class ImHistogramPlotter : public ImWindow {
public:
#ifdef MOLFLOW
	enum plotTabs : BYTE {bounces=0, distance=1, time=2, none=3}; // using BYTE as it is the smalest type capable of holding 3 values
#else
	enum plotTabs : BYTE { bounces = 0, distance = 1}; // using BYTE as it is the smalest type capable of holding 3 values
#endif
	void Draw();
	void Init(Interface* mApp_);
	void LoadHistogramSettings();
	bool anchor = true;
	bool IsPlotted(plotTabs tab, size_t facetId);
	void RefreshFacetLists();
	void Reset(); // hard reset, retain no data
protected:
	//functions
	void DrawPlot();
	void RemovePlot();
	void AddPlot();
	void DrawMenuBar();
	void RefreshPlots();
	void Export(bool toFile, bool plottedOnly);

	//types
	class ImHistogramSettings : public ImWindow {
	public:
		void Draw();
		float width;
		ImHistogramPlotter* parent;
		typedef struct {
			bool amIDisabled = true;
			short recBounce = false;
			std::string maxRecNbBouncesInput = "10000";
			long nbBouncesMax = 10000;
			std::string bouncesBinSizeInput = "1";
			short bouncesBinSize = 1;
			short recFlightDist = false;
			std::string maxFlightDistInput = "10";
			double maxFlightDist = 10;
			std::string distBinSizeInput = "0.001";
			double distBinSize = 0.001f;

#ifdef MOLFLOW
			short recTime = false;
			std::string maxFlightTimeInput = "0.1";
			double maxFlightTime = 0.1f;
			std::string timeBinSizeInput = "1e-5";
			double timeBinSize = 0.00001f;
#endif
			bool showMemEst = false;
			double memEst = 0;
		} histSet;
		histSet globalHistSet, facetHistSet;
		bool Apply();
		void DrawSettingsGroup(histSet& set, bool tristate=false);
	};

	//variables
	InterfaceGeometry* interfGeom;
	ImHistogramSettings settingsWindow;
	plotTabs plotTab = bounces, prevPlotTab = none;
	std::string xAxisName = "Number of bounces";
	bool normalize = false;
	std::vector<size_t> comboOpts[IM_HISTOGRAM_TABS];
	std::vector<ImPlotData> data[IM_HISTOGRAM_TABS];
	ImPlotData globals[IM_HISTOGRAM_TABS];
	long comboSelection=-2;
	int maxDisplayed = 1000;
	bool limitPoints = true;
	bool showValueOnHover = true;
	bool overrange = true;
	bool logX = false, logY = false;
};