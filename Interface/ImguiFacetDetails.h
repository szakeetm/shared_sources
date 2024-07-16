#pragma once
#include "ImguiWindowBase.h"

class ImFacetDetails : public ImWindow {
public:
	void Draw();
	void Update();
protected:
	void OnShow() override;
	void Invert();
	void DrawTable();
	bool sticking = true;
	bool opacity = true;
	bool structure = true;
	bool link = true;
	bool desorption = true;
	bool reflection = true;
	bool twoSided = true;
	bool nbVertex = true;
	bool area = true;
	bool tempK = true;
	bool facet2DBox = true;
	bool textureUV = true;
	bool meshSample = true;
	bool textureRecord = true;
	bool memory = true;
	bool planarity = true;
	bool profile = true;
	bool impingRate = true;
	bool particleDensity = true;
	bool gasDensity = true;
	bool pressure = true;
	bool speed = true;
	bool mcHits = true;
	bool equivHits = true;
	bool des = true;
	bool equivAbs = true;
	bool force = true;
	bool force2 = true;
	bool torque = true;
	int columns = 1;

	std::vector < std::vector<std::string> > content; // row-column ordered string representation of the table
};