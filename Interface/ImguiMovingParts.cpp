#include "ImguiMovingParts.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "Interface.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

void ImMovingParts::Draw() {
    if(!drawn) return;
    ImGui::SetNextWindowPos(ImVec2(5*txtW, 3*txtH), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(txtW*75,txtH*16), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(txtW*75,txtH*16), ImVec2(txtW*100,txtH*20));
    ImGui::Begin("Define moving parts", &drawn, ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextWrapped("Movement paramenters set here will only apply to facets whih are marked \"moving\" in their parameters");
    ImGui::BeginChild("###movementChild", ImVec2(0,ImGui::GetContentRegionAvail().y-1.5*txtH) , true);
    {
        if(ImGui::RadioButton("No moving parts", mode==Modes::None)) mode = Modes::None;
        if(ImGui::RadioButton("Fixed (same velocity vector everywhere)", mode==Modes::Fixed)) mode = Modes::Fixed;
        {
            if(mode!=Modes::Fixed) ImGui::BeginDisabled();
            if(ImGui::BeginTable("###MovMartT1",7,ImGuiTableFlags_SizingStretchProp, ImVec2(ImGui::GetContentRegionAvail().x-txtW*20,0))) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Velocity vector [m/s]");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("vx");
                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(txtW*6);
                ImGui::InputText("##vx", &vxI);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("vy");
                ImGui::TableSetColumnIndex(4);
                ImGui::SetNextItemWidth(txtW*6);
                ImGui::InputText("##vy", &vyI);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("vz");
                ImGui::TableSetColumnIndex(6);
                ImGui::SetNextItemWidth(txtW*6);
                ImGui::InputText("##vz", &vzI);

                ImGui::EndTable();
            }

            if(mode!=Modes::Fixed) ImGui::EndDisabled();
        }
        if(ImGui::RadioButton("Rotation around axis", mode==Modes::Rotation)) mode = Modes::Rotation;
        {
            if(mode!=Modes::Rotation) ImGui::BeginDisabled();
            
            if(ImGui::BeginTable("###MovingPartsTable",8,ImGuiTableFlags_SizingStretchProp)){
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Axis base point");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("ax");
                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(txtW*6);
                ImGui::InputText("##ax", &axI);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("ay");
                ImGui::TableSetColumnIndex(4);
                ImGui::SetNextItemWidth(txtW*6);
                ImGui::InputText("##ay", &ayI);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("az");
                ImGui::TableSetColumnIndex(6);
                ImGui::SetNextItemWidth(txtW*6);
                ImGui::InputText("##az", &azI);
                ImGui::TableSetColumnIndex(7);
                if(ImGui::Button("Use selected vertex")) {
                    SelectedVertAsAxisOriginButtonPress();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Axis direction");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("rx");
                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(txtW*6);
                ImGui::InputText("##rx", &rxI);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("ry");
                ImGui::TableSetColumnIndex(4);
                ImGui::SetNextItemWidth(txtW*6);
                ImGui::InputText("##ry", &ryI);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("rz");
                ImGui::TableSetColumnIndex(6);
                ImGui::SetNextItemWidth(txtW*6);
                ImGui::InputText("##rz", &rzI);
                ImGui::TableSetColumnIndex(7);
                if(ImGui::Button("Base to sel. vertex")) {
                    SelectedVertAsAxisDirectionButtonPress();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Rotation speed");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("RPM");
                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(txtW*6);
                if(ImGui::InputText("##rpm", &rpmI)) {
                    if(Util::getNumber(&rpm, rpmI)) {
                        deg = rpm*6;
                        hz = rpm/60;
                        degI = fmt::format("{:.3g}", deg);
                        hzI = fmt::format("{:.3g}", hz);
                    }
                }
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("deg/s");
                ImGui::TableSetColumnIndex(4);
                ImGui::SetNextItemWidth(txtW*6);
                if(ImGui::InputText("##deg", &degI)) {
                    if(Util::getNumber(&deg, degI)) {
                        rpm = deg/6;
                        hz = deg/360;
                        rpmI = fmt::format("{:.3g}", rpm);
                        hzI = fmt::format("{:.3g}", hz);
                    }
                }
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("Hz");
                ImGui::TableSetColumnIndex(6);
                ImGui::SetNextItemWidth(txtW*6);
                if(ImGui::InputText("##Hz", &hzI)) {
                    if(Util::getNumber(&hz, hzI)) {
                        deg = hz*360;
                        rpm = hz*60;
                        degI = fmt::format("{:.3g}", deg);
                        rpmI = fmt::format("{:.3g}", rpm);
                    }
                }

                ImGui::EndTable();
            }
            if(mode!=Modes::Rotation) ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    ImGui::PlaceAtRegionCenter(" Apply ");
    if(ImGui::Button("Apply")) {
        ApplyButtonPress();
    }
    ImGui::End();
}

void ImMovingParts::ApplyButtonPress()
{
    if(mode==Modes::Fixed){
        if(!Util::getNumber(&vx, vxI)) ImIOWrappers::InfoPopup("Error", "Invalid vx value");
        if(!Util::getNumber(&vy, vyI)) ImIOWrappers::InfoPopup("Error", "Invalid vy value");
        if(!Util::getNumber(&vz, vzI)) ImIOWrappers::InfoPopup("Error", "Invalid vz value");
    }
    if(mode==Modes::Rotation) {
        if(!Util::getNumber(&ax, axI)) ImIOWrappers::InfoPopup("Error", "Invalid ax value");
        if(!Util::getNumber(&ay, ayI)) ImIOWrappers::InfoPopup("Error", "Invalid ay value");
        if(!Util::getNumber(&az, azI)) ImIOWrappers::InfoPopup("Error", "Invalid az value");

        if(!Util::getNumber(&rx, rxI)) ImIOWrappers::InfoPopup("Error", "Invalid rx value");
        if(!Util::getNumber(&ry, ryI)) ImIOWrappers::InfoPopup("Error", "Invalid ry value");
        if(!Util::getNumber(&rz, rzI)) ImIOWrappers::InfoPopup("Error", "Invalid rz value");

        if(!Util::getNumber(&rpm, rpmI)) ImIOWrappers::InfoPopup("Error", "Invalid rpm value");
        if(!Util::getNumber(&deg, degI)) ImIOWrappers::InfoPopup("Error", "Invalid deg/s value");
        if(!Util::getNumber(&hz, hzI)) ImIOWrappers::InfoPopup("Error", "Invalid Hz value");
    }

    switch(mode) {
        case Modes::Fixed:
            AXIS_DIR.x = vx; AXIS_DIR.y = vy; AXIS_DIR.z = vz;
            break;
        case Modes::Rotation:
            AXIS_P0.x = ax; AXIS_P0.y = ay; AXIS_P0.z = az;
            AXIS_DIR.x = rx; AXIS_DIR.y = ry; AXIS_DIR.z = rz;
            if(AXIS_DIR.Norme()<1E-5){
                ImIOWrappers::InfoPopup("Error", "The rotation vector is shorter than 1E-5 cm.\n"
                    "Very likely this is a null vector\n"
                    "If not, increase its coefficients while keeping its direction");
                return;
            }
            break;
    }
    ImIOWrappers::AskToSaveBeforeDoing( [this]() { Apply(); });
}

void ImMovingParts::Apply(){
    LockWrapper lw(mApp->imguiRenderLock);
    if (!mApp->AskToReset()) return;
    mApp->worker.model->sp.motionType = (int)mode;
    switch (mode) {
    case Modes::Fixed:
        mApp->worker.model->sp.motionVector2 = AXIS_DIR;
        break;
    case Modes::Rotation: 
        mApp->worker.model->sp.motionVector1 = AXIS_P0;
        mApp->worker.model->sp.motionVector2 = AXIS_DIR.Normalized() * (deg / 180.0 * 3.14159);
        break;
    }

    mApp->worker.MarkToReload(); 
    mApp->UpdateFacetlistSelected();
    mApp->UpdateViewers();
    mApp->changedSinceSave = true;
}

void ImMovingParts::SelectedVertAsAxisOriginButtonPress()
{
    size_t nbs = interfGeom->GetNbSelectedVertex();
    if (nbs != 1) {
        ImIOWrappers::InfoPopup("Error", fmt::format("Select exactly one vertex\n(You have selected {}).", nbs));
        return;
    }
    else {
        for (int i = 0; i < interfGeom->GetNbVertex(); i++) {
            if (interfGeom->GetVertex(i)->selected) {
                ax = interfGeom->GetVertex(i)->x;
                axI = fmt::format("{:.3g}", ax);
                ay = interfGeom->GetVertex(i)->y;
                ayI = fmt::format("{:.3g}",ay);
                az = interfGeom->GetVertex(i)->z;
                azI = fmt::format("{:.3g}",az);
                break;
            }
        }
    }
}

void ImMovingParts::SelectedVertAsAxisDirectionButtonPress()
{
    size_t nbs = interfGeom->GetNbSelectedVertex();
    if (nbs != 1) {
        ImIOWrappers::InfoPopup("Error", fmt::format("Select exactly one vertex\n(You have selected {}).", nbs));
        return;
    }
    else {
        for (int i = 0; i < interfGeom->GetNbVertex(); i++) {
            if (interfGeom->GetVertex(i)->selected) {
                rxI = fmt::format("{:.3g}", interfGeom->GetVertex(i)->x-ax);
                ryI = fmt::format("{:.3g}", interfGeom->GetVertex(i)->y-ay);
                rzI = fmt::format("{:.3g}", interfGeom->GetVertex(i)->z-az);
                break;
            }
        }
    }
}

void ImMovingParts::Update() { // get values from selection / geometry
    mode = static_cast<Modes>(mApp->worker.model->sp.motionType);

    vx = 0;
    vxI = "0";
    vy = 0;
    vyI = "0";
    vz = 0;
    vzI = "0";

    ax = 0;
    axI = "0";
    ay = 0;
    ayI = "0";
    az = 0;
    azI = "0";
    rx = 0;
    rxI = "0";
    ry = 0;
    ryI = "0";
    rz = 0;
    rzI = "0";

    rpm = 0;
    rpmI = "0";
    deg = 0;
    degI = "0";
    hz = 0;
    hzI = "0";

    if (mode == Fixed) {
        vx = mApp->worker.model->sp.motionVector2.x;
        vxI = fmt::format("{:.3g", vx);
        vy = mApp->worker.model->sp.motionVector2.y;
        vyI = fmt::format("{:.3g", vy);
        vz = mApp->worker.model->sp.motionVector2.z;
        vzI = fmt::format("{:.3g", vz);
    }
    else if (mode == Rotation) {
        ax = mApp->worker.model->sp.motionVector1.x;
        axI = fmt::format("{:.3g", ax);
        ay = mApp->worker.model->sp.motionVector1.y;
        ayI = fmt::format("{:.3g", ay);
        az = mApp->worker.model->sp.motionVector1.z;
        azI = fmt::format("{:.3g", az);
        Vector3d rot = mApp->worker.model->sp.motionVector2.Normalized();
        rx = rot.x;
        rxI = fmt::format("{:.3g}", rx);
        ry = rot.y;
        ryI = fmt::format("{:.3g}", ry);
        rz = rot.z;
        rzI = fmt::format("{:.3g}", rz);
        deg = mApp->worker.model->sp.motionVector2.Norme() / 3.14159 * 180.0;
        rpm = deg / 6;
        rpmI = fmt::format("{:.3g}", rpm);
        hz = deg / 360;
        hzI = fmt::format("{:.3g}", hz);
    }
}