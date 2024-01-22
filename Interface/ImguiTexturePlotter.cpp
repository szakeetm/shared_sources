#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "ImguiTexturePlotter.h"
#include "Geometry_shared.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Helper/MathTools.h"
#include "ImguiPopup.h"
#include <imgui_internal.h>
#include "ImguiExtensions.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

void ImTexturePlotter::Draw()
{
	if (!drawn) return;
	GetData();

	// assemble table & columns flags
	tableFlags = 0;
	tableFlags |= resizableColumns ? ImGuiTableFlags_Resizable : 0;
	tableFlags |= fitToWindow ? 0 : ImGuiTableFlags_ScrollX;
	
	std::vector<size_t> facets = interfGeom->GetSelectedFacets();
	static int lastSel = -1;
	if (facets.size() > 0) {
		if (facets[0] != selFacetId) {
			selFacetId = facets[0];
			name = "Texture Plotter [Facet #" + std::to_string(facets[0] + 1) + "]###TexturePlotter";
			selFacet = interfGeom->GetFacet(selFacetId);
		}
	} else {
		selFacet = nullptr;
		selFacetId = -1;
		width = 0;
		height = 0;
		name = "Texture Plotter []###TexturePlotter";
		selection.clear();
	}
	if (lastSel != selFacetId) {
		lastSel = selFacetId;
		isUpToDate = false; // selection changed
	}
	ImGui::SetNextWindowSizeConstraints(ImVec2(78 * txtW, 15 * txtH), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::Begin(name.c_str(), &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);
	DrawMenuBar();
	ImGui::BeginChild("##TPTab", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y - 4.5 * txtH),true);
	DrawTextureTable();
	ImGui::EndChild();
	if (ImGui::Button("Find Max")) {
		selection.clear();
		selection.push_back(std::pair<int, int>(maxY, maxX));
		scrollToSelected = true;
	} ImGui::SameLine();

	if (ImGui::Button("Force Refresh")) {
		isUpToDate = false;
	} ImGui::SameLine();

	dummyWidth = static_cast<float>(ImGui::GetContentRegionAvail().x - txtW * (31.5+3));
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	ImGui::SetNextItemWidth(30 * txtW);
	if (ImGui::BeginCombo("##View", comboOpts[viewIdx])) {
		for (short i = 0; i < comboOpts.size(); i++) {
			if (ImGui::Selectable(comboOpts[i])) {
				viewIdx = i;
				isUpToDate = false;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::HelpMarker("Click, hold and drag to select multiple\nSelect one and shift+click another to select all inbetween\nClick selected to deselect");
	ImGui::SameLine();

	ImGui::End();

}

void ImTexturePlotter::Init(Interface* mApp_)
{
	mApp = mApp_;
	interfGeom = mApp->worker.GetGeometry();
}

void ImTexturePlotter::Update()
{
	isUpToDate = false;
}

void ImTexturePlotter::DrawTextureTable()
{
	if( selFacet!=nullptr) GetData();
	if (width < 1 || height < 1) return;

	static ImVec2 selectionStart;
	static ImVec2 selectionEnd;
	static ImRect selectionRect;
	static bool hovered;

	static ImGuiIO& io = ImGui::GetIO();
	
	if (ImGui::BeginTable("##TPTable", width+1, ImGuiTableFlags_Borders	| ImGuiTableFlags_ScrollY | tableFlags | ImGuiTableFlags_RowBg)) {
		if (hovered && !isDragging && ImGui::IsMouseDragging(0,0.1f)) {
			selectionStart = ImGui::GetMousePos();
			isDragging = true;
			selection.clear();
		}

		if (isDragging) {
			selectionEnd = ImGui::GetMousePos();
			double startx, starty, endx, endy;
			startx = selectionStart.x < selectionEnd.x ? selectionStart.x : selectionEnd.x;
			starty = selectionStart.y < selectionEnd.y ? selectionStart.y : selectionEnd.y;

			endx = selectionStart.x > selectionEnd.x ? selectionStart.x : selectionEnd.x;
			endy = selectionStart.y > selectionEnd.y ? selectionStart.y : selectionEnd.y;

			selectionRect = ImRect(startx,starty,endx,endy); // in case the selection starts further down the table than it ends
		}
		ImGui::TableSetupScrollFreeze(1, 1);
		//headers
		ImGui::TableSetupColumn("v\\u", ImGuiTableColumnFlags_WidthFixed, txtW * 3); // corner
		for (int i = 0; i < width; ++i) {
			ImGui::TableSetupColumn(std::to_string(i + 1).c_str(), 0, columnWidth*txtW);
		}
		ImGui::TableNextRow();
		ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(186, 212, 243, 255));
		ImGui::TableSetColumnIndex(0);
		ImGui::Text(u8"v\u20D7\\u\u20D7");
		for (int i = 0; i < width; ++i) {
			ImGui::TableSetColumnIndex(i + 1);
			const char* column_name = ImGui::TableGetColumnName(i+1); // Retrieve name passed to TableSetupColumn()
			ImGui::PushID(i+1);
			std::string id(column_name);
			if (ImGui::Selectable(id, false)) SelectColumn(i);
			ImGui::PopID();
		}
		hovered = false;
		ImGuiListClipper clipper = ImGuiListClipper();
		clipper.Begin(height);
		while (clipper.Step()) {
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); // move to first column
				if (ImGui::Selectable(std::to_string(i + 1), false)) {	//label row
					SelectRow(i);
					scrollToSelected = false;
				}
				for (int j = 0; j <= width; j++) {
					if (j != width) {
						ImGui::TableSetColumnIndex(j+1);
					}
					bool isSelected = IsCellSelected(i,j);
					if (scrollToSelected && selection.size()==1 && isSelected) { // works but not well because ImGui is not redrawn all the time
						ImGui::SetScrollHereX(0.5f);
						ImGui::SetScrollHereY(0.5f);
					}
					if (isDragging) {
						scrollToSelected = false;
						ImRect cell(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
						if (cell.Overlaps(selectionRect) || selectionRect.Overlaps(cell)) {
							if (!Contains(selection, std::pair<int, int>(i, j - 1))) {
								selection.push_back(std::pair<int, int>(i, j - 1));
							}
						}
						else if (Contains(selection, std::pair<int, int>(i, j - 1))) {
							selection.erase(std::remove(selection.begin(), selection.end(), std::pair<int, int>(i, j-1)), selection.end());
						}
					}
					if (ImGui::IsItemHovered()) hovered = true;
					if (j == width) continue;
					if(ImGui::Selectable(data[i][j]+"###" + std::to_string(i) + "/" + std::to_string(j), isSelected)) {
						scrollToSelected = false;
						if (isDragging) continue;
						if (selection.size()==1 && io.KeysDown[SDL_SCANCODE_LSHIFT]) { // shift - box select
							BoxSelect(selection[0], std::pair<int, int>(i, j));
							continue;
						}

						selection.clear();

						if (isSelected) { // deselect if was selected
							selection.erase(std::remove(selection.begin(), selection.end(), std::pair<int,int>(i,j)), selection.end());
							continue;
						}
						selection.push_back(std::pair<int, int>(i, j)); // regular select
					}
				}
			}
		}
		ImGui::EndTable();
		if (!ImGui::IsMouseDown(0)) isDragging = false; // end drag
		if(isDragging) ImGui::GetWindowDrawList()->AddRectFilled(selectionStart, selectionEnd, IM_COL32(64, 128, 255, 64));
		if (selection.size()!=0)
		{
			ImVec4 bounds = SelectionBounds();
			selFacet->SelectElem(bounds.y, bounds.x, bounds.w - bounds.y + 1, bounds.z-bounds.x+1);
		}
		else {
			selFacet->UnselectElem();
		}
		bool anyKeyDown = false;
		for (const bool& key : io.KeysDown) {
			if (key) { anyKeyDown = true; break; }
		}
		if (selection.size() == 1 && anyKeyDown) {
			if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))		selection[0].first--;
			if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))		selection[0].first++;
			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))		selection[0].second--;
			if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))	selection[0].second++;

			if (selection[0].second < 0) selection[0].second = 0;
			if (selection[0].second > width-1) selection[0].second = width-1;
			if (selection[0].first < 0) selection[0].first = 0;
			if (selection[0].first > height-1) selection[0].first = height-1;
			scrollToSelected = true;
		}
		if (io.MouseDown[0]) scrollToSelected = false;
	}
}

