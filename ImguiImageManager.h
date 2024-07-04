#pragma once
#include <SDL2/SDL_opengl.h>
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
	std::pair<ImVec2, ImVec2> GetTextureUVs();
private:
	std::string filename = "";
	GLuint texture = 0;
	int width = 0, height = 0;
	bool oldOpenGL = false;
};