#include "ImguiSelectTextureType.h"
#include "imgui.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Helper/StringHelper.h"
#include "ImguiWindow.h"
#include "ImguiExtensions.h"
#include "imgui_internal.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"
#include "Helper/MathTools.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

#if defined(SYNRAD)
extern SynRad* mApp;
#include "../src/SynRad.h"
#endif

ImSelectTextureType::ImSelectTextureType()
{
	drawn = false;
	squareTextrueCheck = 2;
	mode = none;
	exactlyCheck = 0;
	betweenCheck = 0;
	desorbtionCheck = 2;
	absorbtionCheck = 2;
	reflectionCheck = 2;
	transpPassCheck = 2;
	directionCheck = 2;
	exactlyInput = "";
	minInput = "";
	maxInput = "";
	select = [](int src) {
		mApp->imWnd->selByTex.Preprocess();
		InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();

		if (src == btnSelect) interfGeom->UnselectAll();
		for (size_t i = 0; i < interfGeom->GetNbFacet(); i++) {
			InterfaceFacet* f = interfGeom->GetFacet(i);
			bool match = f->sh.isTextured;
			if (mApp->imWnd->selByTex.squareTextrueCheck != 2) match = match && ((mApp->imWnd->selByTex.squareTextrueCheck == 1) == IsEqual(f->tRatioU, f->tRatioV));
			if (mApp->imWnd->selByTex.mode == exactly) match = match && IsEqual(mApp->imWnd->selByTex.exactlyValue, f->tRatioU) || IsEqual(mApp->imWnd->selByTex.exactlyValue, f->tRatioV);
			if (mApp->imWnd->selByTex.mode == between) match = match && ((mApp->imWnd->selByTex.minValue <= f->tRatioU) && (f->tRatioU <= mApp->imWnd->selByTex.maxValue)) || ((mApp->imWnd->selByTex.minValue <= f->tRatioV) && (f->tRatioV <= mApp->imWnd->selByTex.maxValue));
			#if defined(MOLFLOW)
			if (mApp->imWnd->selByTex.desorbtionCheck != 2) match = match && f->sh.countDes;
			#endif
			if (mApp->imWnd->selByTex.absorbtionCheck != 2) match = match && (mApp->imWnd->selByTex.absorbtionCheck == 1) == f->sh.countAbs;
			if (mApp->imWnd->selByTex.reflectionCheck != 2) match = match && (mApp->imWnd->selByTex.reflectionCheck == 1) == f->sh.countRefl;
			if (mApp->imWnd->selByTex.transpPassCheck != 2) match = match && (mApp->imWnd->selByTex.transpPassCheck == 1) == f->sh.countTrans;
			if (mApp->imWnd->selByTex.directionCheck != 2) match = match && (mApp->imWnd->selByTex.directionCheck == 1) && f->sh.countDirection;

			if (match) f->selected = (src != rmvSelect);
		}
		interfGeom->UpdateSelection();
		mApp->UpdateFacetParams(true);
		mApp->UpdateFacetlistSelected();
	};
}

void ImSelectTextureType::Show()
{
	drawn = true;
}

void ImSelectTextureType::Preprocess() {
	mode = none;
	if (exactlyCheck) mode = exactly;
	if (betweenCheck) mode = between;
	if (mode == exactly) {
		if (!Util::getNumber(&exactlyValue, exactlyInput)) {
			mApp->imWnd->popup.Open("Error", "Invaluid value in input field", { std::make_shared<MyButtonInt>("Ok",buttonOk,ImGui::keyEnter) });
			return;
		}
	}
	else if (mode == between) {
		if (!Util::getNumber(&minValue, minInput) || !Util::getNumber(&maxValue, maxInput)) {
			mApp->imWnd->popup.Open("Error", "Invaluid value in input field", { std::make_shared<MyButtonInt>("Ok",buttonOk,ImGui::keyEnter) });
			return;
		}
		if (minValue > maxValue) {
			mApp->imWnd->popup.Open("Error", "Minimum cannot be greater than maximum", { std::make_shared<MyButtonInt>("Ok",buttonOk,ImGui::keyEnter) });
			return;
		}
	}
}
void ImSelectTextureType::Draw()
{
	if (!drawn) return;
	float txtW = ImGui::CalcTextSize(" ").x;
	float txtH = ImGui::GetTextLineHeightWithSpacing();
	//ImGui::SetNextWindowSize(ImVec2(txtW*85, txtH*20));
	if (ImGui::Begin("Select facets by texture properties [BETA - WIP]", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
		if (ImGui::BeginChild("Texture resolution", ImVec2(0, txtH * 7.5), true, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextDisabled("Texture resolution");
			ImGui::TriState("Square texture", &squareTextrueCheck);
			ImGui::TextWrapped("For non-square textures, condition applies to either of the two dimensions:");
			if (ImGui::BeginTable("##SFBTPtable", 2, ImGuiTableFlags_SizingFixedFit)) {
				// TODO this window needs a 3 state checkbox where 0 = not checked, 1 = full checked, 2 = half checked
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::Checkbox("Exactly", &exactlyCheck)) {
					if (exactlyCheck) betweenCheck = false;
				}
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(txtW * 15);
				if (ImGui::InputText("##3", &exactlyInput)) {
					mode = exactly;
				}
				ImGui::SameLine();
				ImGui::Text("/cm");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::Checkbox("Between", &betweenCheck)) {
					if (betweenCheck) exactlyCheck = false;
				}
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(txtW * 15);
				if (ImGui::InputText("##4", &minInput)) {
					mode = between;
				}
				ImGui::SameLine();
				ImGui::Text("and");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(txtW * 15);
				if (ImGui::InputText("##5", &maxInput)) {
					mode = between;
				}
				ImGui::SameLine();
				ImGui::Text("/cm");
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
		if (ImGui::BeginChild("##TextureType", ImVec2(0, txtH * 6), true)) {
			ImGui::TextDisabled("Texture type");
			if (ImGui::BeginTable("##SFBTPtable2", 2)) {
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, true);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TriState("Count desorbtion", &desorbtionCheck);
				ImGui::TableSetColumnIndex(1);
				ImGui::TriState("Count reflection", &reflectionCheck);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TriState("Count absorbtion", &absorbtionCheck);
				ImGui::TableSetColumnIndex(1);
				ImGui::TriState("Count transp. pass", &transpPassCheck);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(1);
				ImGui::TriState("Count direction", &directionCheck);
				ImGui::PopItemFlag();
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
		if (ImGui::Button("  Select  ")) {
			select(btnSelect);
		} ImGui::SameLine();
		if (ImGui::Button("  Add to sel.  ")) {
			select(addSelect);
		} ImGui::SameLine();
		if (ImGui::Button("  Remove from sel.  ")) {
			select(rmvSelect);
		} ImGui::SameLine();
	}
	ImGui::End();
}