void ImTexturePlotter::GetData()
{
	if (isUpToDate) return;
	if (!selFacet) {
		data.clear();
		return;
	}
	{
		LockWrapper lWrap(mApp->imguiRenderLock);
		if (!mApp->worker.ReloadIfNeeded()) return;
	}
	auto lock = GetHitLock(mApp->worker.globalState.get(), 10000);
	if (!lock) return;
	
	for (int i = 0; i < data.size(); i++) std::vector<std::string>().swap(data[i]); // empty rows
	std::vector<std::vector<std::string>>().swap(data); // empty all
	size_t nbMoments = mApp->worker.interfaceMomentCache.size();
	maxValue = 0.0f;
	width = selFacet->sh.texWidth;
	height = selFacet->sh.texHeight;
	while (data.size() < height) data.push_back(std::vector<std::string>());

	if (width == 0 || height == 0) return;

	bool error = false;
	const FacetMomentSnapshot& facetSnapshot = mApp->worker.globalState->facetStates[selFacetId].momentResults[mApp->worker.displayedMoment];
	if ((width - 1) + (height - 1) * width >= facetSnapshot.texture.size()) error = true;
	switch (viewIdx) { // whole switch copied from legacy and slightly adjusted, a lot of repeted code, perhaps could be rewritten?
	case 0: {// Cell area
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				FacetMomentSnapshot* dummPointer = nullptr;
				double val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::CellArea, 1.0, 1.0, 1.0, (int)(i + j * width), *dummPointer).value;

				if (val > maxValue) {
					maxValue = val;
					maxX = i; maxY = j;
				}

				data[j].push_back(fmt::format("{:.6g}", val));
			}
		}
		break; }
	case 1: {// MC Hits
		profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		//TextureCell *texture = (TextureCell *)((BYTE *)buffer + (selFacet->sh.hitOffset + facetHitsSize + profSize + mApp->worker.displayedMoment*w*h * sizeof(TextureCell)));
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				if (error) {
					data[j].push_back("Error");
					continue;
				}
				//int tSize = selFacet->sp.texWidth*selFacet->sp.texHeight;
				try {
					PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::MCHits, 1.0, 1.0, 1.0, (int)(i + j * width), facetSnapshot);
					double realVal = val.value;
					if (realVal > maxValue) {
						maxValue = realVal;
						maxX = i; maxY = j;
					}
					data[j].push_back(fmt::format("{}", (int)realVal));
				}
				catch (...) {
					data[j].push_back("Error");
				}
			}
		}
		break; }
	case 2: {// Impingement rate
		profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		double moleculesPerTP = mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment);
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				if (error) {
					data[j].push_back("Error");
					continue;
				}
				try {
					PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::ImpingementRate, moleculesPerTP, 1.0, mApp->worker.model->sp.gasMass, (int)(i + j * width), facetSnapshot);
					double realVal = val.value;
					if (realVal > maxValue) {
						maxValue = realVal;
						maxX = i; maxY = j;
					}
					data[j].push_back(fmt::format("{:.4g}", realVal));
				} catch (...) {
					data[j].push_back("Error");
				}
			}
		}

		break; }
	case 3: {// Particle density [1/m3]

		profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;

		double moleculesPerTP = mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment);
		double densityCorrection = selFacet->DensityCorrection();

		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				if (error) {
					data[j].push_back("Error");
					continue;
				}
				try {
					PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::ParticleDensity, moleculesPerTP, densityCorrection, mApp->worker.model->sp.gasMass, (int)(i + j * width), facetSnapshot);
					double rho = val.value;

					if (rho > maxValue) {
						maxValue = rho;
						maxX = i; maxY = j;
					}
					data[j].push_back(fmt::format("{:.6g}", rho));
				}
				catch (...) {
						data[j].push_back("Error");
					}
				}
		}

		break; }
	case 4: {// Gas density [kg/m3]

		profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;

		double moleculesPerTP = mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment);
		double densityCorrection = selFacet->DensityCorrection();

		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				if (error) {
					data[j].push_back("Error");
					continue;
				}
				try {
					PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::GasDensity, moleculesPerTP, densityCorrection, mApp->worker.model->sp.gasMass, (int)(i + j * width), facetSnapshot);
					double rho_mass = val.value;
					if (rho_mass > maxValue) {
						maxValue = rho_mass;
						maxX = i; maxY = j;
					}
					data[j].push_back(fmt::format("{:.6g}", rho_mass));
				}
				catch (...) {
					data[j].push_back("Error");
				}
			}
		}

		break; }
	case 5: {// Pressure
		profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;

		double moleculesPerTP = mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment);
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				if (error) {
					data[j].push_back("Error");
					continue;
				}
				try {
					PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::Pressure, moleculesPerTP, 1.0, mApp->worker.model->sp.gasMass, (int)(i + j * width), facetSnapshot);
					double p = val.value;

					if (p > maxValue) {
						maxValue = p;
						maxX = i; maxY = j;
					}

					data[j].push_back(fmt::format("{:.6g}", p));
				}
				catch (...) {
					data[j].push_back("Error");
				}
			}
		}
		break; }
	case 6: {// Average gas velocity [m/s]

		profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;

		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				if (error) {
					data[j].push_back("Error");
					continue;
				}
				try {
					PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::AvgGasVelocity, 1.0, 1.0, 1.0, (int)(i + j * width), facetSnapshot);
					double realVal = val.value;

					if (realVal > maxValue) {
						maxValue = realVal;
						maxX = i; maxY = j;
					}
					data[j].push_back(fmt::format("{:.6g}", realVal));
				}
				catch (...) {
					data[j].push_back("Error");
				}
			}
		}
		break; }
	case 7: {// Gas velocity vector

		profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		size_t nbElem = selFacet->sh.texWidth * selFacet->sh.texHeight;
		size_t tSize = nbElem * sizeof(TextureCell);
		size_t dSize = nbElem * sizeof(DirectionCell);
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				if (selFacet->sh.countDirection && facetSnapshot.direction.size() != 0) {
					if (error) {
						data[j].push_back("Error");
						continue;
					}
					try {
						PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::GasVelocityVector, 1.0, 1.0, 1.0, (int)(i + j * width), facetSnapshot);
						Vector3d v_vect = val.vect;

						std::string out;
						out.append(fmt::format("{:.4g}, ", v_vect.x));
						out.append(fmt::format("{:.4g}, ", v_vect.y));
						out.append(fmt::format("{:.4g}", v_vect.z));
						data[j].push_back(out);

						double length = v_vect.Norme();
						if (length > maxValue) {
							maxValue = length;
							maxX = i; maxY = j;
						}
					}
					catch (...) {
						data[j].push_back("Error");
					}
				}
				else {
					data[j].push_back("Direction not recorded");
				}
			}
		}
		break; }
	case 8: {// Nb of velocity vectors

		profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		size_t nbElem = selFacet->sh.texWidth * selFacet->sh.texHeight;
		size_t tSize = nbElem * sizeof(TextureCell);
		size_t dSize = nbElem * sizeof(DirectionCell);

		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				if (selFacet->sh.countDirection) {
					if (error) {
						data[j].push_back("Error");
						continue;
					}
					try {
						PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::NbVelocityVectors, 1.0, 1.0, 1.0, (int)(i + j * width), facetSnapshot);
						size_t count = val.count;

						data[j].push_back(fmt::format("{}", (int)count));
						double countEq = (double)count;
						if (countEq > maxValue) {
							maxValue = countEq;
							maxX = i; maxY = j;
						}
					}
					catch (...) {
					data[j].push_back("Error");
				}
				}
				else {
					data[j].push_back("Direction not recorded");
				}
			}
		}
		break; }
	}
	isUpToDate = true;
}

