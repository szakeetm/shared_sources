//
// Created by pbahr on 11/9/21.
//

#include <vector>
#include <imgui.h>
#include <GeometryTools.h>
#include "ImguiShowNeighbors.h"
#include "fmt/ranges.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

void ShowNeighborSelect(MolFlow *mApp, bool *show_select){
    ImGui::Begin("Select neighbors", show_select); // Create a window called "Hello, world!"
    // and append into it.

    static float deg = 90.0f;
    ImGui::SliderFloat("Degrees to select", &deg, 0.0f, 360.0f);

    static bool selectedForSelection = false;
    static int toBeSelected = -1;
    ImGui::Checkbox("Select only for given facets", &selectedForSelection);
    if(selectedForSelection){
        static size_t item_current_idx = 0; // Here we store our selection data as an index.
        if (ImGui::BeginListBox("##selected box"))
        {
            auto selectedFac = mApp->worker.GetGeometry()->GetSelectedFacets();
            for (size_t n = 0; n < mApp->worker.GetGeometry()->GetNbSelectedFacets(); n++)
            {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(fmt::format("{}",selectedFac[n] + 1).c_str(), is_selected))
                    item_current_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        int selectedFacet_p1 = toBeSelected +1;
        if(ImGui::SliderInt("Select for facet #", &selectedFacet_p1, 1, (int) mApp->worker.GetGeometry()->GetNbFacet()))
            toBeSelected = selectedFacet_p1 - 1;

        if(ImGui::Button("Use current selection (first only)")){
            if(mApp->worker.GetGeometry()->GetNbSelectedFacets() > 0){
                toBeSelected = (int) mApp->worker.GetGeometry()->GetSelectedFacets().front();
            }
        }
    }

    static bool smallerThan = true;
    ImGui::Checkbox("For Angle smaller than?", &smallerThan);
    if(ImGui::Button("Select facets with sharp angled neighbors")){

        std::vector<size_t> selectedFacets;
        std::vector<CommonEdge> edges;

        auto currentSelection = mApp->worker.GetGeometry()->GetSelectedFacets();
        if(GeometryTools::GetAnalysedCommonEdges(mApp->worker.GetGeometry(), edges)) {
            for (auto &edge: edges) {
                if(!selectedForSelection || (selectedForSelection && (toBeSelected == edge.facetId[0] || toBeSelected == edge.facetId[1]))) {
                    // And when either small angle or bigger angle is found
                    if (smallerThan && edge.angle < deg / 180.0 * 3.14159
                        || !smallerThan && edge.angle >= deg/180.0*3.14159) {
                        selectedFacets.push_back(edge.facetId[0]);
                        selectedFacets.push_back(edge.facetId[1]);
                    }
                }
            }
        }
        mApp->worker.GetGeometry()->SetSelection(selectedFacets, false, false);
    }
    ImGui::End();
}