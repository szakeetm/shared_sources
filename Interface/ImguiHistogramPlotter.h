#pragma once
#include "ImguiWindowBase.h"
#include "Geometry_shared.h"
#include "Buffer_shared.h"

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
	void UpdatePlotter();
protected:
	//functions
	void OnShow() override;
	void DrawPlot();
	void RemovePlot(size_t idx, plotTabs tab);
	void AddPlot(size_t idx);
	bool IsPlotted(size_t idx);
	void DrawMenuBar();
	void RefreshPlots();
	void Export(bool toFile, bool plottedOnly);
	void ShowFacet(size_t idx, bool add);
	

	//types
	class ImHistogramSettings : public ImWindow {
	public:
		void Draw();
		float width;
		void UpdateOnFacetChange();
		ImHistogramPlotter* parent;
		bool Apply();
		void DrawSettingsGroup(HistogramParams& set, bool global = false, bool disabled = false);
		void EvaluateMixedState();
		void CalculateMemoryEstimate_Current();
		void CalculateMemoryEstimate_New(bool global);

		HistogramParams globalHistSet, facetHistSet;

		short globalRecordBounce = 0;
		std::string globalBouncesMaxInput = "10000";
		std::string globalBouncesBinSizeInput = "1";

		short globalRecordDistance = 0;
		std::string globalDistanceMaxInput = "10.0";
		std::string globalDistanceBinSizeInput = "0.001";

#if defined(MOLFLOW)
		short globalRecordTime = 0;
		std::string globalTimeMaxInput = "10.0";
		std::string globalTimeBinSizeInput = "0.001";
#endif
		short facetRecordBounce = 0;
		std::string facetBouncesMaxInput = "10000";
		std::string facetBouncesBinSizeInput = "1";

		short facetRecordDistance = 0;
		std::string facetDistanceMaxInput = "10.0";
		std::string facetDistanceBinSizeInput = "0.001";

#if defined(MOLFLOW)
		short facetRecordTime = 0;
		std::string facetTimeMaxInput = "10.0";
		std::string facetTimeBinSizeInput = "0.001";
#endif
		struct HistogramGUISettings {

			//Extra flags to handle mixed state
			bool facetRecBounceMixed = false;
			bool facetHitLimitMixed = false;
			bool facetHitBinsizeMixed = false;

			bool facetRecDistanceMixed = false;
			bool facetDistanceLimitMixed = false;
			bool facetDistanceBinsizeMixed = false;

#if defined(MOLFLOW)
			bool facetRecTimeMixed = false;
			bool facetTimeLimitMixed = false;
			bool facetTimeBinsizeMixed = false;

			size_t memCurrentGlobal = 0, memCurrentFacet = 0;
			std::string memNewGlobal = "0 bytes", memNewFacet = "0 bytes";
#endif
		} states;
	};

	//variables
	InterfaceGeometry* interfGeom;
	plotTabs plotTab = bounces, prevPlotTab = none;
	std::string xAxisName = "Number of bounces";
	bool normalize = false;
	std::vector<size_t> histogrammedFacets[IM_HISTOGRAM_TABS];
	std::vector<ImPlotData> data[IM_HISTOGRAM_TABS];
	ImPlotData globals[IM_HISTOGRAM_TABS];
	int maxDisplayed = 1000;
	bool limitPoints = true;
	bool showValueOnHover = true;
	bool overrange = true;
	bool logX = false, logY = false;
	ImHistogramSettings settingsWindow;

	short aggregateState = 0;
	bool mixedState = false;
	std::vector<short> histogramDrawToggle[IM_HISTOGRAM_TABS];
	short globalDrawToggle[IM_HISTOGRAM_TABS];
	void ApplyAggregateState();
	void UpdateSidebarMasterToggle();
};