bool ImTexturePlotter::IsCellSelected(size_t row, size_t col)
{
	bool out = false;

	for (const auto& pair : selection) {
		if (pair.first == row && pair.second == col) return true;
	}
	return out;
}

void ImTexturePlotter::SelectRow(size_t row)
{
	selection.clear();
	for (size_t i = 0; i < width; i++) {
		selection.push_back(std::pair<int, int>(row,i));
	}
}

void ImTexturePlotter::SelectColumn(size_t col)
{
	selection.clear();
	for (size_t i = 0; i < width; i++) {
		selection.push_back(std::pair<int, int>(i, col));
	}
}

void ImTexturePlotter::BoxSelect(const std::pair<int, int>& start, const std::pair<int, int>& end)
{
	int startRow, startCol, endRow, endCol;
	
	startRow = start.first < end.first ? start.first : end.first;
	startCol = start.second < end.second ? start.second : end.second;

	endRow = start.first > end.first ? start.first : end.first;
	endCol = start.second > end.second ? start.second : end.second;

	for (size_t row = startRow; row <= endRow; row++) {
		for (size_t col = startCol; col <= endCol; col++) {
			selection.push_back(std::pair<int, int>(row, col));
		}
	}
}

ImVec4 ImTexturePlotter::SelectionBounds() {
	int startRow = height, startCol = width, endRow = 0, endCol = 0;
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			bool isSel = IsCellSelected(y, x);
			if (isSel) {
				if (y < startRow) startRow = y;
				if (x < startCol) startCol = x;
				if (y > endRow) endRow = y;
				if (x > endCol) endCol = x;
			}
		}
	}
	return ImVec4(startRow,startCol,endRow,endCol);
}

