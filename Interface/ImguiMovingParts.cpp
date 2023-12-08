#include "ImguiMovingParts.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImMovingParts::Draw() {
    if(!drawn) return;
    ImGui::SetNextWindowPos(ImVec2(5*txtW, 3*txtH), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(txtW*75,txtH*15), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(txtW*75,txtH*15), ImVec2(txtW*100,txtH*60));
    ImGui::Begin("Define moving parts", &drawn, ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextWrapped("Movement paramenters set here will only apply to facets whih are marked \"moving\" in their parameters");
    ImGui::BeginChild("###movementChild", ImVec2(0,ImGui::GetContentRegionAvail().y-1.5*txtH) , true);
    {
        if(ImGui::RadioButton("No moving parts", mode==Modes::None)) mode = Modes::None;
        if(ImGui::RadioButton("Fixed (same velocity vector everywhere)", mode==Modes::Fixed)) mode = Modes::Fixed;
        {
            if(mode!=Modes::Fixed) ImGui::BeginDisabled();
            if(ImGui::BeginTable("###MovMartT1",7,ImGuiTableFlags_SizingStretchProp, ImVec2(ImGui::GetContentRegionAvailWidth()-txtW*20,0))) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Velocity vector [m/s]");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("vx");
                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##vx", &vxI);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("vy");
                ImGui::TableSetColumnIndex(4);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##vy", &vyI);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("vz");
                ImGui::TableSetColumnIndex(6);
                ImGui::SetNextItemWidth(txtW*4);
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
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##ax", &axI);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("ay");
                ImGui::TableSetColumnIndex(4);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##ay", &ayI);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("az");
                ImGui::TableSetColumnIndex(6);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##az", &azI);
                ImGui::TableSetColumnIndex(7);
                if(ImGui::Button("Use selected vertex")) {}

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Axis direction");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("rx");
                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##rx", &rxI);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("ry");
                ImGui::TableSetColumnIndex(4);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##ry", &ryI);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("rz");
                ImGui::TableSetColumnIndex(6);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##rz", &rzI);
                ImGui::TableSetColumnIndex(7);
                if(ImGui::Button("Base to sel. vertex")) {}

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Rotation speed");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("RMP");
                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##rmp", &rpmI);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("deg/s");
                ImGui::TableSetColumnIndex(4);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##deg", &degI);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("Hz");
                ImGui::TableSetColumnIndex(6);
                ImGui::SetNextItemWidth(txtW*4);
                ImGui::InputText("##Hz", &hzI);

                ImGui::EndTable();
            }
            if(mode!=Modes::Rotation) ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    ImGui::PlaceAtRegionCenter(" Apply ");
    if(ImGui::Button("Apply")) {

    }
    ImGui::End();
}