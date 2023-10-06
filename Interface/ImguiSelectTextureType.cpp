#include "ImguiSelectTextureType.h"
#include "imgui.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Helper/StringHelper.h"
#include "ImguiWindow.h"
#include "ImguiExtensions.h"
#include "imgui_internal.h"

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
	squareTextrueCheck = 1;
	mode = exactly;
	desorbtionCheck = 1;
	absorbtionCheck = 1;
	reflectionCheck = 1;
	transpPassCheck = 1;
	directionCheck = 1;
	exactlyInput = "";
	minInput = "";
	maxInput = "";
	select = []() {
		mApp->imWnd->selByTex.Preprocess();
	};
	addSelect = []() {
		mApp->imWnd->selByTex.Preprocess();
	};
	rmvSelect = []() {
		mApp->imWnd->selByTex.Preprocess();
	};
}

void ImSelectTextureType::Show()
{
	drawn = true;
}

void ImSelectTextureType::Preprocess() {
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
				ImGui::RadioButton("Exactly", &mode, exactly);
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(txtW * 15);
				ImGui::InputText("##3", &exactlyInput);
				ImGui::SameLine();
				ImGui::Text("/cm");
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::RadioButton("Between", &mode, between);
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(txtW * 15);
				ImGui::InputText("##4", &minInput);
				ImGui::SameLine();
				ImGui::Text("and");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(txtW * 15);
				ImGui::InputText("##5", &maxInput);
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
			select();
		} ImGui::SameLine();
		if (ImGui::Button("  Add to sel.  ")) {
			addSelect();
		} ImGui::SameLine();
		if (ImGui::Button("  Remove from sel.  ")) {
			rmvSelect();
		} ImGui::SameLine();
	}
	ImGui::End();
}
