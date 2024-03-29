/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
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