bool ImTexturePlotter::SaveTexturePlotter(bool toFile)
{
	if (!selFacet) return false;

	// find selection bounds
	int startRow=height, startCol=width, endRow=0, endCol=0;
	if (selection.size() == 0) { // no selection, export all
		startRow = 0;
		startCol = 0;
		endRow = height-1;
		endCol = width-1;
	}
	else {
		ImVec4 bounds = SelectionBounds();
		startRow = bounds.x;
		startCol = bounds.y;
		endRow = bounds.z;
		endCol = bounds.w;
	}
	// wrtie to file
	if (toFile) {
		std::string fileFilters = "txt,csv";
		std::string fn = NFD_SaveFile_Cpp(fileFilters, "");
		if (!fn.empty()) {
			FILE* f = fopen(fn.c_str(), "w");
			if (f == NULL) {
				ImIOWrappers::InfoPopup("Error", "Cannot open file\nFile: " + fn);
				return false;
			}
			std::string out = Serialize({ startRow,startCol,endRow,endCol });
			if (fn.find(".csv") != std::string::npos) {
				size_t found = out.find('\t');
				while (found != std::string::npos) {
					out.replace(found, 1, ",");
					found = out.find('\t', found + 1);
				}
			}
			fprintf(f, out.c_str());
			fclose(f);
		}
	}
	else {
		SDL_SetClipboardText(Serialize({ startRow,startCol,endRow,endCol }).c_str());
	}
	return true;
}

