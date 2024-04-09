#pragma once
#include "ImguiWindowBase.h"
#include "GeometryViewer.h"

class ImGeoViewer : public ImWindow {
public:
	void Draw();
	void OnShow() override;
protected:
	GeometryViewer* glViewer;
	ImVec2 availableSpace, availableTLcorner;
};