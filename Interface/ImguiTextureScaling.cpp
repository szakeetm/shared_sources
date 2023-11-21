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
	
	ImGui::Begin("Texture Scailing", &drawn, ImGuiWindowFlags_NoSavedSettings);
	
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
		ImGui::SetNextItemWidth(10 * txtW);
		ImGui::InputText("##maxInput", &maxInput);
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
					molflowGeom->texAutoScaleIncludeConstantFlow = includeComboVal;
					WorkerUpdate();
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
	if (ImGui::Button("Set to current")) SetCurrent();
	ImGui::SameLine();
	if (ImGui::Button("Apply")) Apply();
	ImGui::SameLine();
	UpdateSize();
	ImGui::Text("Swap: "+swapText);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("Current", ImVec2(ImGui::GetContentRegionAvail().x, 6 * txtH), true);
	ImGui::TextDisabled("Current");
	GetCurrentRange();
	ImGui::Text(fmt::format("Min: {:.3f}", cMinScale));
	ImGui::Text(fmt::format("Max: {:.3f}", cMaxScale));
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

void ImTextureScailing::SetCurrent()
{
	autoscale = molflowGeom->texAutoScale;
	GetCurrentRange();
	minScale = cMinScale;
	maxScale = cMaxScale;
	minInput = std::to_string(minScale);
	maxInput = std::to_string(maxScale);
}

void ImTextureScailing::Apply()
{
	if (!Util::getNumber(&minScale, minInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid minimum value");
		Update();
		return;
	}
	if (!Util::getNumber(&maxScale, maxInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid maximum value");
		Update();
		return;
	}
	if (minScale > maxScale) {
		ImIOWrappers::InfoPopup("Error", "min must be lower than max");
		Update();
		return;
	}
	molflowGeom->texture_limits[molflowGeom->textureMode].manual.min.steady_state = minScale;
	molflowGeom->texture_limits[molflowGeom->textureMode].manual.max.steady_state = maxScale;
	molflowGeom->texAutoScale = false;
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
	ImVec2 availableSpace = ImGui::GetWindowSize();
	ImVec2 availableTLcorner = ImGui::GetWindowPos();

	ImVec2 gradientSize = ImVec2(availableSpace.x*0.8, 2*txtH);

	ImVec2 midpoint = ImMath::add(availableTLcorner, ImMath::scale(availableSpace,0.5));
	ImVec2 TLcorner = ImMath::substract(midpoint, ImMath::scale(gradientSize, 0.5));
	ImVec2 BRcorner = ImMath::add(midpoint, ImMath::scale(gradientSize, 0.5));

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	std::string hoveredVal = "";

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
	ImVec2 mousePos = ImGui::GetMousePos();
	if (mousePos.x < TLcorner.x) mousePos.x = TLcorner.x;
	else if (mousePos.x > BRcorner.x) mousePos.x = BRcorner.x;
	
	if (ImGui::IsWindowHovered() && ImMath::inside(TLcorner, BRcorner, mousePos)) {
		if(!logScale)
			hoveredVal = fmt::format("{:.4f}", Utils::mapRange(mousePos.x,TLcorner.x,BRcorner.x,minScale,maxScale));
		else {
			// log(x) where x <=0 is undefined, clamp scales above 0
			if (minScale < 1e-20) minScale = 1e-20;
			if (maxScale < 1e-20) maxScale = 1e-20;
			/*	Code based on:
				Linear and Logarithmic Interpolation
				Markus Deserno
				Max-Planck-Institut fur� Polymerforschung, Ackermannweg 10, 55128 Mainz, Germany
				https://www.cmu.edu/biolphys/deserno/pdf/log_interpol.pdf
			*/
			long double graphSpaceX = Utils::mapRange(mousePos.x, TLcorner.x, BRcorner.x, minScale, maxScale);
			long double a=graphSpaceX-minScale, b=maxScale-graphSpaceX;
			long double f = a / (a + b); // ratio of distances to gradient edges
			long double val = pow(maxScale,f)*pow(minScale,1-f);
			hoveredVal = fmt::format("{:e}", val);
		}
	}

	ImGui::EndChild();
	ImGui::Text(hoveredVal);
}

void ImTextureScailing::GetCurrentRange()
{
	if (molflowGeom->texAutoScale) {
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
	else {
		if (molflowGeom->texAutoScaleIncludeConstantFlow == 0) {
			cMinScale = molflowGeom->texture_limits[molflowGeom->textureMode].manual.min.moments_only;
			cMaxScale = molflowGeom->texture_limits[molflowGeom->textureMode].manual.max.moments_only;
		}
		else if (molflowGeom->texAutoScaleIncludeConstantFlow == 1) {
			cMinScale = std::min(molflowGeom->texture_limits[molflowGeom->textureMode].manual.min.steady_state, molflowGeom->texture_limits[molflowGeom->textureMode].manual.min.moments_only);
			cMaxScale = std::max(molflowGeom->texture_limits[molflowGeom->textureMode].manual.max.steady_state, molflowGeom->texture_limits[molflowGeom->textureMode].manual.max.moments_only);
		}
		else { // == 2
			cMinScale = molflowGeom->texture_limits[molflowGeom->textureMode].manual.min.steady_state;
			cMaxScale = molflowGeom->texture_limits[molflowGeom->textureMode].manual.max.steady_state;
		}
	}

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