void ImTexturePlotter::DrawMenuBar()
{
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Export")) {
			if (ImGui::MenuItem("To clipboard")) SaveTexturePlotter(false);
			if (ImGui::MenuItem("To file")) SaveTexturePlotter();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ImGui::Checkbox("Resizable columns", &resizableColumns);
			ImGui::Checkbox("Fit to window", &fitToWindow);
			if (ImGui::MenuItem("Autosize to data")) {
				resizableColumns = false;
				fitToWindow = false;
			}
			if (ImGui::MenuItem("Autosize to window")) {
				resizableColumns = false;
				fitToWindow = true;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Data")) {
			if (data.size() > maxX && data[maxX].size() > maxY) {
				ImGui::Text(fmt::format("Highest value is {} in row {}, col {}", data[maxX][maxY], maxY+1, maxX+1));
			}
			else {
				ImGui::Text("No Data");
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

// pass bounds as {startRow, startCol, endRow, endCol}
std::string ImTexturePlotter::Serialize(SelRect bounds, char lineBreak, std::string rowBreak)
{
	std::string out;
	for (size_t row = bounds.startRow; row <= bounds.endRow; row++) {
		for (size_t col = bounds.startCol; col <= bounds.endCol; col++) {
			out.append(data[row][col]);
			out.append(rowBreak);
		}
		out[out.size() - 1] = lineBreak;
	}
	out.erase(out.end() - 1);
	return out;
}
