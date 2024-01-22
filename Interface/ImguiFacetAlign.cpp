#include "ImguiFacetAlign.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImFacetAlign::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowPos(ImVec2(txtW * 5, txtH * 3), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 20.25));
	ImGui::Begin("Align selected facets to an other", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
	ImGui::BeginChild("###ASFTAO-1", ImVec2(0, txtH * 4), true);
	ImGui::TextDisabled("Step 1: select facets of the object");
	ImGui::Text(fmt::format("{} facets will be aligned", toAlign.size()));
	if (ImGui::Button("Update from selection")) {

	}
	ImGui::EndChild();
	ImGui::BeginChild("###ASFTAO-2", ImVec2(0, txtH * 7), true);
	ImGui::TextDisabled("Step 2: select snapping facets & points");
	ImGui::TextWrapped("1. Choose two facets that will be snapped together.\n\n2. Choose 2-2 vertices on the source and destination \nfacets: One will serve as an anchor point, one as a\ndirection aligner. Once you have 2 facets and 4\nvertices selected, proceed to step 3.");
	ImGui::EndChild();
	ImGui::BeginChild("###ASFTAO-3", ImVec2(0, txtH * 5.5), true);
	ImGui::TextDisabled("Step 3: align");
	ImGui::Checkbox("Invert normal", &invertNormal);
	ImGui::Checkbox("Swap anchor/direction vertices on source", &swapOnSource);
	ImGui::Checkbox("Swap anchor/direction vertices on destination", &swapOnDestination);
	ImGui::EndChild();
	if (ImGui::Button("Align")) {

	}
	ImGui::SameLine();
	if (ImGui::Button("Undo")) {

	}
	ImGui::End();
}
