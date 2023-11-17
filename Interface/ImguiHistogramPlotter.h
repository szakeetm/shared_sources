#pragma once
#include "ImguiWindowBase.h"
#include "Geometry_shared.h"

class ImHistogramPlotter : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
protected:
	//functions
	void DrawPlot();
	void RemovePlot();
	void AddPlot();
	void MenuBar();
	void RefreshPlots();
	void Export(bool toFile, bool plottedOnly);

	//types
	enum plotTabs : BYTE {bounces=0, distance=1, time=2}; // using BYTE as it is the smalest type capable of holding 3 values
	
	class ImHistagramSettings : public ImWindow {
	public:
		void Draw();
		float width;
		ImHistogramPlotter* parent;
	protected:
		typedef struct {
			bool amIDisabled = true;
			bool globalRecBounce = false;
			std::string maxRecNbBouncesInput = "10000";
			size_t nbBouncesMax = 10000;
			std::string bouncesBinSizeInput = "1";
			int bouncesBinSize = 1;

			bool recFlightDist = false;
			std::string maxFlightDistInput = "10";
			double maxFlightDist = 10;
			std::string distBinSizeInput = "0.001";
			double distBinSize = 0.001f;

			bool recTime = false;
			std::string maxFlightTimeInput = "0.1";
			double maxFlightTime = 0.1f;
			std::string timeBinSizeInput = "1e-5";
			double timeBinSize = 0.00001f;

			bool showMemEst = false;
			double memEst = 0;
		} histSet;
		histSet globalHistSet, facetHistSet;
		bool Apply();
		void DrawSettingsGroup(histSet& set);
	};

	//variables
	InterfaceGeometry* interfGeom;
	ImHistagramSettings settingsWindow;
	plotTabs plotTab = bounces;
	std::string xAxisName = "Number of bounces";
	bool normalize = false;
	std::vector<size_t> comboOpts;
	std::vector<ImPlotData> data[3];
	ImPlotData globals[3];
	long comboSelection=-2;
	int maxDisplayed = 1000;
	bool limitPoints = true;
	bool showValueOnHover = true;
};