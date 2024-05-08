
#include "imgui.h"
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

void ImTextureScaling::Draw()
{
	if (!drawn) return;
	
	if (photoMode) {
		ImGui::SetNextWindowSize(ImVec2(50 * txtW, 5 * txtH));
		ImGui::Begin("Legend###TextureScaling", &drawn, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground);
		if (!mApp->whiteBg) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
		}
		DrawGradient();
		if (!mApp->whiteBg) {
			ImGui::PopStyleColor();
		}
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x-3*txtW,txtH));
		ImGui::SameLine();
		ImGui::HelpMarker("Hover me and press ESC to exit photo mode");
		if (ImGui::IsWindowHovered() && ImGui::IsKeyPressed(ImGuiKey_Escape)) photoMode = false;
		ImGui::End();
		return;
	}
	ImGui::SetNextWindowPos(ImVec2(3*txtW, 3*txtH), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(74*txtW,15*txtH));

	ImGui::Begin("Texture Scaling###TextureScaling", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
	
	ImGui::BeginChild("Range", ImVec2(ImGui::GetContentRegionAvail().x - 16.f * txtW, 5.75f * txtH), true);
	ImGui::TextDisabled("Texture Range");
	if (ImGui::BeginTable("##layoutHelper", 4, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Min");
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Max");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(10 * txtW);
		if (autoscale) ImGui::BeginDisabled();
		ImGui::InputText("##minInput", &minInput[showComboVal]);
		if (autoscale) ImGui::EndDisabled();
		if (ImGui::IsItemFocused() && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))) {
			autoscale = false;
			ApplyButtonPress();
		}
		ImGui::SetNextItemWidth(10 * txtW);
		if (autoscale) ImGui::BeginDisabled();
		ImGui::InputText("##maxInput", &maxInput[showComboVal]);
		if (autoscale) ImGui::EndDisabled();
		if (ImGui::IsItemFocused() && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))) {
			autoscale = false;
			ApplyButtonPress();
		}
		ImGui::TableNextColumn();
		if (ImGui::Checkbox("Autoscale", &autoscale)) {
			molflowGeom->texAutoScale = autoscale;
			WorkerUpdate();
		}
		ImGui::SetNextItemWidth(txtW * 20);
		if (!autoscale) ImGui::BeginDisabled();
		if (ImGui::BeginCombo("##includeCombo", includeComboLabels[includeComboVal])) {
			for (short i = 0; i < 3; i++) {
				if (ImGui::Selectable(includeComboLabels[i])) {
					includeComboVal = static_cast<AutoScaleMode>(i);
					molflowGeom->texAutoScaleMode = includeComboVal;
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
	if (ImGui::Button("Apply min/max")) ApplyButtonPress();
	ImGui::SameLine();
	ImGui::Text("Swap: "+swapText);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("Geometry", ImVec2(ImGui::GetContentRegionAvail().x, 5.75f * txtH), true);
	ImGui::TextDisabled("Geometry"); ImGui::SameLine();
	ImGui::HelpMarker("These are the values obtained from geometry\nthey are used by autoscale");
	GetCurrentRange();
	ImGui::Text(fmt::format("Min: {:.3g}", cMinScale));
	ImGui::Text(fmt::format("Max: {:.3g}", cMaxScale));
	if (ImGui::Button("Copy to manual")) SetCurrentButtonPress();
	ImGui::EndChild();

	ImGui::BeginChild("Gradient", ImVec2(0.f, ImGui::GetContentRegionAvail().y-1.5f*txtH), true);
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
	ImGui::SameLine();
	if (ImGui::Button("Photo Mode")) photoMode = true;
	ImGui::End();
}

void ImTextureScaling::Init(Interface* mApp_)
{
	mApp = mApp_;
	interfGeom = mApp->worker.GetGeometry();
	molflowGeom = mApp->worker.GetMolflowGeometry();
	colorMap = GenerateColorMap();
	Update(); // will get the toggle and combo states and min/max values from the molflowGeom object
}

void ImTextureScaling::Load()
{
	minScale[AutoscaleMomentsOnly] = interfGeom->texture_limits[AutoscaleMomentsOnly].manual.min.steady_state;
	minScale[AutoscaleMomentsAndConstFlow] = interfGeom->texture_limits[AutoscaleMomentsAndConstFlow].manual.min.steady_state;
	minScale[AutoscaleConstFlow] = interfGeom->texture_limits[AutoscaleConstFlow].manual.min.steady_state;

	maxScale[AutoscaleMomentsOnly] = interfGeom->texture_limits[AutoscaleMomentsOnly].manual.max.steady_state;
	maxScale[AutoscaleMomentsAndConstFlow] = interfGeom->texture_limits[AutoscaleMomentsAndConstFlow].manual.max.steady_state;
	maxScale[AutoscaleConstFlow] = interfGeom->texture_limits[AutoscaleConstFlow].manual.max.steady_state;

	minInput[AutoscaleMomentsOnly] = fmt::format("{}", minScale[AutoscaleMomentsOnly]);
	minInput[AutoscaleMomentsAndConstFlow] = fmt::format("{}", minScale[AutoscaleMomentsAndConstFlow]);
	minInput[AutoscaleConstFlow] = fmt::format("{}", minScale[AutoscaleConstFlow]);

	maxInput[AutoscaleMomentsOnly] = fmt::format("{}", maxScale[AutoscaleMomentsOnly]);
	maxInput[AutoscaleMomentsAndConstFlow] = fmt::format("{}", maxScale[AutoscaleMomentsAndConstFlow]);
	maxInput[AutoscaleConstFlow] = fmt::format("{}", maxScale[AutoscaleConstFlow]);
}

void ImTextureScaling::UpdateSize()
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
void ImTextureScaling::Update() {
	GetCurrentRange();
	autoscale = molflowGeom->texAutoScale;
	includeComboVal = molflowGeom->texAutoScaleMode;
	logScale = molflowGeom->texLogScale;
	colors = molflowGeom->texColormap;
	showComboVal = molflowGeom->textureMode;
	UpdateSize();

	minInput[showComboVal] = std::to_string(minScale[showComboVal]);
	maxInput[showComboVal] = std::to_string(maxScale[showComboVal]);

}

void ImTextureScaling::SetCurrentButtonPress()
{
	GetCurrentRange();
	minInput[showComboVal] = fmt::format("{:.3g}", cMinScale);
	maxInput[showComboVal] = fmt::format("{:.3g}", cMaxScale);
	autoscale = false;
	ApplyButtonPress();
}

void ImTextureScaling::ApplyButtonPress()
{
	molflowGeom->texAutoScaleMode = includeComboVal;
	molflowGeom->texAutoScale = autoscale;
	if (autoscale) {} // empty if statement, acts like adding !autoscale && to all following else statements
	else if (!Util::getNumber(&minScale[showComboVal], minInput[showComboVal])) {
		ImIOWrappers::InfoPopup("Error", "Invalid minimum value");
		Update();
		return;
	}
	else if (!Util::getNumber(&maxScale[showComboVal], maxInput[showComboVal])) {
		ImIOWrappers::InfoPopup("Error", "Invalid maximum value");
		Update();
		return;
	}
	else if (minScale[showComboVal] > maxScale[showComboVal]) {
		ImIOWrappers::InfoPopup("Error", "min must be lower than max");
		Update();
		return;
	}
	else {
		molflowGeom->texture_limits[molflowGeom->textureMode].manual.min.steady_state = minScale[showComboVal];
		molflowGeom->texture_limits[molflowGeom->textureMode].manual.max.steady_state = maxScale[showComboVal];
	}

	WorkerUpdate();
}

bool ImTextureScaling::WorkerUpdate()
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

void ImTextureScaling::DrawGradient()
{
	ImGui::BeginChild("##ImGradient", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 1 * txtH));
	ImVec2 availableSpace = ImMath::SubstractVec2(ImGui::GetWindowSize(), ImVec2(0, 0));
	ImVec2 availableTLcorner = ImGui::GetWindowPos();

	ImVec2 gradientSize = ImVec2(availableSpace.x*0.8f, 1.5f*txtH);

	ImVec2 midpoint = ImMath::AddVec2(availableTLcorner, ImVec2(availableSpace.x*0.5f, availableSpace.y*0.4f));
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

	static double gradientMinScale = 0, gradientMaxScale = 1;
	if (autoscale) {
		if (cMinScale < cMaxScale) {
			gradientMinScale = cMinScale;
			gradientMaxScale = cMaxScale;
		}
	}
	else {
		gradientMinScale = minScale[showComboVal];
		gradientMaxScale = maxScale[showComboVal];
	}

	for (const auto& tick : majorTicVals) {
		double val = MathHelper::mapRange(tick, TLcorner.x, BRcorner.x, gradientMinScale, gradientMaxScale);
		if (logScale) val = logScaleInterpolate(val, gradientMinScale, gradientMaxScale);

		std::string text = fmt::format("{:.2e}", val);
		ImVec2 textSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0, text.c_str());
		drawList->AddText(ImVec2(static_cast<float>(tick) - textSize.x / 2.f, BRcorner.y), mApp->whiteBg || !photoMode ? colorMap[0] : ImGui::GetColorU32(IM_COL32(255, 255, 255, 255)), text.c_str());
		drawList->AddRectFilled(ImVec2(static_cast<float>(tick), (midpoint.y + BRcorner.y) / 2.f), ImVec2(static_cast<float>(tick + 1), BRcorner.y), colorMap[0]);
	}
	if (!photoMode) {
		// handle hovering
		ImVec2 mousePos = ImGui::GetMousePos();
		static ImVec2 hoverMarkPos = ImMath::SubstractVec2(mousePos, midpoint);
		if (mousePos.x < TLcorner.x) mousePos.x = TLcorner.x;
		else if (mousePos.x > BRcorner.x) mousePos.x = BRcorner.x;

		if (ImGui::IsWindowHovered() && ImMath::IsInsideVec2(TLcorner, BRcorner, mousePos)) {
			double linX = MathHelper::mapRange(mousePos.x, TLcorner.x, BRcorner.x, gradientMinScale, gradientMaxScale);
			if(!logScale)
				hoveredVal = fmt::format("{:.3g}", linX);
			else {
				double val = logScaleInterpolate(linX, gradientMinScale, gradientMaxScale);
				hoveredVal = fmt::format("{:.3g}", val);
			}
			hoverMarkPos = ImMath::SubstractVec2(mousePos, midpoint); // get mousePos relative to midpoint
			//this is needed so when not hovered and window is moved the marker stays in the same place relative to the gradient
		}
		// draws a vertical line under the mouse cursors position
		ImVec2 posTmp = ImMath::AddVec2(hoverMarkPos, midpoint); // get absolute position of marker
		drawList->AddRectFilled(ImVec2(posTmp.x,TLcorner.y-5), ImVec2(posTmp.x+1,BRcorner.y+5), colorMap[0]);
	}

	ImGui::EndChild();
	if (!photoMode) ImGui::Text(hoveredVal);
	else ImGui::Text("");
}

void ImTextureScaling::GetCurrentRange()
{
	auto [min, max] = molflowGeom->GetTextureAutoscaleMinMax();
	cMinScale = min;
	cMaxScale = max;
}

double ImTextureScaling::logScaleInterpolate(double x, double leftTick, double rightTick)
{
	// log(x) where x <=0 is undefined, clamp scales above 0
	if (leftTick < 1e-20) leftTick = 1e-20;
	if (rightTick < 1e-20) rightTick = 1e-20;
	/*	Code based on:
		Linear and Logarithmic Interpolation
		Markus Deserno
		Max-Planck-Institut fur� Polymerforschung, Ackermannweg 10, 55128 Mainz, Germany
		https://www.cmu.edu/biolphys/deserno/pdf/log_interpol.pdf (accessed 21-11-2023)
	*/
	double a = x - leftTick, b = rightTick - x;
	double f = a / (a + b); // ratio of distances to gradient edges
	double val = pow(rightTick, f) * pow(leftTick, 1 - f);
	return val;
}

std::vector<ImU32> ImTextureScaling::GenerateColorMap()
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
