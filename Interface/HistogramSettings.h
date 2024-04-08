
#pragma once

#include "GLApp/GLWindow.h"
#include "Buffer_shared.h" //HistogramParams
#include <vector>

class GLWindow;
class GLButton;
class GLTextField;
class GLTitledPanel;
class GLLabel;
class GLToggle;

class InterfaceGeometry;
class Worker;

class HistogramSettings : public GLWindow {

public:

  // Construction
  HistogramSettings(InterfaceGeometry *s,Worker *w);

  // Component methods
  void Refresh(const std::vector<size_t>& selectedFacetIds);
  void EnableDisableControls();
  void Apply();
  void UpdateMemoryEstimate_Current(); //Called from outside for ex. on number of moments change

  // Implementation
  void ProcessMessage(GLComponent *src,int message) override;

private:

struct HistogramGUISettings {

	HistogramParams globalParams;
	HistogramParams facetParams;

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
#endif
};

  InterfaceGeometry     *interfGeom;
  Worker	   *work;

  GLTitledPanel *globalSettingsPanel, *facetSettingsPanel;

  GLToggle *globalRecordBounceToggle,*facetRecordBounceToggle;
  GLToggle *globalRecordDistanceToggle, *facetRecordDistanceToggle;

  GLTextField *globalHitLimitText,*facetHitLimitText;
  GLTextField *globalHitBinsizeText,*facetHitBinsizeText;

  GLTextField *globalDistanceLimitText,*facetDistanceLimitText;
  GLTextField *globalDistanceBinsizeText, *facetDistanceBinsizeText;

#if defined(MOLFLOW)
  GLToggle *globalRecordTimeToggle, *facetRecordTimeToggle;

  GLTextField* globalTimeLimitText, * facetTimeLimitText;
  GLTextField* globalTimeBinsizeText, * facetTimeBinsizeText;
#endif

  GLLabel* globalMemoryEstimateLabel_current;
  GLLabel* facetMemoryEstimateLabel_current; //expensive to update (scan all facets) - only when selection changes
  GLLabel* globalMemoryEstimateLabel_new;
  GLLabel* facetMemoryEstimateLabel_new; //cheap to update (uses cached nbSelectedFacets) - every toggle or textbox change

  GLButton *applyButton;

  //Helpers to know which memory estimate (global or facets) to update on click/text edit
  std::vector<GLComponent*> globalToggles,facetToggles,globalTextFields,facetTextFields;

  void UpdateMemoryEstimate_New_SelectedFacets(size_t nbSelectedFacets);
  void UpdateMemoryEstimate_New_Global();
  HistogramGUISettings GetGUIValues(); //Unified function for Apply() and UpdateMemoryEstimate(); Throws Error

  size_t nbSelectedFacetCache=0; //To avoid rescanning geometry on every keystroke
};