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
    const float PAD = 10.0f;
    int corner = 0;

    const ImGuiViewport* viewport;
    bool use_work_area = true; // experiment with work area vs normal area

    float width;
};