//
// Created by pbahr on 8/2/21.
//

#include "ImguiAABB.h"
#include "imgui/imgui.h"
#include <imgui/imgui_internal.h>
#include "transfer_function_widget.h"

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
    
    enum FacetDataColumnID {
        FacetDataColumnID_ID,
        FacetDataColumnID_Steps
    };

    struct FacetData {
        int ID;
        int steps;
        
        static const ImGuiTableSortSpecs* s_current_sort_specs;

        // Compare function to be used by qsort()
        static int IMGUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
        {
            const FacetData* a = (const FacetData*)lhs;
            const FacetData* b = (const FacetData*)rhs;
            for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
            {
                // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
                int delta = 0;
                switch (sort_spec->ColumnUserID)
                {
                    case FacetDataColumnID_ID:             delta = (a->ID - b->ID); break;
                    case FacetDataColumnID_Steps:           delta = (a->steps > b->steps) ? 1 : (a->steps == b->steps) ? 0 : -1;     break;
                    
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
    const ImGuiTableSortSpecs* FacetData::s_current_sort_specs = NULL;
}

ImguiAABBVisu::ImguiAABBVisu(){
    // A texture so we can color the background of the window by the colormap
    GLuint colormap_texture;
    glGenTextures(1, &colormap_texture);
    glBindTexture(GL_TEXTURE_1D, colormap_texture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    auto colormap = tfn_widget.get_colormap();
    glTexImage1D(GL_TEXTURE_1D,
                 0,
                 GL_RGBA8,
                 colormap.size() / 4,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 colormap.data());
}

void ImguiAABBVisu::ShowAABB(MolFlow *mApp, bool *show_aabb, bool &redrawAabb, bool &rebuildAabb) {
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
    ImGui::Checkbox("Render colors based on hit stats", &mApp->aabbVisu.showStats);
    ImGui::Checkbox("Use traversal step heatmap", &mApp->aabbVisu.travStep);

    if (tfn_widget.changed()) {
        auto colormap = tfn_widget.get_colormap();
        glTexImage1D(GL_TEXTURE_1D,
                     0,
                     GL_RGBA8,
                     colormap.size() / 4,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     colormap.data());
    }
    if(ImGui::SliderFloat2("Hit stat threshold", (float*)&mApp->aabbVisu.trimByProb, 0.0f, 1.0f, "%.2f")){
        mApp->aabbVisu.trimRange = mApp->aabbVisu.trimByProb[1] - mApp->aabbVisu.trimByProb[0];
    }

    ImGui::Checkbox("Draw all structures", &mApp->aabbVisu.drawAllStructs);
    ImGui::Checkbox("Use old BVH", &mApp->aabbVisu.oldBVH);
    ImGui::SliderInt("BVH width", &mApp->worker.model->wp.bvhMaxPrimsInNode, 0, 32);

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

    static bool color_win = false;
    if(ImGui::Button("Edit colormap")) {
        color_win = !color_win;
    }

    if(color_win) {
        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowSize(ImVec2(0, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0)); // Lift normal size constraint

        if (ImGui::Begin("Choose colormap"),
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_ChildWindow) {
            if (ImGui::Button("Apply")) {
                mApp->aabbVisu.colorMap = tfn_widget.get_colormapf();
                redrawAabb = true;
                color_win = false;
            }
            if (ImGui::Button("Close")) {
                color_win = false;
            }
            tfn_widget.draw_ui();
            ImGui::End();
        }
        ImGui::PopStyleVar(); // Lift normal size constraint}
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

    // Create item list
    static ImVector<FacetData> facItems;
    auto& facets = mApp->worker.model->facets;
    if (!facets.empty() && facItems.size() != facets.size())
    {
        auto& fac = facets;
        facItems.resize(facets.size(), FacetData());

        mApp->aabbVisu.trimByProb[0] = 1e99;
        mApp->aabbVisu.trimByProb[1] = 0;
        for (int n = 0; n < facItems.Size; n++)
        {
            auto& f = facets[n];
            FacetData& item = facItems[n];
            item.ID = n;
            item.steps =  (f->nbTraversalSteps > 0) ? (double)f->nbTraversalSteps / (double)f->nbIntersections : 0.0;
            mApp->aabbVisu.trimByProb[0] = std::min(mApp->aabbVisu.trimByProb[0], (float)item.steps);
            mApp->aabbVisu.trimByProb[1] = std::max(mApp->aabbVisu.trimByProb[1], (float)item.steps);
        }
    }
    else if(mApp->worker.IsRunning()){
        auto& bvh = bvhs.front();
        mApp->aabbVisu.trimByProb[0] = 1e99;
        mApp->aabbVisu.trimByProb[1] = 0;
        for (int n = 0; n < facItems.Size; n++)
        {
            FacetData& item = facItems[n];
            auto& f = facets[item.ID];
            item.steps =  (f->nbTraversalSteps > 0) ? (double)f->nbTraversalSteps / (double)f->nbIntersections : 0.0;
            mApp->aabbVisu.trimByProb[0] = std::min(mApp->aabbVisu.trimByProb[0], (float)item.steps);
            mApp->aabbVisu.trimByProb[1] = std::max(mApp->aabbVisu.trimByProb[1], (float)item.steps);
        }
    }



    if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
    {
        static ImGuiTableFlags tFlags =
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit |
                ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
                ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_Sortable;

        if (ImGui::BeginTabItem("Description"))
        {
            if (ImGui::BeginTable("Boxes", 4, tFlags)) {
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
                                if(selected)
                                    mApp->aabbVisu.selectedNode = -1;
                                else
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
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Facet"))
        {
            if (ImGui::BeginTable("faclist", 2, tFlags)) {
                ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 0.0f, FacetDataColumnID_ID);
                ImGui::TableSetupColumn("Steps", ImGuiTableColumnFlags_WidthStretch, 0.0f, FacetDataColumnID_Steps);
                ImGui::TableHeadersRow();

                // Sort our data if sort specs have been changed!
                if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
                    if (sorts_specs->SpecsDirty){
                        FacetData::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
                        if (facItems.Size > 1)
                            qsort(&facItems[0], (size_t)facItems.Size, sizeof(facItems[0]), FacetData::CompareWithSortSpecs);
                        FacetData::s_current_sort_specs = NULL;
                        sorts_specs->SpecsDirty = false;
                    }

                // Demonstrate using clipper for large vertical lists
                ImGuiListClipper clipper;
                    clipper.Begin(facItems.size());
                    while (clipper.Step()) {
                        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                            FacetData* item = &facItems[i];
                            //ImGui::PushID(item->ID);
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);

                            /*ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                            bool selected = mApp->aabbVisu.selectedNode == item->ID;
                            char label[32];
                            sprintf(label, "%d", item->ID);
                            if (ImGui::Selectable(label, selected, selectable_flags, ImVec2(0, 0)))
                            {
                                if(selected)
                                    mApp->aabbVisu.selectedNode = -1;
                                else
                                    mApp->aabbVisu.selectedNode = item->ID;
                            }*/

                            ImGui::Text("%d", item->ID);
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", item->steps);
                            //ImGui::PopID();
                        }
                    }
                    ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}