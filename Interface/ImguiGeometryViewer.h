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
	int NextPowOfTwo(unsigned int value);
protected:
	float margin = 6;
	GeometryViewer* glViewer;
	ImVec2 availableSpace, availableTLcorner, windowPos;
	bool preventDragging = false;
	int textureWidth=100, textureHeight=100;
	void* pixels = nullptr;
	int pitch = 0;
	
	GLuint textureID=0;
	GLuint frameBufferID=0;
	bool CreateTexture();
	bool hadErrors;
	bool needsRerender = true;
	bool needsTextureResize = true;
	bool oldOpenGL = false;
	GLint maxTextureSize = 0;
};