#include "ImguiFacetMove.h"

void ShowAppFacetMove(bool* p_open, Interface* mApp, InterfaceGeometry* interfGeom)
{
    int txtW = ImGui::CalcTextSize(" ").x;
    int txtH = ImGui::GetTextLineHeightWithSpacing();
    float width, cursorY;
    ImGui::SetNextWindowSize(ImVec2(80 * txtW,0));
    ImGui::Begin("Facet Move", p_open,  0 | ImGuiWindowFlags_NoSavedSettings);
    
    ImGui::RadioButton("Absolute Offset", &mode, absolute_offset);
    ImGui::RadioButton("Direction and Distance", &mode, direction_and_distance);


    if (mode == absolute_offset) { // Compute labels of input fields based on selected mode
        prefix = "d";
    }
    else {
        prefix = "dir";
    }
    
    if (!base_selected) {
        selection = "Nothing selected";
        dirMessage = "Choose base first";
    }

    ImGui::Text(prefix+"X"); ImGui::SameLine();
    ImGui::InputText("cm##X", &axis_X);
    ImGui::Text(prefix+"Y"); ImGui::SameLine();
    ImGui::InputText("cm##Y", &axis_Y);
    ImGui::Text(prefix+"Z"); ImGui::SameLine();
    ImGui::InputText("cm##Z", &axis_Z);
    ImGui::BeginChild("In direction", ImVec2(0, 9.0f*txtH), true, 0);
    {
        ImGui::Text("In direction"); {
            if (mode == absolute_offset) {
                ImGui::BeginDisabled();
            }
        }

        {
            ImGui::Text("         Distance"); ImGui::SameLine();
            ImGui::SetNextItemWidth(23 * txtW);
            ImGui::InputText("cm##D", &distance);
        }

        if (mode == absolute_offset) {
            ImGui::EndDisabled();
        }

        ImGui::PlaceAtRegionCenter("Facet normal");
        if (ImGui::Button("Facet normal")) {
            FacetNormalButtonPress(interfGeom);
        }
        ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH;
        if (ImGui::BeginTable("options", 2, table_flags)) {
            ImGui::TableNextRow(); //Set up top row
            ImGui::TableSetColumnIndex(0);
            ImGui::BeginGroup();
            ImGui::PlaceAtRegionCenter("Base");
            ImGui::Text("Base");
            ImGui::EndGroup();
            ImGui::TableSetColumnIndex(1);
            if (!base_selected) {
                ImGui::BeginDisabled();
            }
            ImGui::BeginGroup();
            ImGui::PlaceAtRegionCenter("Direction");
            ImGui::Text("Direction"); 
            ImGui::EndGroup();
            if (!base_selected) {
                ImGui::EndDisabled();
            }

            ImGui::TableNextRow(); //Set up message row
            ImGui::TableSetColumnIndex(0);
            ImGui::BeginGroup();
            ImGui::PlaceAtRegionCenter(selection);
            ImGui::Text(selection);
            ImGui::EndGroup();
            ImGui::TableSetColumnIndex(1);
            if (!base_selected) {
                ImGui::BeginDisabled();
            }
            ImGui::BeginGroup();
            ImGui::PlaceAtRegionCenter(dirMessage);
            ImGui::Text(dirMessage);
            if (!base_selected) {
                ImGui::EndDisabled();
            }
            ImGui::EndGroup();
            ImGui::TableNextRow(); //Set up buttons row
            ImGui::TableSetColumnIndex(0);
            ImGui::BeginGroup();
            {                
                ImGui::PlaceAtRegionCenter(" Selected Vertex ");
                width = ImGui::CalcTextSize(" Selected Vertex ").x;
                if (ImGui::Button("Selected Vertex##0")) {
                    base_selected = BaseVertexSelectButtonPress(interfGeom);
                }
                ImGui::PlaceAtRegionCenter(" Selected Vertex ");
                if (ImGui::Button("Facet center##0", ImVec2(width, 0))) {
                    base_selected = BaseFacetSelectButtonPress(interfGeom);
                }
            } ImGui::EndGroup();

            ImGui::TableSetColumnIndex(1);
            ImGui::BeginGroup();
            {
                if (!base_selected) {
                    ImGui::BeginDisabled();
                }

                ImGui::PlaceAtRegionCenter(" Selected Vertex ");
                if (ImGui::Button("Selected Vertex##1"))
                {
                    VertexDirectionButtonPress(interfGeom);
                }
                ImGui::PlaceAtRegionCenter(" Selected Vertex ");
                if (ImGui::Button("Facet center##1",ImVec2(width,0)))
                {
                    FacetCenterButtonPress(interfGeom);
                }
                if (!base_selected) {
                    ImGui::EndDisabled();
                }
            }ImGui::EndGroup();
            ImGui::EndTable();
        }

    }
    ImGui::EndChild();
    ImGui::PlaceAtRegionCenter("Move facets                        Copy facets"); //This is very ugly TODO:Find a better way
    if (ImGui::Button("Move facets", ImVec2(width, 0))) {
        ExecuteFacetMove(mApp, interfGeom, false);
    }
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(6 * txtW,txtH));
    ImGui::SameLine();
    if (ImGui::Button("Copy facets", ImVec2(width, 0))) {
        ExecuteFacetMove(mApp, interfGeom, true);
    }

    ImGui::End();
}

