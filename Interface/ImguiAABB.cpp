//
// Created by pbahr on 8/2/21.
//

#include "ImguiAABB.h"
#include "imgui/imgui.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
extern SynRad*mApp;
#endif

void ShowAABB(MolFlow *mApp, bool *show_aabb, bool &redrawAabb, bool &rebuildAabb) {
    ImGui::PushStyleVar(
            ImGuiStyleVar_WindowMinSize,
            ImVec2(400.0f, 0.0f)); // Lift normal size constraint, however the presence of
    // a menu-bar will give us the minimum height we want.

    ImGui::Begin(
            "AABB viewer", show_aabb,
            ImGuiWindowFlags_NoSavedSettings); // Pass a pointer to our bool

    ImGui::PopStyleVar(1);

    ImGui::DragIntRange2("Show AABB level", &mApp->aabbVisu.showLevelAABB[0], &mApp->aabbVisu.showLevelAABB[1], 1.0f,
                         -1, 10);
    ImGui::SliderFloat("AABB alpha", &mApp->aabbVisu.alpha, 0.0f, 1.0f);
    ImGui::Checkbox("Show left  branches", &mApp->aabbVisu.showBranchSide[0]);
    ImGui::Checkbox("Show right branches", &mApp->aabbVisu.showBranchSide[1]);
    ImGui::Checkbox("Show AABB leaves", &mApp->aabbVisu.showAABBLeaves);
    ImGui::Checkbox("Reverse Expansion", &mApp->aabbVisu.reverseExpansion);

    ImGui::Checkbox("Draw all structures", &mApp->aabbVisu.drawAllStructs);
    ImGui::Checkbox("Use old BVH", &mApp->aabbVisu.oldBVH);
    const char *items[] = {"SAH", "HLBVH", "Middle", "EqualCounts", "MolflowSplit", "ProbSplit"};
    if (ImGui::BeginListBox("Splitting technique")) {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
            const bool is_selected = ((int) mApp->aabbVisu.splitTechnique == n);
            if (ImGui::Selectable(items[n], is_selected))
                mApp->aabbVisu.splitTechnique = (BVHAccel::SplitMethod) n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndListBox();
    }

    if (ImGui::Button("Apply aabb view"))
        redrawAabb = true;

    if (ImGui::Button("Build new AABB"))
        rebuildAabb = true;

}