#include "ImguiFacetMove.h"

void ShowAppFacetMove(bool* p_open, MolFlow* mApp, InterfaceGeometry* interfGeom)
{
    ImGui::PushStyleCompact();
    ImGui::SetNextWindowSize(ImVec2(250,0));
    ImGui::Begin("Facet Move", p_open,  0 | ImGuiWindowFlags_NoSavedSettings);
    ImGui::SetWindowFontScale(0.9);
    
    ImGui::RadioButton("Absolute Offset", &mode, absolute_offset);
    ImGui::RadioButton("Direction and Distance", &mode, direction_and_distance);


    if (mode == absolute_offset) // Compute labels of input fields based on selected mode
    {
        prefix = "d";
    }
    else
    {
        prefix = "dir";
    }
    
    if (!base_selected)
    {
        selection = "Nothing selected";
        dirMessage = "Choose base first";
    }

    ImGui::Text(prefix+"X"); ImGui::SameLine();
    ImGui::InputText("cm##X", &axis_X);
    ImGui::Text(prefix+"Y"); ImGui::SameLine();
    ImGui::InputText("cm##Y", &axis_Y);
    ImGui::Text(prefix+"Z"); ImGui::SameLine();
    ImGui::InputText("cm##Z", &axis_Z);
    int nextX = 0, nextY = 10 * ImGui::CalcTextSize(" ").y;
    ImGui::BeginChild("In direction", ImVec2(0, nextY), true, 0);
    {
        ImGui::Text("In direction");
        if (mode == absolute_offset) ImGui::BeginDisabled();
        {
            ImGui::Text("Distance"); ImGui::SameLine();
            ImGui::SetNextItemWidth(30 * ImGui::CalcTextSize(" ").x);
            ImGui::InputText("cm##D", &distance);
        }

        if (mode == absolute_offset) ImGui::EndDisabled();
        ImGui::PlaceAtRegionCenter("Facet normal");
        if (ImGui::Button("Facet normal"))
        {
            facetNormalButton(interfGeom);
        }
        ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders;
        ImGui::BeginTable("options", 2, table_flags);
        {
            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            {
                ImGui::PlaceAtRegionCenter("Base");
                ImGui::Text("Base");
                ImGui::PlaceAtRegionCenter(selection);
                ImGui::Text(selection);
                ImGui::PlaceAtRegionCenter("Selected Vertex");
                if (ImGui::Button("Selected Vertex##0"))
                {
                    base_selected = baseVertexSelect(interfGeom);
                }
                ImGui::PlaceAtRegionCenter("Facet center");
                if (ImGui::Button("Facet center##0"))
                {
                    base_selected = baseFacetSelect(interfGeom);
                }
            } ImGui::EndGroup();


            if (!base_selected) ImGui::BeginDisabled();

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            {
                ImGui::PlaceAtRegionCenter("Direction");
                ImGui::Text("Direction");
                ImGui::PlaceAtRegionCenter(dirMessage);
                ImGui::Text(dirMessage);
                ImGui::PlaceAtRegionCenter("Selected Vertex");

                if (ImGui::Button("Selected Vertex##1"))
                {
                    dirVertex(interfGeom);
                }
                ImGui::PlaceAtRegionCenter("Facet center");
                if (ImGui::Button("Facet center##1"))
                {
                    dirFacetCenter(interfGeom);
                }
            }ImGui::EndGroup();

            if (!base_selected) ImGui::EndDisabled();
        }
        ImGui::EndTable();

    }
    ImGui::EndChild();
    ImGui::PlaceAtRegionCenter("Move facets   Copy facets");
    if (ImGui::Button("Move facets"))
    {
        executeMove(mApp, interfGeom, false);
    } ImGui::SameLine();
    if (ImGui::Button("Copy facets"))
    {
        executeMove(mApp, interfGeom, true);
    }

    if (popup) //popup for any errors
    {
        ImGui::OpenPopup("Error");
        float nextX = 100 * ImGui::CalcTextSize(" ").x, nextY = 10 * ImGui::CalcTextSize(" ").y;
        ImGui::SetNextWindowSize(ImVec2(nextX, nextY));
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2 - nextX /2, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2 - nextY /2));
        if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
        {
            ImGui::PlaceAtRegionCenter(message);
            ImGui::Text(message);
            ImGui::PlaceAtRegionCenter("OK");
            if (ImGui::Button("OK"))
            {
                ImGui::CloseCurrentPopup();
                popupToggle();
            }
            ImGui::EndPopup();
        }
    }

    ImGui::SetWindowFontScale(1.0);
    ImGui::End();
    ImGui::PopStyleCompact();
}

