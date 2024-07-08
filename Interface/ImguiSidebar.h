#pragma once

#include "ImguiWindowBase.h"
class MolFlow;

// Dummy data structure that we use for the Table demo.
// (pre-C++11 doesn't allow us to instantiate ImVector<MyItem> template if this structure if defined inside the demo function)
namespace {
    // We are passing our own identifier to TableSetupColumn() to facilitate identifying columns in the sorting code.
    // This identifier will be passed down into ImGuiTableSortSpec::ColumnUserID.
    // But it is possible to omit the user id parameter of TableSetupColumn() and just use the column index instead! (ImGuiTableSortSpec::ColumnIndex)
    // If you don't use sorting, you will generally never care about giving column an ID!
    enum FacetDataColumnID {
        FacetDataColumnID_ID,
        FacetDataColumnID_Hits,
        FacetDataColumnID_Des,
        FacetDataColumnID_Abs
    };

    struct FacetData {
        int ID;
        size_t hits;
        size_t des;
        double abs;
        bool selected;

        // We have a problem which is affecting _only this demo_ and should not affect your code:
        // As we don't rely on std:: or other third-party library to compile dear imgui, we only have reliable access to qsort(),
        // however qsort doesn't allow passing user data to comparing function.
        // As a workaround, we are storing the sort specs in a static/global for the comparing function to access.
        // In your own use case you would probably pass the sort specs to your sorting/comparing functions directly and not use a global.
        // We could technically call ImGui::TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is called
        // very often by the sorting algorithm it would be a little wasteful.
        static const ImGuiTableSortSpecs* s_current_sort_specs;

        // Compare function to be used by qsort()
        static int IMGUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs) {
            const FacetData* a = (const FacetData*)lhs;
            const FacetData* b = (const FacetData*)rhs;
            for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
                // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
                int delta = 0;
                switch (sort_spec->ColumnUserID) {
                case FacetDataColumnID_ID:
                    delta = (a->ID - b->ID);
                    break;
                case FacetDataColumnID_Hits:
                    delta = (a->hits > b->hits) ? 1 : (a->hits == b->hits) ? 0 : -1;
                    break;
                case FacetDataColumnID_Des:
                    delta = (a->des > b->des) ? 1 : (a->des == b->des) ? 0 : -1;
                    break;
                case FacetDataColumnID_Abs:
                    delta = (a->abs > b->abs) ? 1 : (a->abs == b->abs) ? 0 : -1;
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

    const ImGuiTableSortSpecs* FacetData::s_current_sort_specs = nullptr;
};

class ImSidebar : public ImWindow
{
public:
	void Draw();
	void Init(Interface* mApp_);
	void OnShow() override;
    void Update();
protected:
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
	double PumpingSpeedFromSticking(double sticking, double area, double temperature);
	double StickingFromPumpingSpeed(double pumpingSpeed, double area, double temperature);
	void DrawSectionDebug();
	void DrawSectionViewerSettings();
    void UpdateFacetSettings();
    void ApplyFacetSettings();
	void DrawSectionSelectedFacet();
    void UpdateSimulationData();
	void DrawSectionSimulation();
	void DrawFacetTable();
	void UpdateTable();
    void UpdateSelectionFromTable(bool shift = false, bool ctrl = false);
	ImVector<FacetData> items;
    size_t nbSelectedFacets = 0, selected_facet_id = 0;
    InterfaceFacet* sel;
	std::string title, simBtnLabel;
#ifdef MOLFLOW
	MolFlow* molApp = nullptr;
#endif
    struct FacetSettings {
        enum { use_og = 0, use_og_area = 1 };
        int des_idx = 0;
        std::string exponentInput = "";
        std::string outgassingInput = "1.0";
        std::string outgassingAreaInput = "1.0";
        bool modeOfOg = use_og;
        double og = 1.0;
        double og_area = 1.0;
        std::string sfInput = "1.0";
        std::string psInput = "1.0";
        double sf = 1.0;
        double ps = 1.0;
        int sides_idx = 0;
        double opacity = 1.0;
        std::string opacityInput = "1.0";
        double temp = 1.0;
        std::string temperatureInput = "1.0";
        bool facetSettingsChanged = false;
        int prof_idx = 0;
        double area = 1.0;
    };
    FacetSettings fSet;

    std::string hit_stat;
    std::string des_stat;
    std::string leak_stat = "None";
    std::string time_stat;
    bool runningState = false;
};