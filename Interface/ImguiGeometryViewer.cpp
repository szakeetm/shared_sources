#include "ImguiGeometryViewer.h"
#include "ImguiExtensions.h"
#include "ImguiWindow.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Interface.h"
#include "GLWindowManager.h"
#include "GeometryViewer.h"

/*
- GL Viewer has to be rendered into an SDL2 texture object which is then placed in an ImGui Window as an Image
currently can't get drawing into texture to work

based on: https://www.codingwiththomas.com/blog/rendering-an-opengl-framebuffer-into-a-dear-imgui-window
and https://gamedev.stackexchange.com/questions/140693/how-can-i-render-an-opengl-scene-into-an-imgui-window
with help from chatGPT 3.5
*/

void ImGeoViewer::Init(Interface* mApp_) {
	mApp = mApp_;
	renderer = SDL_CreateRenderer(mApp->mainScreen, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
		hadErrors = true;
	}
	// adding an extra viewer would require significant changes to core molflow code
	//glViewer = new GeometryViewer(4);
}

ImGeoViewer::~ImGeoViewer()
{
	if (renderer) SDL_DestroyRenderer(renderer);
	if (target) SDL_DestroyTexture(target);
	if (glViewer) free(glViewer);
}

bool ImGeoViewer::CreateTexture()
{
	if (hadErrors) return false;
	if (target) SDL_DestroyTexture(target);
	target = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		textureWidth,
		textureHeight
	);
	if (!target) {
		std::cerr << "Failed to create render texture: " << SDL_GetError() << std::endl;
		hadErrors = true;
		return false;
	}
	// Set Render target to be the texture (despite this the viewer renders into the main window and not the texture)
	if (SDL_SetRenderTarget(renderer, target) != 0) {
		std::cerr << "Failed to set render target: " << SDL_GetError() << std::endl;
		hadErrors = true;
	}
	return true;
}

void ImGeoViewer::DrawViewer()
{

	hadErrors = false;
	if (!glViewer || !drawn) return;
	glViewer->SetBounds(availableTLcorner.x, availableTLcorner.y, availableSpace.x, availableSpace.y);

	CreateTexture();

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	glViewer->Paint(); // despite setting the texture as target still draws in main window

	// Create a new OpenGL texture
	glGenTextures(1, &openglTexture);
	glBindTexture(GL_TEXTURE_2D, openglTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Copy the SDL texture to the OpenGL texture
	int texWidth, texHeight;
	SDL_QueryTexture(target, NULL, NULL, &texWidth, &texHeight);
	void* pixels = nullptr;
	int pitch = 0;
	SDL_LockTexture(target, NULL, &pixels, &pitch);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	SDL_UnlockTexture(target);

	// Unbind the texture
	glBindTexture(GL_TEXTURE_2D, 0);
	
	/*
	// Set Render target back to the main window
	if (SDL_SetRenderTarget(renderer, NULL) != 0) {
		std::cerr << "Failed to set render target: " << SDL_GetError() << std::endl;
		hadErrors = true;
	}
	*/
}

void ImGeoViewer::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 70, txtH * 20), ImVec2(txtW*140, txtH*40));
	ImGuiIO io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	if (preventDragging)
	{
		ImGui::SetNextWindowPos(windowPos);
		preventDragging = false;
	}
	ImGui::Begin("Geometry Viewer", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
	windowPos = ImGui::GetWindowPos();


	if (hadErrors) ImGui::Text("Had errors");


	// event passthrough when window is in focus and viewer is hovered (temporary solution)
	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered() && ImMath::IsInsideVec2(ImMath::AddVec2(availableTLcorner, ImVec2(0, 22)), ImMath::AddVec2(availableTLcorner, ImMath::AddVec2(availableSpace, ImVec2(0, 22))), ImGui::GetMousePos())) {
		try {
			LockWrapper lW(mApp->imguiRenderLock);
			glViewer->ManageEvent(&(mApp->imWnd->event));
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error handling event: " << e.what() << std::endl;
		}
		preventDragging = true;
		//mApp->imWnd->skipImGuiEvents = true;
		if (glViewer) glViewer->SetFocus(true);
	}
	
	// draw the image containing the viewer
	// despite the code to do it, the viewer does not seem to be rendered into this texture
	ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)openglTexture, availableTLcorner, availableTLcorner+availableSpace);
	
	availableSpace = ImMath::SubstractVec2(ImGui::GetWindowSize(), ImVec2(2*margin, margin+25+22));
	availableTLcorner = ImMath::AddVec2(ImGui::GetWindowPos(),ImVec2(margin,25));
	textureWidth = (int)availableSpace.x;
	textureHeight = (int)availableSpace.y;
	
	ImGui::End();
	io.ConfigWindowsMoveFromTitleBarOnly = false;
}

void ImGeoViewer::OnShow()
{
	// adding an extra viewer would require significant changes to core molflow code
	glViewer = mApp->viewers[3];
	glViewer->SetVisible(true);
}

void ImGeoViewer::OnHide()
{
	glViewer->SetVisible(false);
	mApp->Place3DViewer();
}