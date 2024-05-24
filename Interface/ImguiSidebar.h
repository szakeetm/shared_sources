#pragma once
#include "ImguiWindowBase.h"

class ImSidebar : public ImWindow {
public:
	void Draw();
    void OnShow() override;
protected:
#if defined(MOLFLOW)
    double PumpingSpeedFromSticking(double sticking, double area, double temperature);
	double StickingFromPumpingSpeed(double pumpingSpeed, double area, double temperature);
#endif
    void Place();
    ImVec2 window_pos, window_pos_pivot, size;
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove;
    const float PAD = 1.0f;
    int corner = 0;

    const ImGuiViewport* viewport;
    bool use_work_area = true; // experiment with work area vs normal area

    float width;
    std::string sfInput = "1.0";
    std::string psInput = "1.0";
    double sf = 1.0;
    double ps = 1.0;
    int sides_idx = 0;
    double opacity = 1.0;
    std::string opacityInput = "1.0";
    double temp = 1.0;
    std::string temperatureInput = "1.0";
    double area = 1.0;
    int prof_idx = 0;
    std::string hit_stat;
    std::string des_stat;
    ImGuiTableFlags tFlags =
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit |
        /*ImGuiTableFlags_RowBg | */ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_Sortable;
};