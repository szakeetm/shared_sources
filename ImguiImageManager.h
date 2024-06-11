#pragma once
#include "graphicsLibraries.h"
#include <string>
#include <imgui.h>

class ImImage {
public:
	ImImage();
	ImImage(const std::string& filename);
	~ImImage();
	bool LoadFromFile(const std::string& filename);
	void* GetTexture();
	ImVec2 GetSize();
	void Draw();
private:
	std::string filename = "";
	GLuint texture = 0;
	int width = 0, height = 0;
};