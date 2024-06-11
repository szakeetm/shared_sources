#pragma once
#include "ImguiWindowBase.h"
#include "GeometryViewer.h"
#include "graphicsLibraries.h"

class ImGeoViewer : public ImWindow {
public:
	void Draw();
	void OnShow() override;
	void OnHide() override;
	void Init(Interface* mApp_);
	~ImGeoViewer();
	void DrawViewer();
protected:
	float margin = 6;
	GeometryViewer* glViewer;
	ImVec2 availableSpace, availableTLcorner, windowPos;
	bool preventDragging = false;
	int textureWidth=100, textureHeight=100;
	void* pixels = nullptr;
	int pitch = 0;
	
	GLuint textureID;
	GLuint frameBufferID;
	SDL_Renderer* renderer;
	SDL_Texture* target;
	bool CreateTexture();
	bool hadErrors;
};