bool BaseVertexSelectButtonPress(InterfaceGeometry* interfGeom) {
    auto selVertices = interfGeom->GetSelectedVertices();
    if (selVertices.size() != 1)
    {
        ImguiPopup::Popup("Select exactly one vertex", "Error");
        return false;
    }
    else
    {
        baseLocation = (Vector3d) * (interfGeom->GetVertex(selVertices[0]));
        selection = fmt::format("Vertex {}", selVertices[0] + 1);
        dirMessage = " ";
        return true;
    }
}

bool BaseFacetSelectButtonPress(InterfaceGeometry* interfGeom) {
    auto selFacets = interfGeom->GetSelectedFacets();
    if (selFacets.size() != 1) {
        ImguiPopup::Popup("Select exactly one facet", "Error");
        return false;
    }
    baseLocation = interfGeom->GetFacet(selFacets[0])->sh.center;
    selection = fmt::format("Center of facet {}", selFacets[0] + 1);
    dirMessage = " ";
    return true;
}

void ExecuteFacetMove(Interface* mApp, InterfaceGeometry* interfGeom, bool copy) {
    double X, Y, Z, D{0};
    //handle input errors
    if (interfGeom->GetNbSelectedFacets() == 0) {
        ImguiPopup::Popup("No facets selected", "Error");
        return;
    }
    if (!Util::getNumber(&X,axis_X)){
        ImguiPopup::Popup("Invalid X offset/direction", "Error");
        return;
    }
    if (!Util::getNumber(&Y, axis_Y)) {
        ImguiPopup::Popup("Invalid Y offset/direction", "Error");
        return;
    }
    if (!Util::getNumber(&Z, axis_Z)) {
        ImguiPopup::Popup("Invalid Z offset/direction", "Error");
        return;
    }
    bool towardsDirectionMode = mode == direction_and_distance;
    if (towardsDirectionMode && !Util::getNumber(&D, distance)) {
        ImguiPopup::Popup("Invalid offset distance", "Error");
        return;
    }
    if (towardsDirectionMode && X == 0.0 && Y == 0.0 && Z == 0.0) {
        ImguiPopup::Popup("Direction can't be null-vector", "Error");
        return;
    }
    //execute

    {
        LockWrapper myLock(mApp->imguiRenderLock);
        if (mApp->AskToReset()) {
            interfGeom->MoveSelectedFacets(X, Y, Z, towardsDirectionMode, D, copy); //make D optional to avoid errors in debug mode
            mApp->worker.MarkToReload();
            mApp->changedSinceSave = true;
            mApp->UpdateFacetlistSelected();
            mApp->UpdateViewers();
        }
    }
}

void FacetNormalButtonPress(InterfaceGeometry* interfGeom)
{
    auto selFacets = interfGeom->GetSelectedFacets();
    if (selFacets.size() != 1) {
        ImguiPopup::Popup("Select exactly one facet", "Error");
        return;
    }
    Vector3d facetNormal = interfGeom->GetFacet(selFacets[0])->sh.N;
    axis_X=std::to_string(facetNormal.x);
    axis_Y=std::to_string(facetNormal.y);
    axis_Z=std::to_string(facetNormal.z);

    //Switch to direction mode
    mode = direction_and_distance;
}

void VertexDirectionButtonPress(InterfaceGeometry* interfGeom)
{
    auto selVertices = interfGeom->GetSelectedVertices();
    if (selVertices.size() != 1) {
        ImguiPopup::Popup("Select exactly one vertex", "Error");
        return;
    }
    Vector3d translation = *(interfGeom->GetVertex(selVertices[0])) - baseLocation;

    axis_X= std::to_string(translation.x);
    axis_Y= std::to_string(translation.y);
    axis_Z= std::to_string(translation.z);
    distance = std::to_string(translation.Norme());

    dirMessage = fmt::format("Vertex {}", selVertices[0] + 1);
}

void FacetCenterButtonPress(InterfaceGeometry* interfGeom)
{
    auto selFacets = interfGeom->GetSelectedFacets();
    if (selFacets.size() != 1) {
        ImguiPopup::Popup("Select exactly one facet", "Error");
        return;
    }
    Vector3d translation = interfGeom->GetFacet(selFacets[0])->sh.center - baseLocation;

    axis_X = std::to_string(translation.x);
    axis_Y = std::to_string(translation.y);
    axis_Z = std::to_string(translation.z);
    distance = std::to_string(translation.Norme());

    dirMessage = fmt::format("Center of facet {}", selFacets[0] + 1);
}