#include "ImguiGeometryViewer.h"
#include "ImguiExtensions.h"
#include "ImguiWindow.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Interface.h"
#include "GLWindowManager.h"

/*
- GL Viewer has to be rendered into an SDL2 texture object which is then placed in an ImGui Window as an Image
currently can't get drawing into texture to work

based on: https://www.codingwiththomas.com/blog/rendering-an-opengl-framebuffer-into-a-dear-imgui-window
and https://gamedev.stackexchange.com/questions/140693/how-can-i-render-an-opengl-scene-into-an-imgui-window
*/

void ImGeoViewer::Draw()
{
	if (!glViewer) return;
	target = SDL_CreateTexture(SDL_GetRenderer(mApp->mainScreen), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, (int)availableSpace.x, (int)availableSpace.y);
	SDL_SetRenderTarget(SDL_GetRenderer(mApp->mainScreen), target);
	glViewer->Paint(); // despite setting the texture as target still draws in main window
	SDL_SetRenderTarget(SDL_GetRenderer(mApp->mainScreen), NULL);
	if (!drawn) return;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 70, txtH * 20), ImVec2(txtW*140, txtH*40));
	ImGui::Begin("Geometry Viewer", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);

	availableSpace = ImMath::SubstractVec2(ImGui::GetWindowSize(), ImVec2(2*margin, margin+22));
	availableTLcorner = ImMath::AddVec2(ImGui::GetWindowPos(),ImVec2(margin,0));

	glViewer->SetBounds(availableTLcorner.x, availableTLcorner.y, availableSpace.x, availableSpace.y);
	// event passthrough when window is in focus and viewer is hovered (temporary solution)
	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered() && ImMath::IsInsideVec2(ImMath::AddVec2(availableTLcorner, ImVec2(0, 22)), ImMath::AddVec2(availableTLcorner, ImMath::AddVec2(availableSpace, ImVec2(0, 22))), ImGui::GetMousePos())) {
		mApp->imWnd->skipImGuiEvents = true;
	}
	//outline
	ImGui::GetBackgroundDrawList()->AddRect(ImMath::AddVec2(availableTLcorner, ImVec2(0, 22)), ImMath::AddVec2(availableTLcorner, ImMath::AddVec2(availableSpace, ImVec2(0, 22))), IM_COL32(255, 255, 255, 255));
	// draw the image containing the viewer
	ImGui::GetWindowDrawList()->AddImage(target, availableTLcorner, availableTLcorner+availableSpace); // does not work, probably need to pass a different type (GL texture pointer instead of SDL_Texture)
	ImGui::End();
}

void ImGeoViewer::OnShow()
{
	// TODO add separate viewer for this
	glViewer = mApp->viewers[0];
}

void ImGeoViewer::OnHide()
{
	mApp->Place3DViewer();
}