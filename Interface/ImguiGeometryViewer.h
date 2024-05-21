#pragma once
#include "ImguiWindowBase.h"
#include "GeometryViewer.h"
#include <SDL2/SDL_opengl.h>

class ImGeoViewer : public ImWindow {
public:
	void Draw();
	void OnShow() override;
	void OnHide() override;
protected:
	float margin = 6;
	GeometryViewer* glViewer;
	ImVec2 availableSpace, availableTLcorner;
	
	SDL_Texture* target;
};