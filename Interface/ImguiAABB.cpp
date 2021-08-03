//
// Created by pbahr on 8/2/21.
//

#include "ImguiAABB.h"
#include "imgui/imgui.h"
#include <imgui/imgui_internal.h>

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
extern SynRad*mApp;
#endif

// Dummy data structure that we use for the Table demo.
// (pre-C++11 doesn't allow us to instantiate ImVector<MyItem> template if this structure if defined inside the demo function)
namespace
{
    // We are passing our own identifier to TableSetupColumn() to facilitate identifying columns in the sorting code.
    // This identifier will be passed down into ImGuiTableSortSpec::ColumnUserID.
    // But it is possible to omit the user id parameter of TableSetupColumn() and just use the column index instead! (ImGuiTableSortSpec::ColumnIndex)
    // If you don't use sorting, you will generally never care about giving column an ID!
    enum BoxDataColumnID {
        BoxDataColumnID_ID,
        BoxDataColumnID_Chance,
        BoxDataColumnID_Prims,
        BoxDataColumnID_Level

    };

    struct BoxData {
        int ID;
        double chance;
        int prims;
        int level;
        
        static const ImGuiTableSortSpecs* s_current_sort_specs;

        // Compare function to be used by qsort()
        static int IMGUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
        {
            const BoxData* a = (const BoxData*)lhs;
            const BoxData* b = (const BoxData*)rhs;
            for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
            {
                // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
                int delta = 0;
                switch (sort_spec->ColumnUserID)
                {
                    case BoxDataColumnID_ID:             delta = (a->ID - b->ID); break;
                    case BoxDataColumnID_Chance:           delta = (a->chance > b->chance) ? 1 : (a->chance == b->chance) ? 0 : -1;     break;
                    case BoxDataColumnID_Prims:             delta = (a->prims - b->prims); break;
                    case BoxDataColumnID_Level:             delta = (a->level - b->level); break;

                    default: IM_ASSERT(0); break;
                }
                if (delta > 0)
                    return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
                if (delta < 0)
                    return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
            }

            // qsort() is instable so always return a way to differenciate items.
            // Your own compare function may want to avoid fallback on implicit sort specs e.g. a Name compare if it wasn't already part of the sort specs.
            return (a->ID - b->ID);
        }
    };
    const ImGuiTableSortSpecs* BoxData::s_current_sort_specs = NULL;
}

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
    ImGui::Checkbox("Apply same color", &mApp->aabbVisu.sameColor);

    ImGui::Checkbox("Draw all structures", &mApp->aabbVisu.drawAllStructs);
    ImGui::Checkbox("Use old BVH", &mApp->aabbVisu.oldBVH);
    ImGui::SliderInt("BVH width", &mApp->worker.model->wp.bvhWidth, 0, 32);

    const char *items[] = {"SAH", "HLBVH", "Middle", "EqualCounts", "MolflowSplit", "ProbSplit"};
    if (ImGui::BeginListBox("Splitting technique")) {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
            const bool is_selected = ((int) mApp->aabbVisu.splitTechnique == n);
            if (ImGui::Selectable(items[n], is_selected)) {
                mApp->aabbVisu.splitTechnique = (BVHAccel::SplitMethod) n;
                mApp->worker.model->wp.splitMethod = n;
            }

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

    
    // Create item list
    static ImVector<BoxData> tabItems;
    auto& bvhs = mApp->worker.model->bvhs;
    if (/*tabItems.empty() && */!bvhs.empty() && tabItems.size() != bvhs.front().ints.size())
    {
        auto& bvh = bvhs.front();
        size_t ii = 0;
        tabItems.resize(bvh.ints.size(), BoxData());
        for (int n = 0; n < tabItems.Size; n++)
        {
            auto& f = bvh.ints[n];
            BoxData& item = tabItems[n];
            item.ID = n;
            item.chance =  (f.nbChecks > 0) ? (double)f.nbIntersects / (double)f.nbChecks : 0.0;
            item.prims =  f.nbPrim;
            item.level =  f.level;
        }
    }
    else if(mApp->worker.IsRunning()){
        auto& bvh = bvhs.front();
        for (int n = 0; n < tabItems.Size; n++)
        {
            BoxData& item = tabItems[n];
            auto& f = bvh.ints[item.ID];
            item.chance =  (f.nbChecks > 0) ? (double)f.nbIntersects / (double)f.nbChecks : 0.0;
        }
    }

    static ImGuiTableFlags tFlags =
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit |
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
            ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
            ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
            ImGuiTableFlags_Sortable;

    if (ImGui::BeginTable("boxlist", 4, tFlags)) {
        ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 0.0f, BoxDataColumnID_ID);
        ImGui::TableSetupColumn("Chance", ImGuiTableColumnFlags_WidthStretch, 0.0f, BoxDataColumnID_Chance);
        ImGui::TableSetupColumn("#Prims", ImGuiTableColumnFlags_WidthStretch, 0.0f, BoxDataColumnID_Prims);
        ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthStretch, 0.0f, BoxDataColumnID_Level);
        ImGui::TableHeadersRow();

        // Sort our data if sort specs have been changed!
        if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
            if (sorts_specs->SpecsDirty){
                BoxData::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
                if (tabItems.Size > 1)
                    qsort(&tabItems[0], (size_t)tabItems.Size, sizeof(tabItems[0]), BoxData::CompareWithSortSpecs);
                BoxData::s_current_sort_specs = NULL;
                sorts_specs->SpecsDirty = false;
            }

        // Demonstrate using clipper for large vertical lists
        ImGuiListClipper clipper;
            clipper.Begin(tabItems.size());
            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                    BoxData* item = &tabItems[i];
                    //ImGui::PushID(item->ID);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);

                    ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                    bool selected = mApp->aabbVisu.selectedNode == item->ID;
                    char label[32];
                    sprintf(label, "%d", item->ID);
                    if (ImGui::Selectable(label, selected, selectable_flags, ImVec2(0, 0)))
                    {
                        mApp->aabbVisu.selectedNode = item->ID;
                    }

                    //ImGui::Text("%zu", item->ID);
                    ImGui::TableNextColumn();
                    ImGui::Text("%f", item->chance);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", item->prims);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", item->level);
                    //ImGui::PopID();
                }
            }
            ImGui::EndTable();
    }
}