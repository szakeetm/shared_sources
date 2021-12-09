//
// Created by pbahr on 8/2/21.
//

#include "Geometry_shared.h"
#include <future>
#include "ImguiAABB.h"
#include "imgui/imgui.h"
#include <imgui/imgui_internal.h>
#include "transfer_function_widget.h"
#include "ImguiExtensions.h"

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
namespace {
    // We are passing our own identifier to TableSetupColumn() to facilitate identifying columns in the sorting code.
    // This identifier will be passed down into ImGuiTableSortSpec::ColumnUserID.
    // But it is possible to omit the user id parameter of TableSetupColumn() and just use the column index instead! (ImGuiTableSortSpec::ColumnIndex)
    // If you don't use sorting, you will generally never care about giving column an ID!
    enum BoxDataColumnID {
        BoxDataColumnID_ID,
        BoxDataColumnID_Chance,
        BoxDataColumnID_ISGlob,
        BoxDataColumnID_Prims,
        BoxDataColumnID_Level,
        BoxDataColumnID_IsLeaf

    };

    struct BoxData {
        int ID;
        double chance;
        int prims;
        int level;
        double globalIntersectionRate;
        bool isLeaf;

        static const ImGuiTableSortSpecs *s_current_sort_specs;

        // Compare function to be used by qsort()
        static int IMGUI_CDECL CompareWithSortSpecs(const void *lhs, const void *rhs) {
            const auto *a = (const BoxData *) lhs;
            const auto *b = (const BoxData *) rhs;
            for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
                // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                const ImGuiTableColumnSortSpecs *sort_spec = &s_current_sort_specs->Specs[n];
                int delta = 0;
                switch (sort_spec->ColumnUserID) {
                    case BoxDataColumnID_ID:
                        delta = (a->ID - b->ID);
                        break;
                    case BoxDataColumnID_Chance:
                        delta = (a->chance > b->chance) ? 1 : (a->chance == b->chance) ? 0 : -1;
                        break;
                    case BoxDataColumnID_ISGlob:
                        delta = (a->globalIntersectionRate > b->globalIntersectionRate) ? 1
                                                                                        : (a->globalIntersectionRate ==
                                                                                           b->globalIntersectionRate)
                                                                                          ? 0 : -1;
                        break;
                    case BoxDataColumnID_Prims:
                        delta = (a->prims - b->prims);
                        break;
                    case BoxDataColumnID_Level:
                        delta = (a->level - b->level);
                        break;
                    case BoxDataColumnID_IsLeaf:
                        delta = (a->isLeaf) ? 1 : 0;
                        break;
                    default:
                        IM_ASSERT(0);
                        break;
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

    const ImGuiTableSortSpecs *BoxData::s_current_sort_specs = nullptr;

    enum FacetDataColumnID {
        FacetDataColumnID_ID,
        FacetDataColumnID_Steps,
        FacetDataColumnID_Inter,
        FacetDataColumnID_InterGlob
    };

    struct FacetData {
        int ID;
        int steps;
        double intersectionRate;
        double intersectionRate_global;

        static const ImGuiTableSortSpecs *s_current_sort_specs;

        // Compare function to be used by qsort()
        static int IMGUI_CDECL CompareWithSortSpecs(const void *lhs, const void *rhs) {
            const auto *a = (const FacetData *) lhs;
            const auto *b = (const FacetData *) rhs;
            for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
                // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                const ImGuiTableColumnSortSpecs *sort_spec = &s_current_sort_specs->Specs[n];
                int delta = 0;
                switch (sort_spec->ColumnUserID) {
                    case FacetDataColumnID_ID:
                        delta = (a->ID - b->ID);
                        break;
                    case FacetDataColumnID_Steps:
                        delta = (a->steps > b->steps) ? 1 : (a->steps == b->steps) ? 0 : -1;
                        break;
                    case FacetDataColumnID_Inter:
                        delta = (a->intersectionRate > b->intersectionRate) ? 1 : (a->intersectionRate ==
                                                                                   b->intersectionRate) ? 0 : -1;
                        break;
                    case FacetDataColumnID_InterGlob:
                        delta = (a->intersectionRate_global > b->intersectionRate_global) ? 1
                                                                                          : (a->intersectionRate_global ==
                                                                                             b->intersectionRate_global)
                                                                                            ? 0 : -1;
                        break;

                    default:
                        IM_ASSERT(0);
                        break;
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

    const ImGuiTableSortSpecs *FacetData::s_current_sort_specs = nullptr;
    
    // Test rays
    enum RayDataColumnID {
        RayDataColumnID_ID,
        RayDataColumnID_NRays,
        RayDataColumnID_MaxRays
    };

    struct RayData {
        int ID;
        int nRays;
        int maxRays;

        static const ImGuiTableSortSpecs *s_current_sort_specs;

        // Compare function to be used by qsort()
        static int IMGUI_CDECL CompareWithSortSpecs(const void *lhs, const void *rhs) {
            const auto *a = (const RayData *) lhs;
            const auto *b = (const RayData *) rhs;
            for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
                // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                const ImGuiTableColumnSortSpecs *sort_spec = &s_current_sort_specs->Specs[n];
                int delta = 0;
                switch (sort_spec->ColumnUserID) {
                    case RayDataColumnID_ID:
                        delta = (a->ID - b->ID);
                        break;
                    case RayDataColumnID_NRays:
                        delta = (a->nRays > b->nRays) ? 1 : (a->nRays == b->nRays) ? 0 : -1;
                        break;
                    case RayDataColumnID_MaxRays:
                        delta = (a->maxRays > b->maxRays) ? 1 : (a->maxRays == b->maxRays) ? 0 : -1;
                        break;
                    default:
                        IM_ASSERT(0);
                        break;
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

    const ImGuiTableSortSpecs *RayData::s_current_sort_specs = nullptr;
}

ImguiAABBVisu::ImguiAABBVisu() {
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

    mApp->aabbVisu.colorMap = tfn_widget.get_colormapf();
}

inline int Log2Int(uint32_t v) {
#if defined(_MSC_VER)
    unsigned long lz = 0;
    if (_BitScanReverse(&lz, v)) return lz;
    return 0;
#else
    return 31 - __builtin_clz(v);
#endif
}

inline int Log2Int(int32_t v) { return Log2Int((uint32_t) v); }

inline int Log2Int(uint64_t v) {
#if defined(_MSC_VER)
    unsigned long lz = 0;
#if defined(_WIN64)
    _BitScanReverse64(&lz, v);
#else
    if  (_BitScanReverse(&lz, v >> 32))
        lz += 32;
    else
        _BitScanReverse(&lz, v & 0xffffffff);
#endif // _WIN64
    return lz;
#else  // _MSC_VER
    return 63 - __builtin_clzll(v);
#endif
}

inline int Log2Int(int64_t v) { return Log2Int((uint64_t) v); }

int UpdateADSStats(){
    // Reset all stats from nodes and facets
    if (!mApp->worker.model->accel.empty() && mApp->worker.model->accel.front() != nullptr && mApp->worker.model->initialized) {
        mApp->worker.model->accel.front()->ResetStats();
    }

    for(auto& fac : mApp->worker.model->facets){
        fac->nbTraversalSteps = 0;
        fac->nbIntersections = 0;
        fac->nbTests = 0;
    }

    auto battery = mApp->worker.globState.PrepareHitBattery();
    mApp->worker.model->ComputeHitStats(battery);

    return 0;
}

void ImguiAABBVisu::ShowAABB(MolFlow *mApp, bool *show_aabb, bool &redrawAabb, bool &rebuildAabb) {
    ImGui::PushStyleVar(
            ImGuiStyleVar_WindowMinSize,
            ImVec2(400.0f, 500.0f)); // Lift normal size constraint, however the presence of
    // a menu-bar will give us the minimum height we want.

    ImGui::Begin(
            "AABB viewer", show_aabb,
            ImGuiWindowFlags_NoSavedSettings); // Pass a pointer to our bool
    ImGui::PopStyleVar(1);

    static std::shared_ptr<std::vector<float>> rate_vec;
    if (!rate_vec) {
        rate_vec = std::make_shared<std::vector<float>>();
        mApp->aabbVisu.rateVector = rate_vec;
    }

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
        if (ImGui::BeginTabItem("Draw Settings")) {
            ImGui::Checkbox("Draw Acceleration Data Structure", &mApp->aabbVisu.renderAABB);
            if (mApp->aabbVisu.renderAABB) {
                    bool showAll = (mApp->aabbVisu.showLevelAABB[0] == -1) ? true : false;
                    if (ImGui::Checkbox("Show all AABB levels", &showAll)) {
                        if (!showAll) {
                            mApp->aabbVisu.showLevelAABB[0] = 0;
                            mApp->aabbVisu.showLevelAABB[1] = std::round(
                                    8.0f + 1.3f * Log2Int(int64_t(mApp->worker.model->facets.size())));
                        } else {
                            mApp->aabbVisu.showLevelAABB[0] = -1;
                            mApp->aabbVisu.showLevelAABB[1] = -1;
                        }
                    }
                    if (!showAll)
                        ImGui::DragIntRange2("Show AABB level", &mApp->aabbVisu.showLevelAABB[0],
                                             &mApp->aabbVisu.showLevelAABB[1], 1.0f, 0, 30);

                    ImGui::SliderFloat("AABB alpha", &mApp->aabbVisu.alpha, 0.0f, 1.0f);
                    ImGui::Checkbox("Show left  branches", &mApp->aabbVisu.showBranchSide[0]);
                    ImGui::Checkbox("Show right branches", &mApp->aabbVisu.showBranchSide[1]);
                    ImGui::Checkbox("Show AABB leaves", &mApp->aabbVisu.showAABBLeaves);
                    ImGui::Checkbox("BBox Expansion", &mApp->aabbVisu.boxExpansion);
                    ImGui::Checkbox("Reverse Expansion", &mApp->aabbVisu.reverseExpansion);
                    ImGui::Checkbox("Apply same color", &mApp->aabbVisu.sameColor);
                    if (ImGui::Checkbox("Render colors based on hit stats", &mApp->aabbVisu.showStats))
                        mApp->aabbVisu.travStep = (mApp->aabbVisu.showStats) ? false : mApp->aabbVisu.travStep;
                    if (ImGui::Checkbox("Use traversal step heatmap", &mApp->aabbVisu.travStep))
                        mApp->aabbVisu.showStats = (mApp->aabbVisu.travStep) ? false : mApp->aabbVisu.showStats;

                    /*if (!rate_vec) {
                        rate_vec = std::make_shared<std::vector<float>>();
                        mApp->aabbVisu.rateVector = rate_vec;
                    }*/
                ImGui::Checkbox("Render only Split borders", &mApp->aabbVisu.onlyBorder);
                    ImGui::Checkbox("Draw all structures", &mApp->aabbVisu.drawAllStructs);

            }
            if (ImGui::Button("Apply aabb view"))
                redrawAabb = true;

            static bool color_win = false;
            if (ImGui::Button("Edit colormap")) {
                color_win = !color_win;
            }

            if (color_win) {
                // Always center this window when appearing
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowSize(ImVec2(0, 400), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0)); // Lift normal size constraint

                if (ImGui::Begin("Choose colormap"),
                        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_ChildWindow) {
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
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("AABB Builder")) {
            ImGui::Checkbox("Find fastest ADS", &mApp->worker.model->otfParams.benchmarkADS);
                ImGui::Checkbox("Use old BVH", &mApp->aabbVisu.oldBVH);
                static int selected_accel = mApp->worker.model->wp.accel_type;
                {
                    // Using the _simplified_ one-liner Combo() api here
                    // See "Combo" section for examples of how to use the more flexible BeginCombo()/EndCombo() api.
                    const char *items[] = {"BVH", "KD-tree"};
                    if (ImGui::Combo("Accel Type", &selected_accel, items, IM_ARRAYSIZE(items))) {
                        mApp->worker.model->wp.accel_type = selected_accel;
                        mApp->aabbVisu.alpha = selected_accel == 0 ? 0.04f : 0.16f;
                    }
                }

                static std::vector<std::string> items;
                if (selected_accel == 0) {
                    ImGui::SliderInt("Max prims/node", &mApp->worker.model->wp.bvhMaxPrimsInNode, 0, 32);
                    items = {"SAH", "HLBVH", "Middle", "EqualCounts", "MolflowSplit", "ProbSplit", "TestSplit",
                             "HybridSplit"};
                } else {
                    items = {"SAH", "ProbSplit", "ProbHybrid", "TestSplit", "HybridSplit", "HybridBinSplit"};
                }

                if (ImGui::BeginListBox("Splitting technique")) {
                    for (int n = 0; n < items.size(); n++) {
                        const bool is_selected = (mApp->aabbVisu.splitTechnique == n);
                        if (ImGui::Selectable(items[n].c_str(), is_selected)) {
                            mApp->aabbVisu.splitTechnique = n;
                            mApp->worker.model->wp.splitMethod = n;
                        }

                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndListBox();
                }

                if (mApp->worker.model->wp.accel_type == 1) {
                    static bool withRopes = mApp->worker.model->wp.kd_with_ropes;
                    static bool restartRope = mApp->worker.model->wp.kd_restart_ropes;
                    static bool optimizedRopes = mApp->worker.model->wp.kd_with_ropes_optimized;

                    if (ImGui::Checkbox("Optimise ropes", &optimizedRopes)) {
                        mApp->worker.model->wp.kd_with_ropes_optimized = optimizedRopes;
                        if (withRopes) {
                            for (int s = 0; s < mApp->worker.model->accel.size(); s++) {
                                auto kd = dynamic_cast<KdTreeAccel *>(mApp->worker.model->accel.at(s).get());
                                if (kd) {
                                    kd->RemoveRopes();
                                    kd->AddRopes(true);
                                }
                            }
                        }
                    }
                    if (ImGui::Checkbox("Use ropes", &withRopes)) {
                        mApp->worker.model->wp.kd_with_ropes = withRopes;
                        if (withRopes) {
                            for (int s = 0; s < mApp->worker.model->accel.size(); s++) {
                                auto kd = dynamic_cast<KdTreeAccel *>(mApp->worker.model->accel.at(s).get());
                                if (kd) {
                                    kd->AddRopes(optimizedRopes);
                                }
                            }
                        } else {
                            for (int s = 0; s < mApp->worker.model->accel.size(); s++) {
                                auto kd = dynamic_cast<KdTreeAccel *>(mApp->worker.model->accel.at(s).get());
                                if (kd) {
                                    kd->RemoveRopes();
                                }
                            }
                        }
                    }
                    if (ImGui::Checkbox("Restart from nodes with by rope", &restartRope)) {
                        mApp->worker.model->wp.kd_restart_ropes = restartRope;
                        if(mApp->worker.IsRunning())
                            mApp->StartStopSimulation();
                        for (auto & s : mApp->worker.model->accel) {
                            auto kd = dynamic_cast<KdTreeAccel *>(s.get());
                            if (kd) {
                                kd->restartFromNode = restartRope;
                            }
                        }
                    }
                    if(ImGui::InputDouble("Hybrid alpha weight", &mApp->worker.model->wp.hybridWeight, 0.01f, 1.0f, "%.2f")){
                        std::clamp(mApp->worker.model->wp.hybridWeight, 0.0, 1.0);
                    }
                }

                if (ImGui::Button("Build new AABB")) {
                    rebuildAabb = true;
                    rate_vec->assign(rate_vec->size(), 0.0);
                }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Test Particle")) {
            {

                bool withHitBattery = false;
                if (mApp->worker.model->wp.accel_type == 0)
                    withHitBattery |= mApp->aabbVisu.splitTechnique == (int) BVHAccel::SplitMethod::TestSplit;
                else if (mApp->worker.model->wp.accel_type == 1) {
                    withHitBattery |= mApp->aabbVisu.splitTechnique == (int) KdTreeAccel::SplitMethod::HybridBin;
                    withHitBattery |= mApp->aabbVisu.splitTechnique == (int) KdTreeAccel::SplitMethod::HybridSplit;
                    withHitBattery |= mApp->aabbVisu.splitTechnique == (int) KdTreeAccel::SplitMethod::TestSplit;
                }

                {//if (withHitBattery) {
                    ImGui::Checkbox("ADS / Split is using hit battery?", &withHitBattery);

                    // Create item list
                    static ImVector<RayData> rayItems;

                    int maxSamples = mApp->worker.globState.hitBattery.maxSamples;
                    if (ImGui::SliderInt("Change samples size", &maxSamples, 0, HITCACHELIMIT)) {
                        mApp->worker.globState.hitBattery.maxSamples = maxSamples;
                    }
                    if (ImGui::Button("Update hit frequencies")) {
                        mApp->worker.globState.UpdateBatteryFrequencies();
                    }

                    static int nbTestRays = 0;
                    bool updateTestRays = false;
                    ImGui::Checkbox("Enable Ray Sampling", &mApp->worker.model->otfParams.raySampling);
                    if (ImGui::Button("Update #TestRays")) {
                        nbTestRays = 0;
                        for (auto &bat: mApp->worker.globState.hitBattery.rays) {
                            //for(auto& hit : bat) {
                            if (bat.empty()) continue;
                            nbTestRays += bat.Size();
                        }
                        updateTestRays = true;
                    }
                    ImGui::Text("Test Rays: %d", nbTestRays);

                    if (updateTestRays || ((int) rayItems.size() != (int) mApp->worker.globState.hitBattery.size())) {
                        rayItems.resize(mApp->worker.globState.hitBattery.size(), RayData());
                        int nodeLevel_max = 0;


                        for (int n = 0; n < rayItems.Size; n++) {
                            RayData &item = rayItems[n];
                            auto &f = mApp->worker.globState.hitBattery.rays[n];
                            item.ID = n;
                            item.nRays = f.Size();
                            item.maxRays = mApp->worker.globState.hitBattery.rays[item.ID].Capacity();
                        }
                    }

                    if (ImGui::CollapsingHeader("Test rays")) {
                        ImGui::Checkbox("Toggle Testray view", &mApp->aabbVisu.renderSampleRays);

                        static ImGuiTableFlags tFlags =
                                ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit |
                                ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
                                ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                                ImGuiTableFlags_Sortable;

                        if (ImGui::BeginTable("Test rays", 3, tFlags)) {
                            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                            ImGui::TableSetupColumn("#Facet", ImGuiTableColumnFlags_WidthFixed, 0.0f,
                                                    RayDataColumnID_ID);
                            ImGui::TableSetupColumn("nRays", ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                                    RayDataColumnID_NRays);
                            ImGui::TableSetupColumn("Max Rays", ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                                    RayDataColumnID_MaxRays);
                            ImGui::TableHeadersRow();

                            // Sort our data if sort specs have been changed!
                            if (ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs())
                                if (sorts_specs->SpecsDirty) {
                                    RayData::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
                                    if (rayItems.Size > 1)
                                        qsort(&rayItems[0], (size_t) rayItems.Size, sizeof(rayItems[0]),
                                              RayData::CompareWithSortSpecs);
                                    RayData::s_current_sort_specs = nullptr;
                                    sorts_specs->SpecsDirty = false;
                                }

                            // Demonstrate using clipper for large vertical lists
                            ImGuiListClipper clipper;
                            clipper.Begin(rayItems.size());
                            while (clipper.Step()) {
                                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                                    RayData *item = &rayItems[i];
                                    //ImGui::PushID(item->ID);
                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex(0);

                                    ImGuiSelectableFlags selectable_flags =
                                            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

                                    auto selectedFac = mApp->worker.GetGeometry()->GetSelectedFacets();
                                    bool selected = false;
                                    auto selection = selectedFac.begin();
                                    for (; selection != selectedFac.end(); selection++) {
                                        if ((int) (*selection) == item->ID) {
                                            selected = true;
                                            ImGui::SetItemDefaultFocus();
                                            break;
                                        }
                                    }
                                    char label[32];
                                    sprintf(label, "%d", item->ID);
                                    if (ImGui::Selectable(label, selected, selectable_flags, ImVec2(0, 0))) {
                                        if (ImGui::GetIO().KeyCtrl) {
                                            if (selected) {
                                                selectedFac.erase(selection);
                                            } else
                                                selectedFac.push_back(item->ID);
                                        } else {
                                            selectedFac.clear();
                                            selectedFac.push_back(item->ID);
                                        }
                                        //mApp->UpdateFacetlistSelected();
                                        mApp->worker.GetGeometry()->SetSelection(selectedFac, false, false);
                                    }

                                    //ImGui::Text("%zu", item->ID);
                                    /*ImGui::TableNextColumn();
                                    ImGui::Text("%d", item->ID);*/
                                    ImGui::TableNextColumn();
                                    ImGui::Text("%d", item->nRays);
                                    ImGui::TableNextColumn();
                                    ImGui::Text("%d", item->maxRays);
                                    //ImGui::PopID();
                                }
                            }
                            ImGui::EndTable();
                        }
                    }
                }

                static RayStat ray{};
                static RayStat ray_saved[3]{RayStat(Vector3d(-3.8835912746821628,-6.0998824020024864,-13),
                                                    Vector3d(-0.60202049004275404,0.2776830749837918,0.74864106181549261))};

                std::vector<TestRay> &sample = mApp->aabbVisu.sample;
                if (ImGui::Button("Create Test Particle")) {
                    if (ray.rng)
                        delete ray.rng;
                    if (ray.pay)
                        delete ray.pay;

                    ray = RayStat{};
                    ray.rng = new MersenneTwister;
                    if(mApp->worker.model->wp.kd_restart_ropes)
                        // with ropes
                        ray.pay = new RopePayload;
                    ray.rng->SetSeed(GenerateSeed(0));
                    mApp->worker.model->StartFromSource(ray);
                    sample.clear();
                    sample.emplace_back(TestRay(ray.origin, ray.direction, ray.lastIntersected));
                }

                bool enabled = !mApp->worker.model->accel.empty();
                static int rayNodePos = 0;
                static bool hit = false;
                static double oldPos[3]{0.0, 0.0, 0.0};
                static double oldDir[3]{0.0, 0.0, 0.0};
                static int oldFacet = -1;
                if (!enabled)
                    ImGui::BeginDisabled();
                if (ImGui::Button("Next step")) {
                    rayNodePos = 0;
                    if (!ray.rng) {
                        ray.rng = new MersenneTwister;
                        if(mApp->worker.model->wp.kd_restart_ropes)
                            ray.pay = new RopePayload;
                        mApp->worker.model->StartFromSource(ray);
                        sample.clear();
                        sample.emplace_back(TestRay(ray.origin, ray.direction, ray.lastIntersected));
                    }
                    oldPos[0] = ray.origin.x;
                    oldPos[1] = ray.origin.y;
                    oldPos[2] = ray.origin.z;
                    oldDir[0] = ray.direction.x;
                    oldDir[1] = ray.direction.y;
                    oldDir[2] = ray.direction.z;
                    oldFacet = ray.lastIntersected;
                    hit = mApp->worker.model->accel[ray.structure]->IntersectStat(ray);
                    if (hit) {
                        ray.origin = ray.origin + ray.hardHit.hit.colDistTranspPass * ray.direction;
                        ray.lastIntersected = ray.hardHit.hitId;
                        mApp->worker.model->PerformBounce(ray, mApp->worker.model->facets[ray.lastIntersected].get());
                        sample.emplace_back(TestRay(ray.origin, ray.direction, ray.lastIntersected));
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Repeat step")) {
                    rayNodePos = 0;
                    if (!ray.pay && mApp->worker.model->wp.kd_restart_ropes)
                        ray.pay = new RopePayload;
                    if (!ray.rng) {
                        ray.rng = new MersenneTwister;
                        mApp->worker.model->StartFromSource(ray);
                        sample.clear();
                        sample.emplace_back(TestRay(ray.origin, ray.direction, ray.lastIntersected));
                    }
                    ray.origin.x = oldPos[0];
                    ray.origin.y = oldPos[1];
                    ray.origin.z = oldPos[2];
                    ray.direction.x = oldDir[0];
                    ray.direction.y = oldDir[1];
                    ray.direction.z = oldDir[2];
                    ray.lastIntersected = oldFacet;

                    hit = mApp->worker.model->accel[ray.structure]->IntersectStat(ray);
                    if (hit) {
                        ray.origin = ray.origin + ray.hardHit.hit.colDistTranspPass * ray.direction;
                        ray.lastIntersected = ray.hardHit.hitId;
                        mApp->worker.model->PerformBounce(ray, mApp->worker.model->facets[ray.lastIntersected].get());
                        if (!sample.empty())
                            sample.back() = TestRay(ray.origin, ray.direction, ray.lastIntersected);
                    }
                }
                if (!enabled)
                    ImGui::EndDisabled();

                if (hit)
                    ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                          IM_COL32(40, 217, 40, 255));
                else
                    ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                          IM_COL32(217, 40, 40, 255));
                ImGui::SameLine();
                ImGui::Checkbox("", &hit);
                ImGui::PopStyleColor();

                if (!enabled)
                    ImGui::BeginDisabled();
                
                for(int i = 0; i < 3; ++i) {
                    auto& save = ray_saved[i];
                    auto str = fmt::format("Save ray {}", i);
                    if (ImGui::Button(str.c_str())) {
                        if (!save.pay && mApp->worker.model->wp.kd_restart_ropes) {
                            save.pay = new RopePayload;
                        }
                        if (!save.rng) {
                            save.rng = new MersenneTwister;
                        }
                        save = ray;
                        if (ray.rng)
                            *save.rng = *ray.rng;
                        if (ray.pay)
                            *(RopePayload *) save.pay = *(RopePayload *) ray.pay;
                    }
                    if(i != 3-1){
                        ImGui::SameLine();
                    }
                }

                for(int i = 0; i < 3; ++i) {
                    auto& save = ray_saved[i];
                    auto str = fmt::format("Load ray {}", i);
                    if (ImGui::Button(str.c_str())) {
                        rayNodePos = 0;
                        ray = save;
                        if(save.rng)
                            *ray.rng = *save.rng;
                        else
                            ray.rng = new MersenneTwister;
                        if (ray.pay && save.pay)
                            *(RopePayload *) ray.pay = *(RopePayload *) save.pay;

                        // use new ray for visualisation
                        sample.clear();
                        sample.emplace_back(TestRay(ray.origin, ray.direction, ray.lastIntersected));
                    }

                    // Tooltip for row4
                    if(ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::BeginDisabled();
                        char origin_label[32];
                        float origin[3]{static_cast<float>(save.origin.x), static_cast<float>(save.origin.y),
                                        static_cast<float>(save.origin.z)};
                        sprintf(origin_label, "Origin on %d", save.lastIntersected);
                        ImGui::InputFloat3(origin_label, reinterpret_cast<float *>(&origin));
                        float direction[3]{static_cast<float>(save.direction.x), static_cast<float>(save.direction.y),
                                           static_cast<float>(save.direction.z)};
                        ImGui::InputFloat3("Direction", reinterpret_cast<float *>(&direction));
                        if(save.pay && ((RopePayload*)save.pay)->lastNode) {
                            ImGui::Text("Start node on %d", ((RopePayload *) save.pay)->lastNode->nodeId);
                        }
                        ImGui::EndDisabled();
                        ImGui::EndTooltip();
                    }

                    if(i != 3-1){
                        ImGui::SameLine();
                    }
                }

                if (!enabled)
                    ImGui::EndDisabled();

                ImGui::BeginDisabled();
                char origin_label[32];
                sprintf(origin_label, "Origin on %d", oldFacet);
                float oldOrigin[3]{static_cast<float>(oldPos[0]), static_cast<float>(oldPos[1]),
                                   static_cast<float>(oldPos[2])};
                ImGui::InputFloat3(origin_label, reinterpret_cast<float *>(&oldOrigin));
                float origin[3]{static_cast<float>(ray.origin.x), static_cast<float>(ray.origin.y),
                                static_cast<float>(ray.origin.z)};
                sprintf(origin_label, "New Pos on %d", ray.lastIntersected);
                ImGui::InputFloat3(origin_label, reinterpret_cast<float *>(&origin));
                float direction[3]{static_cast<float>(ray.direction.x), static_cast<float>(ray.direction.y),
                                   static_cast<float>(ray.direction.z)};
                ImGui::InputFloat3("Direction", reinterpret_cast<float *>(&direction));
                ImGui::EndDisabled();

                static float new_values[3]{0.0,0.0,0.0};
                ImGui::InputFloat3("Set new values", new_values);
                if(ImGui::Button("Set as new origin")){
                    ray.origin.x = new_values[0];
                    ray.origin.y = new_values[1];
                    ray.origin.z = new_values[2];
                }
                ImGui::SameLine();
                if(ImGui::Button("Set as new direction")){
                    ray.direction.x = new_values[0];
                    ray.direction.y = new_values[1];
                    ray.direction.z = new_values[2];
                }
                static int new_origin = -1;
                ImGui::InputInt("Set new values", &new_origin);
                if(ImGui::Button("Set as new source ID")){
                    ray.lastIntersected = new_origin;
                    oldFacet = new_origin;
                }
                ImGui::SameLine();
                ImGui::Text("%d",ray.lastIntersected);
                if(ImGui::Button("Set as new starting node")){
                    if(!ray.pay && mApp->worker.model->wp.kd_restart_ropes)
                        new RopePayload;
                    if(dynamic_cast<KdTreeAccel*>(mApp->worker.model->accel.at(ray.structure).get())){
                        if(ray.pay)((RopePayload*)ray.pay)->lastNode = &dynamic_cast<KdTreeAccel*>(mApp->worker.model->accel.at(ray.structure).get())->nodes[new_origin];
                    }
                }
                if(ray.pay && ((RopePayload*)ray.pay)->lastNode) {
                    ImGui::SameLine();
                    ImGui::Text("%d", ((RopePayload *) ray.pay)->lastNode->nodeId);
                }

                if (!enabled)
                    ImGui::BeginDisabled();
                if (ImGui::Button("Visualise next node")) {
                    if ((int)ray.traversedNodes.size() > rayNodePos) {
                        mApp->aabbVisu.selectedNode = (int)ray.traversedNodes[rayNodePos++];
                        redrawAabb = true;
                    } else if (rayNodePos == 0 && !ray.traversedNodes.empty()) {
                        mApp->aabbVisu.selectedNode = (int)ray.traversedNodes[rayNodePos++];
                        redrawAabb = true;
                    } else {
                        rayNodePos = 0;
                    }
                }
                ImGui::SameLine();
                ImGui::Text("[%d/%zu] %d", rayNodePos, ray.traversedNodes.size(), mApp->aabbVisu.selectedNode);
                if (!ray.traversedNodes.empty()) {
                    if (ImGui::SliderInt("Node traversal chain", &rayNodePos, 0, ray.traversedNodes.size() - 1)) {
                        mApp->aabbVisu.selectedNode = ray.traversedNodes[rayNodePos];
                        redrawAabb = true;
                    }
                }
                if (!enabled)
                    ImGui::EndDisabled();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("ADS Stats")) {
            static bool constantUpdates = false;
            bool forceUpdate = false;
            ImGui::Checkbox("Update constantly", &constantUpdates);
            ImGui::SameLine();
            if (ImGui::Button("Force update"))
                forceUpdate = true;

            std::promise<int> p;
            static std::future<int> future_int;
            if (!future_int.valid())
                future_int = p.get_future();

            static float trimMax = 0.0f;
            static float trimMin = 1.0e30f;
            if (ImGui::SliderFloat2("Hit stat threshold", (float *) &mApp->aabbVisu.trimByProb, 0.0f,
                                    trimMax > 0.0f ? trimMax : 1.0f,
                                    "%.2f")) {
                mApp->aabbVisu.trimRange = mApp->aabbVisu.trimByProb[1] - mApp->aabbVisu.trimByProb[0];
            }

            if (ImGui::Button("Update ADS stats")) {
                // TODO: Currently only functions when test battery is created explicitly
                future_int = std::async(std::launch::async, &UpdateADSStats);
            }

            static bool active_prev_state = false;
            // Evaluate running progress
            if (future_int.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
                static float progress = 0.0f, load_time = 0.0f;
                ImGui::Loader(progress, load_time);
                active_prev_state = true;
                mApp->wereEvents_imgui = true;
            } else if (active_prev_state) {
                active_prev_state = false;
                // Mark for immediate update
                forceUpdate = true;
            }

            // Create item lists for Node and Facet tabs
            static ImVector<BoxData> tabItems;
            static ImVector<FacetData> facItems;
            if (rebuildAabb || forceUpdate) {
                tabItems.clear();
                facItems.clear();
            }
            auto &accel = mApp->worker.model->accel;

            RTStats tree_stats{};
            if(mApp->worker.model->wp.accel_type == 1)
                for( auto& acc : accel){
                    //auto nodes = static_cast<KdTreeAccel*>(acc.get())->nodes;
                    for(const auto& stat : acc->ints) {
                        tree_stats.nIntersections += stat.nbIntersects;
                        tree_stats.nTraversedInner += stat.nbChecks;
                    }
                }
            // old_bvb
            int nInt = mApp->worker.globState.globalHits.globalHits.nbHitEquiv > 0.0 ? static_cast<int>(
                    static_cast<double>(tree_stats.nIntersections) /
                    mApp->worker.globState.globalHits.globalHits.nbHitEquiv) : 0;
            if(ImGui::InputInt("nIntersects", &nInt)) {
                tree_stats.nIntersections = static_cast<size_t>(nInt);
            }
            int nCheck = mApp->worker.globState.globalHits.globalHits.nbHitEquiv > 0.0 ? static_cast<int>(
                    tree_stats.nTraversedInner / mApp->worker.globState.globalHits.globalHits.nbHitEquiv) : 0;
            if(ImGui::InputInt("nCheck", &nCheck)) {
                tree_stats.nTraversedInner = nCheck;
            }
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
                static ImGuiTableFlags tFlags =
                        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit |
                        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                        ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
                        ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                        ImGuiTableFlags_Sortable;

                if (ImGui::BeginTabItem("Nodes")) {

                    static int selected_combo = 0;
                    {
                        // Using the _simplified_ one-liner Combo() api here
                        // See "Combo" section for examples of how to use the more flexible BeginCombo()/EndCombo() api.
                        const char *items[] = {"Chance", "ISRate global"};
                        if (ImGui::Combo("combo", &selected_combo, items, IM_ARRAYSIZE(items)))
                            forceUpdate = true;
                    }

                    std::vector<IntersectCount> *stats = nullptr;
                    if (!accel.empty() && mApp->worker.model->initialized) {
                        if (accel.front() != nullptr) {
                            stats = &accel.front()->ints;
                        }
                    }

                    if (stats && !stats->empty()) {
                        if (rate_vec && rate_vec->size() != stats->size())
                            rate_vec->resize(stats->size(), -1.0f);

                        if (/*tabItems.empty() && */!accel.empty() && (int) tabItems.size() != (int) stats->size()) {
                            auto &bvh = accel.front();

                            size_t nbIntersections_total = 0;
                            tabItems.resize(stats->size(), BoxData());
                            int nodeLevel_max = 0;
                            for (int n = 0; n < tabItems.Size; n++) {
                                BoxData &item = tabItems[n];
                                auto &f = (*stats)[n];
                                item.ID = n;
                                item.chance = (f.nbChecks > 0) ? (double) f.nbIntersects / (double) f.nbChecks : 0.0;
                                item.prims = f.nbPrim;
                                item.level = f.level;
                                if (mApp->worker.model->wp.accel_type &&
                                    std::dynamic_pointer_cast<KdTreeAccel>(accel.front()) != nullptr) {
                                    item.isLeaf = std::dynamic_pointer_cast<KdTreeAccel>(
                                            accel.front())->nodes[n].IsLeaf();
                                } else if (std::dynamic_pointer_cast<BVHAccel>(accel.front()) != nullptr) {
                                    item.isLeaf = std::dynamic_pointer_cast<BVHAccel>(
                                            accel.front())->GetNodes()[n].nPrimitives > 0;
                                } else {
                                    item.isLeaf = false;
                                }
                                nbIntersections_total += f.nbIntersects;

                                nodeLevel_max = std::max(nodeLevel_max, item.level);
                            }
                            mApp->aabbVisu.showLevelAABB[0] = 0;
                            mApp->aabbVisu.showLevelAABB[1] = nodeLevel_max;

                            for (int n = 0; n < tabItems.Size; n++) {
                                BoxData &item = tabItems[n];
                                auto &f = (*stats)[item.ID];
                                item.globalIntersectionRate = (nbIntersections_total > 0) ? (double) f.nbIntersects /
                                                                                            (double) nbIntersections_total
                                                                                          : 0.0;
                            }

                            if (!mApp->aabbVisu.travStep && mApp->aabbVisu.showStats) {
                                for (int n = 0; n < tabItems.Size; n++) {
                                    BoxData &item = tabItems[n];
                                    if (selected_combo == 0)
                                        (*rate_vec)[item.ID] = item.chance;
                                    else if (selected_combo == 1)
                                        (*rate_vec)[item.ID] = item.globalIntersectionRate;
                                }
                            }
                        } else if (forceUpdate || (constantUpdates && mApp->worker.IsRunning())) {
                            size_t nbIntersections_total = 0;

                            auto &bvh = accel.front();
                            for (int n = 0; n < tabItems.Size; n++) {
                                BoxData &item = tabItems[n];
                                auto &f = (*stats)[item.ID];
                                item.chance = (f.nbChecks > 0) ? (double) f.nbIntersects / (double) f.nbChecks : 0.0;
                                nbIntersections_total += f.nbIntersects;
                            }


                            for (int n = 0; n < tabItems.Size; n++) {
                                BoxData &item = tabItems[n];
                                auto &f = (*stats)[item.ID];
                                item.globalIntersectionRate = (nbIntersections_total > 0) ? (double) f.nbIntersects /
                                                                                            (double) nbIntersections_total
                                                                                          : 0.0;
                            }

                            if (!mApp->aabbVisu.travStep && mApp->aabbVisu.showStats) {
                                for (int n = 0; n < tabItems.Size; n++) {
                                    BoxData &item = tabItems[n];
                                    if (selected_combo == 0)
                                        (*rate_vec)[item.ID] = item.chance;
                                    else if (selected_combo == 1)
                                        (*rate_vec)[item.ID] = item.globalIntersectionRate;
                                }
                            }
                        }

                        static bool jump_to_pos = false;
                        static int jump_pos = -1;
                        if(ImGui::InputInt("Jump to position",&jump_pos))
                            jump_to_pos = true;
                        if(mApp->aabbVisu.selectedNode != -1) {
                            if (ImGui::Button("Jump to selected")) {
                                jump_pos = mApp->aabbVisu.selectedNode;
                                jump_to_pos = true;
                            }
                            ImGui::SameLine();
                            ImGui::Text("Node #%d",mApp->aabbVisu.selectedNode);
                        }
                        if (ImGui::BeginTable("Boxes", 6, tFlags)) {
                            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 0.0f, BoxDataColumnID_ID);
                            ImGui::TableSetupColumn("Chance", ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                                    BoxDataColumnID_Chance);
                            ImGui::TableSetupColumn("ISGlob", ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                                    BoxDataColumnID_ISGlob);
                            ImGui::TableSetupColumn("#Prims", ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                                    BoxDataColumnID_Prims);
                            ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                                    BoxDataColumnID_Level);
                            ImGui::TableSetupColumn("Leaf", ImGuiTableColumnFlags_WidthFixed, 0.0f,
                                                    BoxDataColumnID_IsLeaf);
                            ImGui::TableHeadersRow();

                            // Sort our data if sort specs have been changed!
                            if (ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs())
                                if (sorts_specs->SpecsDirty) {
                                    BoxData::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
                                    if (tabItems.Size > 1)
                                        qsort(&tabItems[0], (size_t) tabItems.Size, sizeof(tabItems[0]),
                                              BoxData::CompareWithSortSpecs);
                                    BoxData::s_current_sort_specs = nullptr;
                                    sorts_specs->SpecsDirty = false;
                                }

                            if(jump_to_pos) {
                                for (int i = 0; i < tabItems.Size; i++) {
                                    BoxData *item = &tabItems[i];

                                    if(jump_pos == item->ID){
                                        ImGui::SetScrollHereY();
                                        jump_to_pos = false;
                                    }

                                    //ImGui::PushID(item->ID);
                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex(0);

                                    ImGuiSelectableFlags selectable_flags =
                                            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                                    bool selected = mApp->aabbVisu.selectedNode == item->ID;
                                    char label[32];
                                    sprintf(label, "%d", item->ID);
                                    if (ImGui::Selectable(label, selected, selectable_flags, ImVec2(0, 0))) {
                                        if (selected)
                                            mApp->aabbVisu.selectedNode = -1;
                                        else
                                            mApp->aabbVisu.selectedNode = item->ID;
                                        redrawAabb = true;
                                    }

                                    // Tooltip for row4
                                    if(ImGui::IsItemActive() || ImGui::IsItemHovered()) {
                                        if(item->isLeaf) {
                                            std::vector<size_t> primIDs;
                                            auto tree = std::dynamic_pointer_cast<KdTreeAccel>(
                                                    accel.front()).get();
                                            auto& node = tree->nodes[item->ID];
                                            int nPrimitives = tree->nodes[item->ID].nPrimitives();
                                            if (nPrimitives == 1) {
                                                primIDs.push_back(node.onePrimitive);
                                            } else {
                                                for (int i = 0; i < nPrimitives; ++i) {
                                                    int index =
                                                            tree->primitiveIndices[node.primitiveIndicesOffset + i];
                                                    primIDs.push_back(index);
                                                }
                                            }
                                            if(ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
                                                mApp->worker.GetGeometry()->SetSelection(primIDs, false, false);
                                            }
                                            ImGui::BeginTooltip();
                                            for(auto id : primIDs)
                                                ImGui::Text("%lu", id);
                                            ImGui::EndTooltip();
                                        }
                                    }

                                    //ImGui::Text("%zu", item->ID);
                                    ImGui::TableNextColumn();
                                    ImGui::Text("%f", item->chance);
                                    ImGui::TableNextColumn();
                                    ImGui::Text("%f", item->globalIntersectionRate);
                                    ImGui::TableNextColumn();
                                    ImGui::Text("%d", item->prims);
                                    ImGui::TableNextColumn();
                                    ImGui::Text("%d", item->level);
                                    ImGui::TableNextColumn();
                                    ImGui::BeginDisabled();
                                    ImGui::Checkbox("", &item->isLeaf);
                                    ImGui::EndDisabled();
                                    //ImGui::PopID();
                                }
                            }
                            else {
                                ImGuiListClipper clipper;
                                clipper.Begin(tabItems.size());
                                while (clipper.Step()) {
                                    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                                        BoxData *item = &tabItems[i];
                                        //ImGui::PushID(item->ID);
                                        ImGui::TableNextRow();
                                        ImGui::TableSetColumnIndex(0);

                                        ImGuiSelectableFlags selectable_flags =
                                                ImGuiSelectableFlags_SpanAllColumns |
                                                ImGuiSelectableFlags_AllowItemOverlap;
                                        bool selected = mApp->aabbVisu.selectedNode == item->ID;
                                        char label[32];
                                        sprintf(label, "%d", item->ID);
                                        if (ImGui::Selectable(label, selected, selectable_flags, ImVec2(0, 0))) {
                                            if (selected)
                                                mApp->aabbVisu.selectedNode = -1;
                                            else
                                                mApp->aabbVisu.selectedNode = item->ID;
                                            redrawAabb = true;
                                        }

                                        // Tooltip for row4
                                        if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
                                            if (item->isLeaf) {
                                                std::vector<size_t> primIDs;
                                                auto tree = std::dynamic_pointer_cast<KdTreeAccel>(
                                                        accel.front()).get();
                                                auto &node = tree->nodes[item->ID];
                                                int nPrimitives = tree->nodes[item->ID].nPrimitives();
                                                if (nPrimitives == 1) {
                                                    primIDs.push_back(node.onePrimitive);
                                                } else {
                                                    for (int i = 0; i < nPrimitives; ++i) {
                                                        int index =
                                                                tree->primitiveIndices[node.primitiveIndicesOffset + i];
                                                        primIDs.push_back(index);
                                                    }
                                                }
                                                if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
                                                    mApp->worker.GetGeometry()->SetSelection(primIDs, false, false);
                                                }
                                                ImGui::BeginTooltip();
                                                for (auto id: primIDs)
                                                    ImGui::Text("%lu", id);
                                                ImGui::EndTooltip();
                                            }
                                        }

                                        //ImGui::Text("%zu", item->ID);
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%f", item->chance);
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%f", item->globalIntersectionRate);
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", item->prims);
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", item->level);
                                        ImGui::TableNextColumn();
                                        ImGui::BeginDisabled();
                                        ImGui::Checkbox("", &item->isLeaf);
                                        ImGui::EndDisabled();
                                        //ImGui::PopID();
                                    }
                                }
                            }
                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndTabItem();

                }

                if (ImGui::BeginTabItem("Facet")) {
                    static int selected_combo = 0;
                    {
                        // Using the _simplified_ one-liner Combo() api here
                        // See "Combo" section for examples of how to use the more flexible BeginCombo()/EndCombo() api.
                        const char *items[] = {"Steps", "ISRate", "ISRate global"};
                        if (ImGui::Combo("combo", &selected_combo, items, IM_ARRAYSIZE(items)))
                            forceUpdate = true;
                    }

                    // Create item list
                    auto &facets = mApp->worker.model->facets;
                    static size_t nbSteps_total = 0;
                    static uint64_t nbSteps_totalsum = 0;
                    if (!rate_vec && rate_vec->size() != facets.size())
                        rate_vec->resize(facets.size(), -1.0f);
                    if (!facets.empty() && facItems.size() != facets.size()) {
                        auto &fac = facets;
                        facItems.resize(facets.size(), FacetData());

                        mApp->aabbVisu.trimByProb[0] = 1.0e38f;
                        mApp->aabbVisu.trimByProb[1] = 0;
                        size_t nbIntersections_total = 0;
                        nbSteps_total = 0;
                        nbSteps_totalsum = 0;

                        for (int n = 0; n < facItems.Size; n++) {
                            auto &f = facets[n];
                            FacetData &item = facItems[n];
                            item.ID = n;
                            item.steps = (f->nbTests > 0) ? (double) f->nbTraversalSteps / (double) f->nbTests : 0.0;
                            item.intersectionRate = (f->nbTests > 0) ? (double) f->nbIntersections / (double) f->nbTests
                                                                     : 0.0;
                            nbIntersections_total += f->nbIntersections;
                            nbSteps_total += item.steps;
                            nbSteps_totalsum += item.steps * mApp->worker.globState.facetStates[item.ID].momentResults[0].hits.nbMCHit;
                            mApp->aabbVisu.trimByProb[0] = std::min(mApp->aabbVisu.trimByProb[0], (float) item.steps);
                            mApp->aabbVisu.trimByProb[1] = std::max(mApp->aabbVisu.trimByProb[1], (float) item.steps);
                            if (selected_combo == 0) {
                                trimMax = std::max(trimMax, (float) item.steps);
                                trimMin = std::min(trimMin, (float) item.steps);
                            } else if (selected_combo == 1) {
                                trimMax = std::max(trimMax, (float) item.intersectionRate);
                                trimMin = std::min(trimMin, (float) item.intersectionRate);
                            }
                        }

                        if(mApp->worker.globState.globalHits.globalHits.nbMCHit > 0)
                            nbSteps_totalsum /= mApp->worker.globState.globalHits.globalHits.nbMCHit;

                        for (int n = 0; n < facItems.Size; n++) {
                            FacetData &item = facItems[n];
                            auto &f = facets[item.ID];
                            item.intersectionRate_global = (f->nbTests > 0) ? (double) f->nbTests /
                                                                              (double) nbIntersections_total : 0.0;
                            if (selected_combo == 2) {
                                trimMax = std::max(trimMax, (float) item.intersectionRate_global);
                                trimMin = std::min(trimMin, (float) item.intersectionRate_global);
                            }
                        }

                        if (mApp->aabbVisu.travStep) {
                            switch (selected_combo) {
                                case 1:
                                    for (int n = 0; n < facItems.Size; n++) {
                                        FacetData &item = facItems[n];
                                        (*rate_vec)[item.ID] = item.intersectionRate;
                                    }
                                    break;
                                case 2:
                                    for (int n = 0; n < facItems.Size; n++) {
                                        FacetData &item = facItems[n];
                                        (*rate_vec)[item.ID] = static_cast<float>(item.intersectionRate_global);
                                    }
                                    break;
                                case 0:
                                default:
                                    for (int n = 0; n < facItems.Size; n++) {
                                        FacetData &item = facItems[n];
                                        (*rate_vec)[item.ID] = static_cast<float>(item.steps);
                                    }
                                    break;
                            }

                        }
                    } else if (forceUpdate || (constantUpdates && mApp->worker.IsRunning())) {
                        auto &bvh = accel.front();
                        //mApp->aabbVisu.trimByProb[0] = 1.0e38f;
                        //mApp->aabbVisu.trimByProb[1] = 0;

                        size_t nbIntersections_total = 0;
                        nbSteps_total = 0;
                        nbSteps_totalsum = 0;
                        for (int n = 0; n < facItems.Size; n++) {
                            FacetData &item = facItems[n];
                            auto &f = facets[item.ID];
                            item.steps = (f->nbTests > 0) ? (double) f->nbTraversalSteps / (double) f->nbTests : 0.0;
                            item.intersectionRate = (f->nbTests > 0) ? (double) f->nbIntersections / (double) f->nbTests
                                                                     : 0.0;
                            nbIntersections_total += f->nbIntersections;
                            nbSteps_total += item.steps;
                            nbSteps_totalsum += item.steps * mApp->worker.globState.facetStates[item.ID].momentResults[0].hits.nbMCHit;


                            //mApp->aabbVisu.trimByProb[0] = std::min(mApp->aabbVisu.trimByProb[0], (float)item.steps);
                            //mApp->aabbVisu.trimByProb[1] = std::max(mApp->aabbVisu.trimByProb[1], (float)item.steps);
                            if (selected_combo == 0) {
                                trimMax = std::max(trimMax, (float) item.steps);
                                trimMin = std::min(trimMin, (float) item.steps);
                            } else if (selected_combo == 1) {
                                trimMax = std::max(trimMax, (float) item.intersectionRate);
                                trimMin = std::min(trimMin, (float) item.intersectionRate);
                            }
                        }

                        if(mApp->worker.globState.globalHits.globalHits.nbMCHit > 0)
                            nbSteps_totalsum /= mApp->worker.globState.globalHits.globalHits.nbMCHit;

                        for (int n = 0; n < facItems.Size; n++) {
                            FacetData &item = facItems[n];
                            auto &f = facets[item.ID];
                            item.intersectionRate_global = (f->nbTests > 0) ? (double) f->nbTests /
                                                                              (double) nbIntersections_total : 0.0;
                            if (selected_combo == 2) {
                                trimMax = std::max(trimMax, (float) item.intersectionRate_global);
                                trimMin = std::min(trimMin, (float) item.intersectionRate_global);
                            }
                        }

                        if (mApp->aabbVisu.travStep) {
                            switch (selected_combo) {
                                case 1:
                                    for (int n = 0; n < facItems.Size; n++) {
                                        FacetData &item = facItems[n];
                                        (*rate_vec)[item.ID] = item.intersectionRate;
                                    }
                                    break;
                                case 2:
                                    for (int n = 0; n < facItems.Size; n++) {
                                        FacetData &item = facItems[n];
                                        (*rate_vec)[item.ID] = item.intersectionRate_global;
                                    }
                                    break;
                                case 0:
                                default:
                                    for (int n = 0; n < facItems.Size; n++) {
                                        FacetData &item = facItems[n];
                                        (*rate_vec)[item.ID] = item.steps;
                                    }
                                    break;
                            }
                        }
                    }

                    size_t avgRayIntersects = 0;
                    float avgRayUps = 0;
                    size_t avgRayDowns = 0;
                    size_t avgRayBoxIntersects = 0;
                    static double avgBoxTime = 0.0;
                    static double avgBoxPTime = 0.0;
                    static double avgBVH = 0.0;
                    static double avgKDTrav = 0.0;
                    static double avgKDR = 0.0;
                    static double avgKDRR = 0.0;

                    if(!accel.empty() && accel.front() && mApp->worker.globState.globalHits.globalHits.nbMCHit > 0) {
                        const size_t& denum = mApp->worker.globState.globalHits.globalHits.nbMCHit;
                        avgRayIntersects = accel.front()->perRayCount.nbIntersectionTests / denum;
                        avgRayUps = (float)accel.front()->perRayCount.nbUpTest / denum;
                        avgRayDowns = accel.front()->perRayCount.nbDownTest / denum;
                        avgRayBoxIntersects = accel.front()->perRayCount.nbBoxIntersectionTests / denum;

                        if(mApp->worker.model->wp.accel_type == 1) {
                            //using Profiling::boxPStats;
                            using Profiling::intersectStatsKD;
                            using Profiling::intersectStatsKDRope;
                            using Profiling::intersectStatsKDRopeRestart;

                            //extern CummulativeBenchmark boxPStats;
                            //avgBoxPTime = boxPStats.GetRatio();
                            avgKDTrav = intersectStatsKD.GetRatio();
                            avgKDR = intersectStatsKDRope.GetRatio();
                            avgKDRR = intersectStatsKDRopeRestart.GetRatio();
                        }
                        else {
                            //using Profiling::boxStats;
                            using Profiling::intersectStatsBVH;

                            //extern CummulativeBenchmark boxStats;
                            //avgBoxTime = boxStats.GetRatio();
                            avgBVH = intersectStatsBVH.GetRatio();
                        }
                    }

                    // avg steps per facet, weighted avg steps per facet,
                    ImGui::TextDisabled("%s", fmt::format("AVG (Steps:{} WSteps:{})",
                                                          nbSteps_total/facets.size(), nbSteps_totalsum).c_str());
                    ImGui::TextDisabled("%s", fmt::format("AVG Per Ray (Int:{} Box:{} Up:{:.2f} Down:{})",
                                                          avgRayIntersects, avgRayBoxIntersects, avgRayUps, avgRayDowns).c_str());
                    ImGui::TextDisabled("%s", fmt::format("Timing (BVH {:.2e}ms)",
                                                          avgBVH).c_str());
                    ImGui::TextDisabled("%s", fmt::format("Timing (KD {:.2e}ms {:.2e}ms {:.2e}ms)",
                                                          avgKDTrav, avgKDR, avgKDRR).c_str());
                    ImGui::TextDisabled("%s", fmt::format("Timing (Box {:.2e}ms {:.2e}ms)",
                                                          avgBoxTime, avgBoxPTime).c_str());
                    if (ImGui::BeginTable("facetList", 4, tFlags)) {
                        ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 0.0f, FacetDataColumnID_ID);

                        ImGui::TableSetupColumn("Steps",
                                                ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                                FacetDataColumnID_Steps);
                        ImGui::TableSetupColumn("ISRate", ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                                FacetDataColumnID_Inter);
                        ImGui::TableSetupColumn("ISRateG", ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                                FacetDataColumnID_InterGlob);
                        ImGui::TableHeadersRow();

                        // Sort our data if sort specs have been changed!
                        if (ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs())
                            if (sorts_specs->SpecsDirty) {
                                FacetData::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
                                if (facItems.Size > 1)
                                    qsort(&facItems[0], (size_t) facItems.Size, sizeof(facItems[0]),
                                          FacetData::CompareWithSortSpecs);
                                FacetData::s_current_sort_specs = nullptr;
                                sorts_specs->SpecsDirty = false;
                            }

                        // Demonstrate using clipper for large vertical lists
                        ImGuiSelectableFlags selectable_flags =
                                ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                        auto selectedFac = mApp->worker.GetGeometry()->GetSelectedFacets();

                        ImGuiListClipper clipper;
                        clipper.Begin(facItems.size());
                        while (clipper.Step()) {
                            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                                FacetData *item = &facItems[i];
                                //ImGui::PushID(item->ID);
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);

                                bool selected = false;
                                auto selection = selectedFac.begin();
                                for (; selection != selectedFac.end(); selection++) {
                                    if ((int) (*selection) == item->ID) {
                                        selected = true;
                                        ImGui::SetItemDefaultFocus();
                                        break;
                                    }
                                }
                                char label[32];
                                sprintf(label, "%d", item->ID);
                                if (ImGui::Selectable(label, selected, selectable_flags, ImVec2(0, 0))) {
                                    if (ImGui::GetIO().KeyCtrl) {
                                        if (selected) {
                                            selectedFac.erase(selection);
                                        } else
                                            selectedFac.push_back(item->ID);
                                    } else {
                                        selectedFac.clear();
                                        selectedFac.push_back(item->ID);
                                    }
                                    //mApp->UpdateFacetlistSelected();
                                    mApp->worker.GetGeometry()->SetSelection(selectedFac, false, false);
                                }

                                //ImGui::Text("%d", item->ID);
                                ImGui::TableNextColumn();
                                ImGui::Text("%d", item->steps);
                                ImGui::TableNextColumn();
                                ImGui::Text("%lf", item->intersectionRate);
                                ImGui::TableNextColumn();
                                ImGui::Text("%lf", item->intersectionRate_global);
                                //ImGui::PopID();
                            }
                        }
                        ImGui::EndTable();
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            if (forceUpdate) {
                mApp->aabbVisu.trimByProb[1] = (trimMax > 0.0) ? trimMax : mApp->aabbVisu.trimByProb[1];
                mApp->aabbVisu.trimByProb[0] = (trimMin < 1.0e25f) ? trimMin : mApp->aabbVisu.trimByProb[0];
                redrawAabb = true;
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}