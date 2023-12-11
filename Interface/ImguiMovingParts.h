#pragma once
#include "ImguiWindowBase.h"
#include <string>

class ImMovingParts : public ImWindow {
public:
    void Draw();
    void Update();
protected:
    void ApplyButtonPress();
    void Apply();
    void SelectedVertAsAxisOriginButtonPress();
    void SelectedVertAsAxisDirectionButtonPress();
    enum Modes : int {
        None=0,
        Fixed=1,
        Rotation=2
    };

    Vector3d AXIS_P0, AXIS_DIR;

    Modes mode = None;
    std::string vxI="0", vyI="0", vzI="0", axI="0", ayI="0", azI="0", rxI="0", ryI="0",rzI="0", rpmI="0", degI="0", hzI="0";
    double vx=0, vy=0, vz=0, ax=0, ay=0, az=0, rx=0, ry=0, rz=0, rpm=0, deg=0, hz=0;
};