bool baseVertexSelect(InterfaceGeometry* interfGeom)
{
    auto selVertices = interfGeom->GetSelectedVertices();
    if (selVertices.size() != 1)
    {
        message = "Select exactly one vertex";
        popupToggle();
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

bool baseFacetSelect(InterfaceGeometry* interfGeom)
{
    auto selFacets = interfGeom->GetSelectedFacets();
    if (selFacets.size() != 1) {
        message = "Select exactly one facet";
        popupToggle();
        return false;
    }
    baseLocation = interfGeom->GetFacet(selFacets[0])->sh.center;
    selection = fmt::format("Center of facet {}", selFacets[0] + 1);
    dirMessage = " ";
    return true;
}

void popupToggle()
{
    popup = !popup;
}

void executeMove(MolFlow* mApp, InterfaceGeometry* interfGeom, bool copy)
{
    double X, Y, Z, D{0};
    //handle input errors
    if (interfGeom->GetNbSelectedFacets() == 0) {
        message = "No facets selected";
        popupToggle();
        return;
    }
    if (!Util::getNumber(&X,axis_X)){
        message = "Invalid X offset/direction";
        popupToggle();
        return;
    }
    if (!Util::getNumber(&Y, axis_Y)) {
        message = "Invalid Y offset/direction";
        popupToggle();
        return;
    }
    if (!Util::getNumber(&Z, axis_Z)) {
        message = "Invalid Z offset/direction";
        popupToggle();
        return;
    }
    bool towardsDirectionMode = mode == direction_and_distance;
    if (towardsDirectionMode && !Util::getNumber(&D, distance)) {
        message = "Invalid offset distance";
        popupToggle();
        return;
    }
    if (towardsDirectionMode && X == 0.0 && Y == 0.0 && Z == 0.0) {
        message = "Direction can't be null-vector";
        popupToggle();
        return;
    }
    //execute
    if (mApp->AskToReset()) {
        bool imGui = true;
        interfGeom->MoveSelectedFacets(X, Y, Z, towardsDirectionMode, D, copy); //make D optional to avoid errors in debug mode
        mApp->worker.MarkToReload();
        mApp->changedSinceSave = true;
        mApp->UpdateFacetlistSelected();
        mApp->UpdateViewers();
    }
}

void facetNormalButton(InterfaceGeometry* interfGeom)
{
    auto selFacets = interfGeom->GetSelectedFacets();
    if (selFacets.size() != 1) {
        message = "Select exactly one facet";
        popupToggle();
        return;
    }
    Vector3d facetNormal = interfGeom->GetFacet(selFacets[0])->sh.N;
    axis_X=std::to_string(facetNormal.x);
    axis_Y=std::to_string(facetNormal.y);
    axis_Z=std::to_string(facetNormal.z);

    //Switch to direction mode
    mode = direction_and_distance;
}

void dirVertex(InterfaceGeometry* interfGeom)
{
    auto selVertices = interfGeom->GetSelectedVertices();
    if (selVertices.size() != 1) {
        message = "Select exactly one vertex";
        popupToggle();
        return;
    }
    Vector3d translation = *(interfGeom->GetVertex(selVertices[0])) - baseLocation;

    axis_X= std::to_string(translation.x);
    axis_Y= std::to_string(translation.y);
    axis_Z= std::to_string(translation.z);
    distance = std::to_string(translation.Norme());

    dirMessage = fmt::format("Vertex {}", selVertices[0] + 1);
}

void dirFacetCenter(InterfaceGeometry* interfGeom)
{
    auto selFacets = interfGeom->GetSelectedFacets();
    if (selFacets.size() != 1) {
        message = "Select exactly one facet";
        popupToggle();
        return;
    }
    Vector3d translation = interfGeom->GetFacet(selFacets[0])->sh.center - baseLocation;

    axis_X = std::to_string(translation.x);
    axis_Y = std::to_string(translation.y);
    axis_Z = std::to_string(translation.z);
    distance = std::to_string(translation.Norme());

    dirMessage = fmt::format("Center of facet {}", selFacets[0] + 1);
}