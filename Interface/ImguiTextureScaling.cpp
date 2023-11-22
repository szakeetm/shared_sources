#include "ImguiTextureScaling.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"
#include "Helper/MathTools.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "Interface.h"
#include "ImguiExtensions.h"
#include <math.h>

void ImTextureScailing::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowPos(ImVec2(3*txtW, 3*txtH), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(72*txtW,15*txtH),ImVec2(1000*txtW,100*txtH));
	
	ImGui::Begin("Texture Scailing", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
	
	ImGui::BeginChild("Range", ImVec2(ImGui::GetContentRegionAvail().x - 15 * txtW, 6 * txtH), true);
	ImGui::TextDisabled("Texture Range");
	if(ImGui::BeginTable("##layoutHelper", 4, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Min");
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Max");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(10 * txtW);
		ImGui::InputText("##minInput", &minInput);
		if (ImGui::IsItemFocused() && (ImGui::IsKeyPressed(SDL_SCANCODE_RETURN) || ImGui::IsKeyPressed(SDL_SCANCODE_RETURN2))) {
			autoscale = false;
			ApplyButtonPress();
		}
		ImGui::SetNextItemWidth(10 * txtW);
		ImGui::InputText("##maxInput", &maxInput);
		if (ImGui::IsItemFocused() && (ImGui::IsKeyPressed(SDL_SCANCODE_RETURN) || ImGui::IsKeyPressed(SDL_SCANCODE_RETURN2))) {
			autoscale = false;
			ApplyButtonPress();
		}
		ImGui::TableNextColumn();
		if (ImGui::Checkbox("Autoscale", &autoscale)) {
			molflowGeom->texAutoScale = autoscale;
			WorkerUpdate();
		}
		ImGui::SetNextItemWidth(txtW*20);
		if (!autoscale) ImGui::BeginDisabled();
		if(ImGui::BeginCombo("##includeCombo", includeComboLabels[includeComboVal])) {
			for (short i = 0; i < 3; i++) {
				if (ImGui::Selectable(includeComboLabels[i])) {
					includeComboVal = i;
				}
			}
			ImGui::EndCombo();
		}
		if (!autoscale) ImGui::EndDisabled();
		ImGui::TableNextColumn();
		if (ImGui::Checkbox("Use colors", &colors)) {
			molflowGeom->texColormap = colors;
			WorkerUpdate();
		}
		if (ImGui::Checkbox("Logarithmic scale", &logScale)) {
			molflowGeom->texLogScale = logScale;
			WorkerUpdate();
		}
		ImGui::EndTable();
	}
	if (ImGui::Button("Set to geometry")) SetCurrentButtonPress();
	ImGui::SameLine();
	if (ImGui::Button("Apply")) ApplyButtonPress();
	ImGui::SameLine();
	UpdateSize();
	ImGui::Text("Swap: "+swapText);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("Geometry", ImVec2(ImGui::GetContentRegionAvail().x, 6 * txtH), true);
	ImGui::TextDisabled("Geometry");
	GetCurrentRange();
	ImGui::Text(fmt::format("Min: {:.3e}", cMinScale));
	ImGui::Text(fmt::format("Max: {:.3e}", cMaxScale));
	ImGui::HelpMarker("These are the values obtained from geomatry\nthey are used by autoscale");
	ImGui::EndChild();

	ImGui::BeginChild("Gradient", ImVec2(0, ImGui::GetContentRegionAvail().y-1.5*txtH), true);
	ImGui::TextDisabled("Gradient");
	DrawGradient();
	ImGui::EndChild();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Show"); ImGui::SameLine();
	if (ImGui::BeginCombo("##Show", showComboLabels[showComboVal])) {
		for (short i = 0; i < 3; i++) if (ImGui::Selectable(showComboLabels[i])) {
			showComboVal = i;
			molflowGeom->textureMode = showComboVal;
			WorkerUpdate();
		}
		ImGui::EndCombo();
	}
	
	ImGui::End();
}

void ImTextureScailing::Init(Interface* mApp_)
{
	mApp = mApp_;
	interfGeom = mApp->worker.GetGeometry();
	molflowGeom = mApp->worker.GetMolflowGeometry();
	colorMap = GenerateColorMap();
}

void ImTextureScailing::UpdateSize()
{
	size_t swap = 0;
	size_t nbFacet = molflowGeom->GetNbFacet();
	for (size_t i = 0; i < nbFacet; i++) {
		InterfaceFacet* f = molflowGeom->GetFacet(i);
		if (f->sh.isTextured) {
			swap += f->GetTexSwapSize(colors);
		}
	}
	swapText = FormatMemory(swap);
}

// get toggle states and values from molflow
void ImTextureScailing::Update() {
	GetCurrentRange();
	autoscale = molflowGeom->texAutoScale;
	includeComboVal = molflowGeom->texAutoScaleIncludeConstantFlow;
	logScale = molflowGeom->texLogScale;
	colors = molflowGeom->texColormap;
	showComboVal = molflowGeom->textureMode;
	UpdateSize();

	minInput = std::to_string(minScale);
	maxInput = std::to_string(maxScale);

}

void ImTextureScailing::SetCurrentButtonPress()
{
	autoscale = molflowGeom->texAutoScale;
	GetCurrentRange();
	minScale = cMinScale;
	maxScale = cMaxScale;
	minInput = std::to_string(minScale);
	maxInput = std::to_string(maxScale);
	ApplyButtonPress();
}

void ImTextureScailing::ApplyButtonPress()
{
	molflowGeom->texAutoScaleIncludeConstantFlow = includeComboVal;
	molflowGeom->texAutoScale = autoscale;
	if (autoscale) {} // empty if statement, acts like adding !autoscale && to all following else statements
	else if (!Util::getNumber(&minScale, minInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid minimum value");
		Update();
		return;
	}
	else if (!Util::getNumber(&maxScale, maxInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid maximum value");
		Update();
		return;
	}
	else if (minScale > maxScale) {
		ImIOWrappers::InfoPopup("Error", "min must be lower than max");
		Update();
		return;
	}
	else {
		molflowGeom->texture_limits[molflowGeom->textureMode].manual.min.steady_state = minScale;
		molflowGeom->texture_limits[molflowGeom->textureMode].manual.max.steady_state = maxScale;
	}

	WorkerUpdate();
}

bool ImTextureScailing::WorkerUpdate()
{
	LockWrapper lock(mApp->imguiRenderLock);
	try {
		mApp->worker.Update(0.0f);
		return true;
	}
	catch (const std::exception& e) {
		ImIOWrappers::InfoPopup("Error (Worker::Update)", e.what());
		return false;
	}
	Update();
}

void ImTextureScailing::DrawGradient()
{
	ImGui::BeginChild("##ImGradient", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 1 * txtH));
	ImVec2 availableSpace = ImMath::SubstractVec2(ImGui::GetWindowSize(), ImVec2(0, 0));
	ImVec2 availableTLcorner = ImGui::GetWindowPos();

	ImVec2 gradientSize = ImVec2(availableSpace.x*0.8, 1.5*txtH);

	ImVec2 midpoint = ImMath::AddVec2(availableTLcorner, ImVec2(availableSpace.x*0.5, availableSpace.y*0.4));
	ImVec2 TLcorner = ImMath::SubstractVec2(midpoint, ImMath::ScaleVec2(gradientSize, 0.5));
	ImVec2 BRcorner = ImMath::AddVec2(midpoint, ImMath::ScaleVec2(gradientSize, 0.5));

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	static std::string hoveredVal = "";
	
	// draw gradient
	if (colors) {
		float offset = gradientSize.x / (colorMap.size() - 1);
		ImVec2 TLtmp = TLcorner, BRtmp = ImVec2(TLcorner.x+offset, BRcorner.y);
		for (int i = 0; i < colorMap.size()-1; ++i) {
			drawList->AddRectFilledMultiColor(TLtmp, BRtmp, colorMap[i], colorMap[i+1], colorMap[i+1], colorMap[i]);
			TLtmp.x += offset;
			BRtmp.x += offset;
		}
	}
	else {
		drawList->AddRectFilledMultiColor(TLcorner, BRcorner, colorMap[0], IM_COL32(255, 255, 255, 255), IM_COL32(255,255,255,255), colorMap[0]);
	}

	// todo: draw minor tick marks
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font = io.Fonts->Fonts[0];
	drawList->PushTextureID(font->ContainerAtlas->TexID);

	std::vector<double> majorTicVals = { TLcorner.x, midpoint.x, BRcorner.x };

	for (const auto& tick : majorTicVals) {
		double val = Utils::mapRange(tick, TLcorner.x, BRcorner.x, minScale, maxScale);
		if (logScale) val = logScaleInterpolate(val, minScale, maxScale);

		std::string text = fmt::format("{:.2e}", val);
		ImVec2 textSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0, text.c_str());
		drawList->AddText(ImVec2(tick -textSize.x/2,BRcorner.y), colorMap[0], text.c_str());
		drawList->AddRectFilled(ImVec2(tick, (midpoint.y + BRcorner.y) / 2), ImVec2(tick + 1, BRcorner.y), colorMap[0]);
	}
	// handle hovering
	ImVec2 mousePos = ImGui::GetMousePos();
	static ImVec2 hoverMarkPos = ImMath::SubstractVec2(mousePos, midpoint);
	if (mousePos.x < TLcorner.x) mousePos.x = TLcorner.x;
	else if (mousePos.x > BRcorner.x) mousePos.x = BRcorner.x;

	if (ImGui::IsWindowHovered() && ImMath::IsInsideVec2(TLcorner, BRcorner, mousePos)) {
		double linX = Utils::mapRange(mousePos.x, TLcorner.x, BRcorner.x, minScale, maxScale);
		if(!logScale)
			hoveredVal = fmt::format("{:.4f}", linX);
		else {
			double val = logScaleInterpolate(linX, minScale, maxScale);
			hoveredVal = fmt::format("{:.2e}", val);
		}
		hoverMarkPos = ImMath::SubstractVec2(mousePos, midpoint); // get mousePos relative to midpoint
		//this is needed so when not hovered and window is moved the marker stays in the same place relative to the gradient
	}
	// draws a vertical line under the mouse cursors position
	ImVec2 posTmp = ImMath::AddVec2(hoverMarkPos, midpoint); // get absolute position of marker
	drawList->AddRectFilled(ImVec2(posTmp.x,TLcorner.y-5), ImVec2(posTmp.x+1,BRcorner.y+5), colorMap[0]);

	ImGui::EndChild();
	ImGui::Text(hoveredVal);
}

void ImTextureScailing::GetCurrentRange()
{
	if (molflowGeom->texAutoScaleIncludeConstantFlow == 0) {
		cMinScale = molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.moments_only;
		cMaxScale =	molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.moments_only;
	}
	else if (molflowGeom->texAutoScaleIncludeConstantFlow == 1) {
		cMinScale =	std::min(molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.steady_state, molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.moments_only);
		cMaxScale =	std::max(molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.steady_state, molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.moments_only);
	}
	else { // == 2
		cMinScale =	molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.steady_state;
		cMaxScale =	molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.steady_state;
	}
}

double ImTextureScailing::logScaleInterpolate(double x, double leftTick, double rightTick)
{
	// log(x) where x <=0 is undefined, clamp scales above 0
	if (leftTick < 1e-20) leftTick = 1e-20;
	if (rightTick < 1e-20) rightTick = 1e-20;
	/*	Code based on:
		Linear and Logarithmic Interpolation
		Markus Deserno
		Max-Planck-Institut fur¨ Polymerforschung, Ackermannweg 10, 55128 Mainz, Germany
		https://www.cmu.edu/biolphys/deserno/pdf/log_interpol.pdf (accessed 21-11-2023)
	*/
	double a = x - leftTick, b = rightTick - x;
	double f = a / (a + b); // ratio of distances to gradient edges
	double val = pow(rightTick, f) * pow(leftTick, 1 - f);
	return val;
}

std::vector<ImU32> ImTextureScailing::GenerateColorMap()
{
	std::vector<ImU32> out;

	out.push_back(ImGui::GetColorU32(IM_COL32(0, 0, 0, 255))); // black
	out.push_back(ImGui::GetColorU32(IM_COL32(255, 0, 0, 255))); // red
	out.push_back(ImGui::GetColorU32(IM_COL32(255, 128, 48, 255))); // orange
	out.push_back(ImGui::GetColorU32(IM_COL32(248, 248, 0, 255))); // yellow
	out.push_back(ImGui::GetColorU32(IM_COL32(40, 255, 40, 255))); // green
	out.push_back(ImGui::GetColorU32(IM_COL32(0, 64, 255, 255))); // light blue
	out.push_back(ImGui::GetColorU32(IM_COL32(0, 0, 176, 255))); // dark blue 
	out.push_back(ImGui::GetColorU32(IM_COL32(128, 0, 144, 255))); // purple 1
	out.push_back(ImGui::GetColorU32(IM_COL32(240, 0, 128, 255))); // purple 2

	return out;
}
