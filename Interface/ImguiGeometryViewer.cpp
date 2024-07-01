#include "ImguiGeometryViewer.h"
#include "ImguiExtensions.h"
#include "ImguiWindow.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Interface.h"
#include "GLWindowManager.h"
#include "GeometryViewer.h"

/*
- GL Viewer has to be rendered into an SDL2 texture object which is then placed in an ImGui Window as an Image

based on: https://www.codingwiththomas.com/blog/rendering-an-opengl-framebuffer-into-a-dear-imgui-window
and https://gamedev.stackexchange.com/questions/140693/how-can-i-render-an-opengl-scene-into-an-imgui-window
with help from chatGPT 3.5
*/

void ImGeoViewer::Init(Interface* mApp_) {
	mApp = mApp_;
	auto v = glGetString(GL_VERSION);
	if (v[0] == '1') oldOpenGL = true;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	// Generate framebuffer
	if (!oldOpenGL) {
		glGenFramebuffers(1, &frameBufferID);
	}
	// adding an extra viewer would require significant changes to core molflow code
	//glViewer = new GeometryViewer(4);
}

ImGeoViewer::~ImGeoViewer()
{
	if (glViewer) free(glViewer);
}

bool ImGeoViewer::CreateTexture()
{
	if (!needsTextureResize) return true;
	// Generate texture
	glDeleteTextures(1, &textureID);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (!oldOpenGL) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, NextPowOfTwo(textureWidth), NextPowOfTwo(textureHeight), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glGetError();
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (!oldOpenGL) {

		// Attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

		// Check framebuffer status
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Framebuffer not complete!" << std::endl;
			return false;
		}
	}
	needsTextureResize = false;
	return true;
}

void ImGeoViewer::DrawViewer()
{
	GLToolkit::CheckGLErrors("Debug");
	if (!drawn) return;
	if (!needsRerender) {
		glViewer->SetBounds(availableTLcorner.x, availableTLcorner.y - 22, availableSpace.x, availableSpace.y);
		glViewer->SetVisible(false);
		return;
	}
	hadErrors = false;
	if (!glViewer || !drawn) return;
	int windowW, windowH;
	SDL_GetWindowSize(mApp->mainScreen, &windowW, &windowH);
	glViewer->SetBounds(0, windowH - availableSpace.y - 22, availableSpace.x, availableSpace.y);
	glViewer->SetVisible(true);

	if (!oldOpenGL) {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
		hadErrors &= !CreateTexture();
		glClear(GL_COLOR_BUFFER_BIT);
		if (!hadErrors) glViewer->Paint(); // only draw if there were no errors
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else {
		hadErrors &= !CreateTexture();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (!hadErrors) glViewer->Paint(); // only draw if there were no errors
		glBindTexture(GL_TEXTURE_2D, textureID);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, NextPowOfTwo(textureWidth), NextPowOfTwo(textureHeight), 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glViewer->SetBounds(availableTLcorner.x, availableTLcorner.y-22, availableSpace.x, availableSpace.y);
	glViewer->SetVisible(false);
	needsRerender = false;
}

void ImGeoViewer::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 70, txtH * 20), ImVec2(txtW*1400, txtH*400));
	if (preventDragging)
	{
		ImGui::SetNextWindowPos(windowPos);
		preventDragging = false;
	}
	ImGui::Begin("Geometry Viewer", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	windowPos = ImGui::GetWindowPos();


	if (hadErrors) { 
		ImGui::Text("Had errors");
		needsRerender = true;
	}


	// event passthrough when window is in focus and viewer is hovered (temporary solution)
	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered() && ImMath::IsInsideVec2(ImMath::AddVec2(availableTLcorner, ImVec2(0, 22)), ImMath::AddVec2(availableTLcorner, availableSpace), ImGui::GetMousePos())) {
		try {
			LockWrapper lW(mApp->imguiRenderLock);
			glViewer->ManageEvent(&(mApp->imWnd->event));
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error handling event: " << e.what() << std::endl;
			needsRerender = true;
		}
		if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			preventDragging = true;

		}
		//mApp->imWnd->skipImGuiEvents = true;
		if (glViewer) glViewer->SetFocus(true);
		needsRerender = true;
	}
	if (ImGui::GetMousePos().y > windowPos.y + 25) preventDragging = true;
	
	// draw the image containing the viewer
	if (!oldOpenGL) {
		ImGui::Image((void*)(intptr_t)textureID, ImVec2(textureWidth, textureHeight), ImVec2(0, 1), ImVec2(1, 0));
	}
	else {
		ImVec2 uv0 = ImVec2(0, (float)textureHeight / (float)NextPowOfTwo(textureHeight));
		ImVec2 uv1 = ImVec2((float)textureWidth / (float)NextPowOfTwo(textureWidth) , 0);
		ImGui::Image((void*)(intptr_t)textureID, ImVec2(textureWidth, textureHeight), uv0, uv1);
	}
	
	ImVec2 prevAvailSpace = availableSpace;
	ImVec2 prevCorner = availableTLcorner;

	availableSpace = ImMath::SubstractVec2(ImGui::GetWindowSize(), ImVec2(2*margin, margin+25+22));
	availableTLcorner = ImMath::AddVec2(ImGui::GetWindowPos(),ImVec2(margin,25));

	glViewer->ImPosX = availableTLcorner.x;
	glViewer->ImPosY = availableTLcorner.y;
	
	if (prevAvailSpace != availableSpace) {
		needsTextureResize = true;
		needsRerender = true;
	}
	
	textureWidth = (int)availableSpace.x;
	textureHeight = (int)availableSpace.y;

	if (oldOpenGL) {
		textureWidth = std::min(textureWidth, maxTextureSize);
		textureHeight = std::min(textureHeight, maxTextureSize);
		availableSpace.x = textureWidth;
	}
	
	ImGui::Dummy(ImGui::GetContentRegionAvail());
	if (ImGui::IsItemHovered()) preventDragging = false;

	ImGui::End();
}

void ImGeoViewer::OnShow()
{
	// adding an extra viewer would require significant changes to core molflow code
	glViewer = mApp->viewers[0];
	glViewer->isInImgui = true;
	glViewer->SetVisible(false);
	glViewer->SetFocus(true);
}

void ImGeoViewer::OnHide()
{
	glViewer->isInImgui = false;
	glViewer->SetFocus(false);
	glViewer->SetVisible(true);
	mApp->Place3DViewer();
}

int ImGeoViewer::NextPowOfTwo(unsigned int value) {
	return (int)pow(2, ceil(log2(value)));
}