#include "ImguiVertexCoordinates.h"
#include "ImguiVertexCoordinates.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Interface.h"
#include "ImguiWindow.h"
#include "Helper/StringHelper.h"

void ImVertexCoordinates::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 20), ImGuiCond_FirstUseEver);
	ImGui::Begin("Vertex coordinates", &drawn, ImGuiWindowFlags_NoSavedSettings);
	ImGui::BeginChild("###VCTC", ImVec2(0, ImGui::GetContentRegionAvail().y - 1.3 * txtH), ImGuiChildFlags_Border);
	DrawTable();
	ImGui::EndChild();
	if (ImGui::Button("X")) {
		mApp->imWnd->input.Open("Set all X coordinates to:", "New coordinate", [this](std::string v) { SetAllTo(X, v); });
	};
	ImGui::SameLine();
	if (ImGui::Button("Y")) {
		mApp->imWnd->input.Open("Set all Y coordinates to:", "New coordinate", [this](std::string v) { SetAllTo(Y, v); });
	};
	ImGui::SameLine();
	if (ImGui::Button("Z")) {
		mApp->imWnd->input.Open("Set all Z coordinates to:", "New coordinate", [this](std::string v) { SetAllTo(Z, v); });
	};
	ImGui::SameLine();
	float dummyWidth = ImGui::GetContentRegionAvail().x - 7 * txtW;
	ImGui::Dummy(ImVec2(dummyWidth, 0)); ImGui::SameLine();
	if (ImGui::Button("Apply")) {
		ApplyButtonPress();
	}
	ImGui::End();
}

void ImVertexCoordinates::UpdateFromSelection(const std::vector<size_t>& selectedVertices)
{
	data.clear();
	data.reserve(selectedVertices.size());
	for (auto& vId : selectedVertices) {
		InterfaceVertex* v = interfGeom->GetVertex(vId);
		vCoords vData;
		vData.vertexId = vId;
		vData.xIn = fmt::format("{}", v->x);
		vData.yIn = fmt::format("{}", v->y);
		vData.zIn = fmt::format("{}", v->z);
		data.push_back(vData);
	}
}

void ImVertexCoordinates::UpdateFromSelection()
{
	UpdateFromSelection(interfGeom->GetSelectedVertices());
}

void ImVertexCoordinates::DrawTable()
{
	if (ImGui::BeginTable("###VCT", 4, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders)) {
		ImGui::TableSetupColumn("Vertex #", ImGuiTableColumnFlags_WidthFixed, txtW * 7);
		ImGui::TableSetupColumn("X");
		ImGui::TableSetupColumn("Y");
		ImGui::TableSetupColumn("Z");
		ImGui::TableHeadersRow();

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, txtH * 0.1));  // Adjusts row height
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));     // No padding between cells

		for (vCoords& v : data) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(fmt::format("{}", v.vertexId + 1));

			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
			ImGui::InputText(fmt::format("###{}-X", v.vertexId), &v.xIn);

			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
			ImGui::InputText(fmt::format("###{}-Y", v.vertexId), &v.yIn);

			ImGui::TableSetColumnIndex(3);
			ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
			ImGui::InputText(fmt::format("###{}-Z", v.vertexId), &v.zIn);
		}

		ImGui::EndTable();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	}
}

void ImVertexCoordinates::ApplyButtonPress()
{
	size_t row = 1; // user facing so indexing from 1
	for (vCoords& v : data) {
		if (!Util::getNumber(&v.x, v.xIn)) {
			ImIOWrappers::InfoPopup("Error", fmt::format("Invalid X coordinate in row {}", row));
			return;
		}
		if (!Util::getNumber(&v.y, v.yIn)) {
			ImIOWrappers::InfoPopup("Error", fmt::format("Invalid Y coordinate in row {}", row));
			return;
		}
		if (!Util::getNumber(&v.z, v.zIn)) {
			ImIOWrappers::InfoPopup("Error", fmt::format("Invalid Z coordinate in row {}", row));
			return;
		}
	}
	mApp->imWnd->popup.Open("Apply?", "Apply changes to geometry?", { std::make_shared<ImIOWrappers::ImButtonFunc>("Ok", ([this]() { Apply(); }), ImGuiKey_Enter, ImGuiKey_KeypadEnter),
		std::make_shared<ImIOWrappers::ImButtonInt>("Cancel",0,ImGuiKey_Escape)});
}

void ImVertexCoordinates::Apply()
{
	LockWrapper lW(mApp->imguiRenderLock);
	if (!mApp->AskToReset(&mApp->worker)) return;

	for (vCoords& v : data) { // second loop to apply
		mApp->changedSinceSave = true;
		interfGeom->MoveVertexTo(v.vertexId, v.x, v.y, v.z);
	}
	interfGeom->Rebuild();
	mApp->worker.MarkToReload();
}

void ImVertexCoordinates::SetAllTo(Axis axis, std::string val)
{
	for (vCoords& v : data) {
		switch (axis)
		{
		case ImVertexCoordinates::X:
			v.xIn = val;
			break;
		case ImVertexCoordinates::Y:
			v.yIn = val;
			break;
		case ImVertexCoordinates::Z:
			v.zIn = val;
			break;
		default:
			break;
		}
	}
}
