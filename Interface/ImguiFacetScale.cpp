#include "ImguiFacetScale.h"

void ImFacetScale::Draw()
{
	if (!drawn) return;
	ImGui::Begin("Scale selected facets", &drawn);
	ImGui::End();
}
