#include "ImguiTextureScaling.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"
#include "Helper/MathTools.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "Interface.h"

void ImTextureScailing::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowPos(ImVec2(3*txtW, 3*txtH), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(72*txtW,15*txtH),ImVec2(1000*txtW,100*txtH));
	
	ImGui::Begin("Texture Scailing", &drawn, ImGuiWindowFlags_NoSavedSettings);
	
	ImGui::BeginChild("Range", ImVec2(ImGui::GetContentRegionAvail().x - 15 * txtW, 6 * txtH), true);
	ImGui::TextDisabled("Texture Range");
	ImGui::BeginTable("##layoutHelper", 4, ImGuiTableFlags_SizingStretchProp);
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
	}
	ImGui::EndTable();
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
	ImGui::Text(fmt::format("Min: {:.3f}", minScale));
	ImGui::Text(fmt::format("Max: {:.3f}", maxScale));
	ImGui::EndChild();

	ImGui::BeginChild("Gradient", ImVec2(0, ImGui::GetContentRegionAvail().y-1.5*txtH), true);
	ImGui::TextDisabled("Gradient");
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
	__super::Init(mApp_);
	molflowGeom = mApp->worker.GetMolflowGeometry();
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
	if (molflowGeom->texAutoScaleIncludeConstantFlow == 0) {
		minScale =
			molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.moments_only;
		maxScale =
			molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.moments_only;
	}
	else if (molflowGeom->texAutoScaleIncludeConstantFlow == 1) {
		minScale =
			std::min(molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.steady_state, molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.moments_only);
		maxScale =
			std::max(molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.steady_state, molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.moments_only);
	}
	else { // == 2
		minScale =
			molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.steady_state;
		maxScale =
			molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.steady_state;
	}
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
	if (molflowGeom->texAutoScaleIncludeConstantFlow == 0) {
		molflowGeom->texture_limits[molflowGeom->textureMode].manual.min.steady_state =
			molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.moments_only;
		molflowGeom->texture_limits[molflowGeom->textureMode].manual.max.steady_state =
			molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.moments_only;
	}
	else if (molflowGeom->texAutoScaleIncludeConstantFlow == 1) {
		molflowGeom->texture_limits[molflowGeom->textureMode].manual.min.steady_state =
			std::min(molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.steady_state, molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.moments_only);
		molflowGeom->texture_limits[molflowGeom->textureMode].manual.max.steady_state =
			std::max(molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.steady_state, molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.moments_only);
	}
	else { // == 2
		molflowGeom->texture_limits[molflowGeom->textureMode].manual.min.steady_state =
			molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.min.steady_state;
		molflowGeom->texture_limits[molflowGeom->textureMode].manual.max.steady_state =
			molflowGeom->texture_limits[molflowGeom->textureMode].autoscale.max.steady_state;
	}
	minInput = minScale;
	maxInput = maxScale;
	autoscale = false;
	molflowGeom->texAutoScale = false;
	WorkerUpdate();
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
	if (!WorkerUpdate()) return;
}

bool ImTextureScailing::WorkerUpdate()
{
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
