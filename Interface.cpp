#include "Interface.h"
#include <direct.h> //_getcwd()

#include "GLApp/GLFileBox.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLInputBox.h"

extern Worker worker;
extern std::vector<string> formulaPrefixes;
extern const char* appName;

extern const char *fileLFilters;
extern const char *fileInsFilters;
extern const char *fileSFilters;
extern const char *fileDesFilters;

extern int   cWidth[];
extern char *cName[];

Interface::Interface() {
	//Get number of cores
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	numCPU = sysinfo.dwNumberOfProcessors;
}

void Interface::UpdateViewerFlags() {
	viewer[curViewer]->showNormal = showNormal->GetState();
	viewer[curViewer]->showRule = showRule->GetState();
	viewer[curViewer]->showUV = showUV->GetState();
	viewer[curViewer]->showLeak = showLeak->GetState();
	viewer[curViewer]->showHit = showHit->GetState();
	viewer[curViewer]->showLine = showLine->GetState();
	viewer[curViewer]->showVolume = showVolume->GetState();
	viewer[curViewer]->showTexture = showTexture->GetState();
	BOOL neededTexture = needsTexture;
	CheckNeedsTexture();

	if (!needsTexture && neededTexture) { //We just disabled mesh
		worker.GetGeometry()->ClearFacetTextures();
	}
	else if (needsTexture && !neededTexture) { //We just enabled mesh
		worker.RebuildTextures();
	}
	viewer[curViewer]->showFilter = showFilter->GetState();
	viewer[curViewer]->showVertex = showVertex->GetState();
	viewer[curViewer]->showIndex = showIndex->GetState();
	//worker.Update(0.0);
}

void Interface::ResetSimulation(BOOL askConfirm) {

	BOOL ok = TRUE;
	if (askConfirm)
		ok = GLMessageBox::Display("Reset simulation ?", "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK;

	if (ok) {
		worker.ResetStatsAndHits(m_fTime);
		nbDesStart = 0;
		nbHitStart = 0;
	}
}

void Interface::UpdateStructMenu() {

	char tmp[128];
	Geometry *geom = worker.GetGeometry();

	structMenu->Clear();
	structMenu->Add("New structure...", MENU_VIEW_NEWSTRUCT);
	structMenu->Add("Delete structure...", MENU_VIEW_DELSTRUCT);
	structMenu->Add(NULL); //Separator
	structMenu->Add("Show all", MENU_VIEW_STRUCTURE, SDLK_F1, CTRL_MODIFIER);
	structMenu->Add("Show previous", MENU_VIEW_PREVSTRUCT, SDLK_F11, CTRL_MODIFIER);
	structMenu->Add("Show next", MENU_VIEW_NEXTSTRUCT, SDLK_F12, CTRL_MODIFIER);
	structMenu->Add(NULL); //Separator

	for (int i = 0; i < geom->GetNbStructure(); i++) {
		sprintf(tmp, "Show #%d (%s)", i + 1, geom->GetStructureName(i));
		if (i < 10)
			structMenu->Add(tmp, MENU_VIEW_STRUCTURE + (i + 1), SDLK_F1 + i + 1, CTRL_MODIFIER);
		else
			structMenu->Add(tmp, MENU_VIEW_STRUCTURE + (i + 1));
	}

	structMenu->SetState(MENU_VIEW_STRUCTURE + geom->viewStruct + 1, TRUE);

	UpdateTitle();
}

void Interface::UpdateCurrentDir(char *fileName) {

	strncpy(currentDir, fileName, 1024);
	char *dp = strrchr(currentDir, '\\');
	if (!dp) dp = strrchr(currentDir, '/');
	if (dp) *dp = 0;

}

void Interface::UpdateCurrentSelDir(char *fileName) {

	strncpy(currentDir, fileName, 1024);
	char *dp = strrchr(currentDir, '\\');
	if (!dp) dp = strrchr(currentDir, '/');
	if (dp) *dp = 0;

}

void Interface::UpdateTitle() {

	static char title[128];

	Geometry *geom = worker.GetGeometry();

	if (!geom->IsLoaded()) {
		sprintf(title, "%s", appName);
	}
	else {
		if (geom->viewStruct < 0) {
			sprintf(title, "%s [%s]", appName, worker.GetShortFileName());
		}
		else {
			sprintf(title, "%s [%s: Struct #%d %s]", appName, worker.GetShortFileName(), geom->viewStruct + 1, geom->GetStructureName(geom->viewStruct));
		}
	}

	SetTitle(title);

}

//-----------------------------------------------------------------------------
// Name: FormatInt()
// Desc: Format an integer in K,M,G,..
//-----------------------------------------------------------------------------
char* Interface::FormatInt(llong v, char *unit)
{

	double x = (double)v;

	static char ret[64];
	if (x < 1E3) {
		sprintf(ret, "%g %s", (double)x, unit);
	}
	else if (x < 1E6) {
		sprintf(ret, "%.1f K%s", x / 1E3, unit);
	}
	else if (x < 1E9) {
		sprintf(ret, "%.2f M%s", x / 1E6, unit);
	}
	else if (x < 1E12) {
		sprintf(ret, "%.2f G%s", x / 1E9, unit);
	}
	else {
		sprintf(ret, "%.2f T%s", x / 1E12, unit);
	}

	return ret;

}

//-----------------------------------------------------------------------------
// Name: FormatPS()
// Desc: Format a double in K,M,G,.. per sec
//-----------------------------------------------------------------------------
char *Interface::FormatPS(double v, char *unit)
{

	static char ret[64];
	if (v < 1000.0) {
		sprintf(ret, "%.1f %s/s", v, unit);
	}
	else if (v < 1000000.0) {
		sprintf(ret, "%.1f K%s/s", v / 1000.0, unit);
	}
	else if (v < 1000000000.0) {
		sprintf(ret, "%.1f M%s/s", v / 1000000.0, unit);
	}
	else {
		sprintf(ret, "%.1f G%s/s", v / 1000000000.0, unit);
	}

	return ret;

}

//-----------------------------------------------------------------------------
// Name: FormatSize()
// Desc: Format a double in K,M,G,.. per sec
//-----------------------------------------------------------------------------
char *Interface::FormatSize(DWORD size)
{

	static char ret[64];
	if (size < 1024UL) {
		sprintf(ret, "%d Bytes", size);
	}
	else if (size < 1048576UL) {
		sprintf(ret, "%.1f KB", (double)size / 1024.0);
	}
	else if (size < 1073741824UL) {
		sprintf(ret, "%.1f MB", (double)size / 1048576.0);
	}
	else {
		sprintf(ret, "%.1f GB", (double)size / 1073741824.0);
	}

	return ret;

}

//-----------------------------------------------------------------------------
// Name: FormatTime()
// Desc: Format time in HH:MM:SS
//-----------------------------------------------------------------------------
char* Interface::FormatTime(float t) {
	static char ret[64];
	int nbSec = (int)(t + 0.5f);
	sprintf(ret, "%02d:%02d:%02d", nbSec / 3600, (nbSec % 3600) / 60, nbSec % 60);
	return ret;
}

void Interface::LoadSelection(char *fName) {

	char fullName[1024];
	strcpy(fullName, "");
	FileReader *f = NULL;

	if (fName == NULL) {
		FILENAME *fn = GLFileBox::OpenFile(currentSelDir, NULL, "Load Selection", fileSelFilters, 0);
		if (fn)
			strcpy(fullName, fn->fullName);
	}
	else {
		strcpy(fullName, fName);
	}

	if (strlen(fullName) == 0) return;

	try {

		Geometry *geom = worker.GetGeometry();
		geom->UnselectAll();
		int nbFacet = geom->GetNbFacet();

		f = new FileReader(fullName);
		while (!f->IsEof()) {
			int s = f->ReadInt();
			if (s >= 0 && s < nbFacet) geom->Select(s);
		}
		geom->UpdateSelection();

		UpdateFacetParams(TRUE);
		UpdateCurrentSelDir(fullName);

	}
	catch (Error &e) {

		char errMsg[512];
		sprintf(errMsg, "%s\nFile:%s", e.GetMsg(), fullName);
		GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);

	}

	SAFE_DELETE(f);
	changedSinceSave = FALSE;

}

void Interface::SaveSelection() {

	FileWriter *f = NULL;
	Geometry *geom = worker.GetGeometry();
	if (geom->GetNbSelected() == 0) return;
	GLProgress *progressDlg2 = new GLProgress("Saving file", "Please wait");
	progressDlg2->SetProgress(0.5);
	progressDlg2->SetVisible(TRUE);
	//GLWindowManager::Repaint();

	FILENAME *fn = GLFileBox::SaveFile(currentSelDir, worker.GetShortFileName(), "Save selection", fileSelFilters, 0);

	if (fn) {

		try {


			char *ext = fn->fullName + strlen(fn->fullName) - 4;

			if (!(*ext == '.')) {
				sprintf(fn->fullName, "%s.sel", fn->fullName); //set to default SEL format
				ext = strrchr(fn->fullName, '.');
			}
			ext++;

			f = new FileWriter(fn->fullName);
			int nbSelected = geom->GetNbSelected();
			int nbFacet = geom->GetNbFacet();
			for (int i = 0; i < nbFacet; i++) {
				if (geom->GetFacet(i)->selected) f->WriteInt(i, "\n");
			}

		}
		catch (Error &e) {
			char errMsg[512];
			sprintf(errMsg, "%s\nFile:%s", e.GetMsg(), fn->fullName);
			GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
		}

		SAFE_DELETE(f);

	}
	progressDlg2->SetVisible(FALSE);
	SAFE_DELETE(progressDlg2);
	changedSinceSave = FALSE;
}

void Interface::ExportSelection() {

	Geometry *geom = worker.GetGeometry();
	if (geom->GetNbSelected() == 0) {
		GLMessageBox::Display("Empty selection", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}

	FILENAME *fn = GLFileBox::SaveFile(currentDir, worker.GetShortFileName(), "Export selection", fileSFilters, 0);
	GLProgress *progressDlg2 = new GLProgress("Saving file...", "Please wait");
	progressDlg2->SetProgress(0.0);
	progressDlg2->SetVisible(TRUE);
	//GLWindowManager::Repaint();
	if (fn) {

		try {
			worker.SaveGeometry(fn->fullName, progressDlg2, TRUE, TRUE);
			AddRecent(fn->fullName);
			//UpdateCurrentDir(fn->fullName);
			//UpdateTitle();
		}
		catch (Error &e) {
			char errMsg[512];
			sprintf(errMsg, "%s\nFile:%s", e.GetMsg(), fn->fullName);
			GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
		}

	}

	progressDlg2->SetVisible(FALSE);
	SAFE_DELETE(progressDlg2);
}

//-----------------------------------------------------------------------------
// Name: UpdateModelParams()
// Desc: Update displayed model parameter on geometry ghange
//-----------------------------------------------------------------------------
void Interface::UpdateModelParams() {

	Geometry *geom = worker.GetGeometry();
	char tmp[256];
	double sumArea = 0;
	facetList->SetSize(4, geom->GetNbFacet(), FALSE, TRUE);
	facetList->SetColumnWidths((int*)cWidth);
	facetList->SetColumnLabels((char **)cName);

	UpdateFacetHits(TRUE);
	AABB bb = geom->GetBB();

	for (int i = 0; i < geom->GetNbFacet(); i++) {
		Facet *f = geom->GetFacet(i);
		if (f->sh.area>0) sumArea += f->sh.area*(f->sh.is2sided ? 2.0 : 1.0);
	}

	sprintf(tmp, "V:%d F:%d Dim:(%g,%g,%g) Area:%g", geom->GetNbVertex(), geom->GetNbFacet(),
		(bb.max.x - bb.min.x), (bb.max.y - bb.min.y), (bb.max.z - bb.min.z), sumArea);
	geomNumber->SetText(tmp);

}

//-----------------------------------------------------------------------------
// Name: AddFormula()
// Desc: Add a formula
//-----------------------------------------------------------------------------
void Interface::AddFormula(GLParser *f, BOOL doUpdate) {

	if (f) {
		if (nbFormula < MAX_FORMULA) {
			formulas[nbFormula].parser = f;
			std::string formulaName = f->GetName();
			if (formulaName.empty()) formulaName = f->GetExpression();
			formulas[nbFormula].name = new GLLabel(formulaName.c_str());
			Add(formulas[nbFormula].name);
			formulas[nbFormula].value = new GLTextField(0, "");
			formulas[nbFormula].value->SetEditable(FALSE);
			Add(formulas[nbFormula].value);
			formulas[nbFormula].setBtn = new GLButton(0, "...");
			Add(formulas[nbFormula].setBtn);
			nbFormula++;
			PlaceComponents();
			if (doUpdate) UpdateFormula();
		}
		else {
			SAFE_DELETE(f);
		}
	}

}

void Interface::ClearFormula() {

	for (int i = 0; i < nbFormula; i++) {
		wnd->PostDelete(formulas[i].name);
		wnd->PostDelete(formulas[i].value);
		wnd->PostDelete(formulas[i].setBtn);
		formulas[i].name = NULL;
		formulas[i].value = NULL;
		formulas[i].setBtn = NULL;
		SAFE_DELETE(formulas[i].parser);
	}
	nbFormula = 0;
	PlaceComponents();

}

void Interface::AddFormula(const char *fName, const char *formula) {

	GLParser *f = new GLParser();
	f->SetExpression(formula);
	f->SetName(fName);
	f->Parse();
	AddFormula(f, FALSE);

}

void Interface::AnimateViewerChange(int next) {

	double xs1, ys1, xs2, ys2;
	double xe1, ye1, xe2, ye2;
	int sx = m_screenWidth - 205;
	int fWidth = m_screenWidth - 215;
	int fHeight = m_screenHeight - 27;
	int Width2 = fWidth / 2 - 1;
	int Height2 = fHeight / 2 - 1;

	// Reset to layout and make all visible

	for (int i = 0; i < MAX_VIEWER; i++)  viewer[i]->SetVisible(TRUE);
	viewer[0]->SetBounds(3, 3, Width2, Height2);
	viewer[1]->SetBounds(6 + Width2, 3, Width2, Height2);
	viewer[2]->SetBounds(3, 6 + Height2, Width2, Height2);
	viewer[3]->SetBounds(6 + Width2, 6 + Height2, Width2, Height2);


	if (modeSolo) {

		// Go from single to layout
		xs1 = (double)3;
		ys1 = (double)3;
		xs2 = (double)fWidth + xs1;
		ys2 = (double)fHeight + ys1;

		switch (next) {
		case 0:
			xe1 = (double)(3);
			ye1 = (double)(3);
			break;
		case 1:
			xe1 = (double)(5 + Width2);
			ye1 = (double)(3);
			break;
		case 2:
			xe1 = (double)(3);
			ye1 = (double)(5 + Height2);
			break;
		case 3:
			xe1 = (double)(5 + Width2);
			ye1 = (double)(5 + Height2);
			break;
		}

		xe2 = (double)(Width2)+xe1;
		ye2 = (double)(Height2)+ye1;

	}
	else {

		// Go from layout to single
		xe1 = (double)3;
		ye1 = (double)3;
		xe2 = (double)fWidth + xe1;
		ye2 = (double)fHeight + ye1;

		switch (next) {
		case 0:
			xs1 = (double)(3);
			ys1 = (double)(3);
			break;
		case 1:
			xs1 = (double)(5 + Width2);
			ys1 = (double)(3);
			break;
		case 2:
			xs1 = (double)(3);
			ys1 = (double)(5 + Height2);
			break;
		case 3:
			xs1 = (double)(5 + Width2);
			ys1 = (double)(5 + Height2);
			break;
		}

		xs2 = (double)(Width2)+xs1;
		ys2 = (double)(Height2)+ys1;

	}

	double t0 = (double)SDL_GetTicks() / 1000.0;
	double t1 = t0;
	double T = 0.15;

	while ((t1 - t0) < T) {
		double t = (t1 - t0) / T;
		int x1 = (int)(xs1 + t*(xe1 - xs1) + 0.5);
		int y1 = (int)(ys1 + t*(ye1 - ys1) + 0.5);
		int x2 = (int)(xs2 + t*(xe2 - xs2) + 0.5);
		int y2 = (int)(ys2 + t*(ye2 - ys2) + 0.5);
		viewer[next]->SetBounds(x1, y1, x2 - x1, y2 - y1);
		wnd->Paint();
		// Overides moving component
		viewer[next]->Paint();
		// Paint modeless
		int n;
		n = GLWindowManager::GetNbWindow();
		GLWindowManager::RepaintRange(1, n);
		t1 = (double)SDL_GetTicks() / 1000.0;
	}

	modeSolo = !modeSolo;
	SelectViewer(next);

}

void Interface::UpdateViewerParams() {

	showNormal->SetState(viewer[curViewer]->showNormal);
	showRule->SetState(viewer[curViewer]->showRule);
	showUV->SetState(viewer[curViewer]->showUV);
	showLeak->SetState(viewer[curViewer]->showLeak);
	showHit->SetState(viewer[curViewer]->showHit);
	showVolume->SetState(viewer[curViewer]->showVolume);
	showLine->SetState(viewer[curViewer]->showLine);
	showTexture->SetState(viewer[curViewer]->showTexture);
	showFilter->SetState(viewer[curViewer]->showFilter);
	showVertex->SetState(viewer[curViewer]->showVertex);
	showIndex->SetState(viewer[curViewer]->showIndex);

	// Force all views to have the same showColormap
	viewer[1]->showColormap = viewer[0]->showColormap;
	viewer[2]->showColormap = viewer[0]->showColormap;
	viewer[3]->showColormap = viewer[0]->showColormap;
	worker.GetGeometry()->texColormap = viewer[0]->showColormap;

}

void Interface::SelectViewer(int s) {

	curViewer = s;
	for (int i = 0; i < MAX_VIEWER; i++) viewer[i]->SetSelected(i == curViewer);
	UpdateViewerParams();

}

void Interface::Place3DViewer() {

	int sx = m_screenWidth - 205;

	// 3D Viewer ----------------------------------------------
	int fWidth = m_screenWidth - 215;
	int fHeight = m_screenHeight - 27;
	int Width2 = fWidth / 2 - 1;
	int Height2 = fHeight / 2 - 1;

	if (modeSolo) {
		for (int i = 0; i < MAX_VIEWER; i++)
			viewer[i]->SetVisible(FALSE);
		viewer[curViewer]->SetBounds(3, 3, fWidth, fHeight);
		viewer[curViewer]->SetVisible(TRUE);
	}
	else {
		for (int i = 0; i < MAX_VIEWER; i++)
			viewer[i]->SetVisible(TRUE);
		viewer[0]->SetBounds(3, 3, Width2, Height2);
		viewer[1]->SetBounds(6 + Width2, 3, Width2, Height2);
		viewer[2]->SetBounds(3, 6 + Height2, Width2, Height2);
		viewer[3]->SetBounds(6 + Width2, 6 + Height2, Width2, Height2);
	}

}

void Interface::UpdateViewers() {
	for (int i = 0; i < MAX_VIEWER; i++)
		viewer[i]->UpdateMatrix();
}

void Interface::SetFacetSearchPrg(BOOL visible, char *text) {
	for (int i = 0; i < MAX_VIEWER; i++) {
		viewer[i]->facetSearchState->SetVisible(visible);
		viewer[i]->facetSearchState->SetText(text);
	}
	GLWindowManager::Repaint();
}

int Interface::OnExit() {
	SaveConfig();
	worker.Exit();
	remove(autosaveFilename.c_str());
	//empty TMP directory
	char tmp[1024];
	char CWD[MAX_PATH];
	_getcwd(CWD, MAX_PATH);
	sprintf(tmp, "del /Q \"%s\\tmp\\*.*\"", CWD);
	system(tmp);
	return GL_OK;
}



int Interface::OneTimeSceneInit_shared() {
	return GL_OK;
}

int Interface::RestoreDeviceObjects_shared() {
	Geometry *geom = worker.GetGeometry();
	geom->RestoreDeviceObjects();
	//worker.Update(0.0f);

	// Restore dialog which are not displayed
	// Those which are displayed are invalidated by the window manager
	RVALIDATE_DLG(formulaSettings);
	RVALIDATE_DLG(collapseSettings);
	RVALIDATE_DLG(moveVertex);
	RVALIDATE_DLG(scaleVertex);
	RVALIDATE_DLG(scaleFacet);
	RVALIDATE_DLG(selectDialog);
	RVALIDATE_DLG(moveFacet);
	RVALIDATE_DLG(extrudeFacet);
	RVALIDATE_DLG(mirrorFacet);
	RVALIDATE_DLG(splitFacet);
	RVALIDATE_DLG(buildIntersection);
	RVALIDATE_DLG(rotateFacet);
	RVALIDATE_DLG(alignFacet);
	RVALIDATE_DLG(addVertex);
	RVALIDATE_DLG(loadStatus);
	RVALIDATE_DLG(facetCoordinates);
	RVALIDATE_DLG(vertexCoordinates);

	UpdateTitle();

	return GL_OK;
}

int Interface::InvalidateDeviceObjects_shared() {
	Geometry *geom = worker.GetGeometry();
	geom->InvalidateDeviceObjects();
	//worker.Update(0.0f);

	// Restore dialog which are not displayed
	// Those which are displayed are invalidated by the window manager
	IVALIDATE_DLG(formulaSettings);
	IVALIDATE_DLG(collapseSettings);
	IVALIDATE_DLG(moveVertex);
	IVALIDATE_DLG(scaleVertex);
	IVALIDATE_DLG(scaleFacet);
	IVALIDATE_DLG(selectDialog);
	IVALIDATE_DLG(moveFacet);
	IVALIDATE_DLG(extrudeFacet);
	IVALIDATE_DLG(mirrorFacet);
	IVALIDATE_DLG(splitFacet);
	IVALIDATE_DLG(buildIntersection);
	IVALIDATE_DLG(rotateFacet);
	IVALIDATE_DLG(alignFacet);
	IVALIDATE_DLG(addVertex);
	IVALIDATE_DLG(loadStatus);
	IVALIDATE_DLG(facetCoordinates);
	IVALIDATE_DLG(vertexCoordinates);

	UpdateTitle();

	return GL_OK;
}

void Interface::ProcessMessage_shared(GLComponent *src, int message) {
	
}

void Interface::CheckNeedsTexture()
{
	needsMesh = needsTexture = FALSE;
	for (int i = 0;i < MAX_VIEWER;i++) {
		needsMesh = needsMesh || (viewer[i]->IsVisible() && viewer[i]->showMesh);
		needsTexture = needsTexture || (viewer[i]->IsVisible() && viewer[i]->showTexture);
	}
}

//SELECTIONS
//-----------------------------------------------------------------------------

void Interface::SelectView(int v) {
	viewer[curViewer]->SetCurrentView(views[v]);
}

//-----------------------------------------------------------------------------

void Interface::SelectSelection(int v) {
	Geometry *geom = worker.GetGeometry();
	geom->SetSelection((&selections[v].selection), &(selections[v].nbSel), viewer[0]->GetWindow()->IsShiftDown(), viewer[0]->GetWindow()->IsCtrlDown());
	idSelection = v;
}

//-----------------------------------------------------------------------------
void Interface::ClearSelectionMenus() {
	memorizeSelectionsMenu->Clear();
	memorizeSelectionsMenu->Add("Add new...", MENU_SELECTION_ADDNEW, SDLK_w, CTRL_MODIFIER);
	memorizeSelectionsMenu->Add(NULL); // Separator
	clearSelectionsMenu->Clear();
	clearSelectionsMenu->Add("Clear All", MENU_SELECTION_CLEARALL);
	clearSelectionsMenu->Add(NULL); // Separator
	selectionsMenu->Clear();
}

void Interface::RebuildSelectionMenus() {
	ClearSelectionMenus();
	int i;
	for (i = 0; i < nbSelection; i++) {
		if (i <= 8) {
			selectionsMenu->Add(selections[i].name, MENU_SELECTION_SELECTIONS + i, SDLK_1 + i, ALT_MODIFIER);
		}
		else {
			selectionsMenu->Add(selections[i].name, MENU_SELECTION_SELECTIONS + i); //no place for ALT+shortcut
		}
		clearSelectionsMenu->Add(selections[i].name, MENU_SELECTION_CLEARSELECTIONS + i);
		memorizeSelectionsMenu->Add(selections[i].name, MENU_SELECTION_MEMORIZESELECTIONS + i);
	}
	selectionsMenu->Add(NULL); //Separator
	selectionsMenu->Add("Select previous", MENU_SELECTION_SELECTIONS + i, SDLK_F11, ALT_MODIFIER);
	selectionsMenu->Add("Select next", MENU_SELECTION_SELECTIONS + i + 1, SDLK_F12, ALT_MODIFIER);
}

void Interface::AddSelection(char *selectionName, ASELECTION s) {

	if (nbSelection < MAX_SELECTION) {
		selections[nbSelection] = s;
		selections[nbSelection].name = _strdup(selectionName);
		nbSelection++;
	}
	else {
		SAFE_FREE(selections[0].name);
		for (int i = 0; i < MAX_SELECTION - 1; i++) selections[i] = selections[i + 1];
		selections[MAX_SELECTION - 1] = s;
		selections[MAX_SELECTION - 1].name = _strdup(selectionName);
	}
	RebuildSelectionMenus();
}

void Interface::ClearSelection(int idClr) {
	SAFE_FREE(selections[idClr].name);
	for (int i = idClr; i < nbSelection - 1; i++) selections[i] = selections[i + 1];
	nbSelection--;
	RebuildSelectionMenus();
}

void Interface::ClearAllSelections() {
	for (int i = 0; i < nbSelection; i++) SAFE_FREE(selections[i].name);
	nbSelection = 0;
	ClearSelectionMenus();
}

void Interface::OverWriteSelection(int idOvr) {
	Geometry *geom = worker.GetGeometry();
	char *selectionName = GLInputBox::GetInput(selections[idOvr].name, "Selection name", "Enter selection name");
	if (!selectionName) return;

	geom->GetSelection(&(selections[idOvr].selection), &(selections[idOvr].nbSel));
	selections[idOvr].name = _strdup(selectionName);
	RebuildSelectionMenus();
}

void Interface::AddSelection() {
	Geometry *geom = worker.GetGeometry();
	char tmp[32];
	sprintf(tmp, "Selection #%d", nbSelection + 1);
	char *selectionName = GLInputBox::GetInput(tmp, "Selection name", "Enter selection name");
	if (!selectionName) return;

	if (nbSelection < MAX_SELECTION) {
		geom->GetSelection(&(selections[nbSelection].selection), &(selections[nbSelection].nbSel));
		selections[nbSelection].name = _strdup(selectionName);
		nbSelection++;
	}
	else {
		SAFE_FREE(selections[0].name);
		for (int i = 0; i < MAX_SELECTION - 1; i++) selections[i] = selections[i + 1];
		geom->GetSelection(&(selections[MAX_SELECTION - 1].selection), &(selections[MAX_SELECTION - 1].nbSel));
		selections[MAX_SELECTION - 1].name = _strdup(selectionName);
	}
	RebuildSelectionMenus();
}

//VIEWS
//-----------------------------------------------------------------------------
void Interface::ClearViewMenus() {
	memorizeViewsMenu->Clear();
	memorizeViewsMenu->Add("Add new...", MENU_VIEW_ADDNEW, SDLK_q, CTRL_MODIFIER);
	memorizeViewsMenu->Add(NULL); // Separator
	clearViewsMenu->Clear();
	clearViewsMenu->Add("Clear All", MENU_VIEW_CLEARALL);
	clearViewsMenu->Add(NULL); // Separator
	viewsMenu->Clear();
}

void Interface::RebuildViewMenus() {
	ClearViewMenus();
	for (int i = 0; i < nbView; i++) {
		int id = i;
		if (nbView >= 10) id = i - nbView + 8;
		if (id >= 0 && id <= 8) {
			viewsMenu->Add(views[i].name, MENU_VIEW_VIEWS + i, SDLK_F1 + id, ALT_MODIFIER);
		}
		else {
			viewsMenu->Add(views[i].name, MENU_VIEW_VIEWS + i);
		}
		clearViewsMenu->Add(views[i].name, MENU_VIEW_CLEARVIEWS + i);
		memorizeViewsMenu->Add(views[i].name, MENU_VIEW_MEMORIZEVIEWS + i);
	}
}


void Interface::AddView(char *viewName, AVIEW v) {

	if (nbView < MAX_VIEW) {
		views[nbView] = v;
		views[nbView].name = _strdup(viewName);
		nbView++;
	}
	else {
		SAFE_FREE(views[0].name);
		for (int i = 0; i < MAX_VIEW - 1; i++) views[i] = views[i + 1];
		views[MAX_VIEW - 1] = v;
		views[MAX_VIEW - 1].name = _strdup(viewName);
	}
	RebuildViewMenus();
}

void Interface::ClearView(int idClr) {
	SAFE_FREE(views[idClr].name);
	for (int i = idClr; i < nbView - 1; i++) views[i] = views[i + 1];
	nbView--;
	RebuildViewMenus();
}

void Interface::ClearAllViews() {
	for (int i = 0; i < nbView; i++) SAFE_FREE(views[i].name);
	nbView = 0;
	ClearViewMenus();
}

void Interface::OverWriteView(int idOvr) {
	Geometry *geom = worker.GetGeometry();
	char *viewName = GLInputBox::GetInput(views[idOvr].name, "View name", "Enter view name");
	if (!viewName) return;

	views[idOvr] = viewer[curViewer]->GetCurrentView();
	views[idOvr].name = _strdup(viewName);
	RebuildViewMenus();
}

void Interface::AddView() {
	Geometry *geom = worker.GetGeometry();
	char tmp[32];
	sprintf(tmp, "View #%d", nbView + 1);
	char *viewName = GLInputBox::GetInput(tmp, "View name", "Enter view name");
	if (!viewName) return;

	if (nbView < MAX_VIEW) {
		views[nbView] = viewer[curViewer]->GetCurrentView();
		views[nbView].name = _strdup(viewName);
		nbView++;
	}
	else {
		SAFE_FREE(views[0].name);
		for (int i = 0; i < MAX_VIEW - 1; i++) views[i] = views[i + 1];
		views[MAX_VIEW - 1] = viewer[curViewer]->GetCurrentView();
		views[MAX_VIEW - 1].name = _strdup(viewName);
	}
	RebuildViewMenus();
}

//-----------------------------------------------------------------------------

void Interface::RemoveRecent(char *fileName) {

	if (!fileName) return;

	BOOL found = FALSE;
	int i = 0;
	while (!found && i < nbRecent) {
		found = strcmp(fileName, recents[i]) == 0;
		if (!found) i++;
	}
	if (!found) return;

	SAFE_FREE(recents[i]);
	for (int j = i; j < nbRecent - 1; j++)
		recents[j] = recents[j + 1];
	nbRecent--;

	// Update menu
	GLMenu *m = menu->GetSubMenu("File")->GetSubMenu("Load recent");
	m->Clear();
	for (i = nbRecent - 1; i >= 0; i--)
		m->Add(recents[i], MENU_FILE_LOADRECENT + i);
	SaveConfig();
}

//-----------------------------------------------------------------------------

void Interface::AddRecent(char *fileName) {

	// Check if already exists
	BOOL found = FALSE;
	int i = 0;
	while (!found && i < nbRecent) {
		found = strcmp(fileName, recents[i]) == 0;
		if (!found) i++;
	}
	if (found) {
		for (int j = i; j < nbRecent - 1; j++) {
			recents[j] = recents[j + 1];
		}
		recents[nbRecent - 1] = _strdup(fileName);
		// Update menu
		GLMenu *m = menu->GetSubMenu("File")->GetSubMenu("Load recent");
		m->Clear();
		for (int i = nbRecent - 1; i >= 0; i--)
			m->Add(recents[i], MENU_FILE_LOADRECENT + i);
		SaveConfig();
		return;
	}

	// Add the new recent file
	if (nbRecent < MAX_RECENT) {
		recents[nbRecent] = _strdup(fileName);
		nbRecent++;
	}
	else {
		// Shift
		SAFE_FREE(recents[0]);
		for (int i = 0; i < MAX_RECENT - 1; i++)
			recents[i] = recents[i + 1];
		recents[MAX_RECENT - 1] = _strdup(fileName);
	}

	// Update menu
	GLMenu *m = menu->GetSubMenu("File")->GetSubMenu("Load recent");
	m->Clear();
	for (int i = nbRecent - 1; i >= 0; i--)
		m->Add(recents[i], MENU_FILE_LOADRECENT + i);
	SaveConfig();
}

void Interface::AddStruct() {
	Geometry *geom = worker.GetGeometry();
	char tmp[32];
	sprintf(tmp, "Structure #%d", geom->GetNbStructure() + 1);
	char *structName = GLInputBox::GetInput(tmp, "Structure name", "Enter name of new structure");
	if (!structName) return;
	geom->AddStruct(structName);
	// Send to sub process
	try { worker.Reload(); }
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
}

void Interface::DeleteStruct() {
	Geometry *geom = worker.GetGeometry();
	char *structNum = GLInputBox::GetInput("", "Structure number", "Number of structure to delete:");
	if (!structNum) return;
	int structNumInt;
	if (!sscanf(structNum, "%d", &structNumInt)) {
		GLMessageBox::Display("Invalid structure number");
		return;
	}
	if (structNumInt<1 || structNumInt>geom->GetNbStructure()) {
		GLMessageBox::Display("Invalid structure number");
		return;
	}
	BOOL hasFacets = FALSE;
	for (int i = 0; i < geom->GetNbFacet() && !hasFacets; i++) {
		if (geom->GetFacet(i)->sh.superIdx == (structNumInt - 1)) hasFacets = TRUE;
	}
	if (hasFacets) {
		int rep = GLMessageBox::Display("This structure has facets. They will be deleted with the structure.", "Structure delete", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING);
		if (rep != GLDLG_OK) return;
	}
	if (!AskToReset()) return;
	geom->DelStruct(structNumInt - 1);
	geom->CalcTotalOutGassing();
	// Send to sub process
	try { worker.Reload(); }
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
}

void Interface::DisplayCollapseDialog() {
	Geometry *geom = worker.GetGeometry();
	if (!collapseSettings) collapseSettings = new CollapseSettings();
	collapseSettings->SetGeometry(geom, &worker);
	collapseSettings->SetVisible(TRUE);
}

void Interface::RenumberSelections(const std::vector<int> &newRefs) {
	for (int i = 0; i < nbSelection; i++) {
		BOOL found = FALSE;
		for (int j = 0; j < selections[i].nbSel; j++) {
			if (selections[i].selection[j] >= newRefs.size() || newRefs[selections[i].selection[j]] == -1) { //remove from selection
				for (int k = j; k < (selections[i].nbSel - 1); k++)
					selections[i].selection[k] = selections[i].selection[k + 1];
				selections[i].nbSel--;
				if (selections[i].nbSel == 0) {
					ClearSelection(i); //last facet removed from selection
				}
				j--; //Do again the element as now it's the next
			}
			else { //renumber
				selections[i].selection[j] = newRefs[selections[i].selection[j]];
			}
		}
	}
}

void Interface::RenumberFormulas(std::vector<int> *newRefs) {
	for (int i = 0; i < nbFormula; i++) {
		char expression[1024];
		strcpy(expression, this->formulas[i].parser->GetExpression());
		if (OffsetFormula(expression, NULL, NULL, newRefs)) {
			this->formulas[i].parser->SetExpression(expression);
			this->formulas[i].parser->Parse();
			std::string formulaName = formulas[i].parser->GetName();
			if (formulaName.empty()) formulaName = expression;
			formulas[i].name->SetText(formulaName.c_str());
		}
	}
}

//-----------------------------------------------------------------------------
// Name: ProcessFormulaButtons()
// Desc: Handle forumla button event
//-----------------------------------------------------------------------------
void Interface::ProcessFormulaButtons(GLComponent *src) {

	// Search formula buttons
	BOOL found = FALSE;
	int i = 0;
	while (!found && i < nbFormula) {
		found = (src == formulas[i].setBtn);
		if (!found) i++;
	}
	if (found) {
		if (!formulaSettings) formulaSettings = new FormulaSettings();
		if (formulaSettings->EditFormula(formulas[i].parser)) {
			// Apply change
			std::string formulaName = formulas[i].parser->GetName();
			if (formulaName.empty()) formulaName = formulas[i].parser->GetExpression();
			formulas[i].name->SetText(formulaName.c_str());
			UpdateFormula();
		}
		else {
			// Delete
			wnd->PostDelete(formulas[i].name);
			wnd->PostDelete(formulas[i].value);
			wnd->PostDelete(formulas[i].setBtn);
			formulas[i].name = NULL;
			formulas[i].value = NULL;
			formulas[i].setBtn = NULL;
			SAFE_DELETE(formulas[i].parser);
			for (int j = i; j < nbFormula - 1; j++)
				formulas[j] = formulas[j + 1];
			nbFormula--;
			PlaceComponents();
			UpdateFormula();
		}
	}

}

BOOL Interface::OffsetFormula(char *expression, int offset, int filter, std::vector<int> *newRefs) {
	//will increase or decrease facet numbers in a formula
	//only applies to facet numbers larger than "filter" parameter
	//If *newRefs is not NULL, a vector is passed containing the new references
	BOOL changed = FALSE;

	string expr = expression; //convert char array to string

	size_t pos = 0; //analyzed until this position
	while (pos < expr.size()) { //while not end of expression

		vector<size_t> location; //for each prefix, we store where it was found

		for (int j = 0; j < (int)formulaPrefixes.size(); j++) { //try all expressions
			location.push_back(expr.find(formulaPrefixes[j], pos));
		}
		size_t minPos = string::npos;
		size_t maxLength = 0;
		for (int j = 0; j < (int)formulaPrefixes.size(); j++)  //try all expressions, find first prefix location
			if (location[j] < minPos) minPos = location[j];
		for (int j = 0; j < (int)formulaPrefixes.size(); j++)  //try all expressions, find longest prefix at location
			if (location[j] == minPos && formulaPrefixes[j].size() > maxLength) maxLength = formulaPrefixes[j].size();
		int digitsLength = 0;
		if (minPos != string::npos) { //found expression, let's find tailing facet number digits
			while ((minPos + maxLength + digitsLength) < expr.length() && expr[minPos + maxLength + digitsLength] >= '0' && expr[minPos + maxLength + digitsLength] <= '9')
				digitsLength++;
			if (digitsLength > 0) { //there was a digit after the prefix
				int facetNumber;
				if (sscanf(expr.substr(minPos + maxLength, digitsLength).c_str(), "%d", &facetNumber)) {
					if (newRefs == NULL) { //Offset mode
						if ((facetNumber - 1) > filter) {
							char tmp[10];
							sprintf(tmp, "%d", facetNumber + offset);
							expr.replace(minPos + maxLength, digitsLength, tmp);
							changed = TRUE;
						}
						else if ((facetNumber - 1) == filter) {
							expr.replace(minPos + maxLength, digitsLength, "0");
							changed = TRUE;
						}
					}
					else { //newRefs mode
						if ((facetNumber - 1) >= (*newRefs).size() || (*newRefs)[facetNumber - 1] == -1) { //Facet doesn't exist anymore
							expr.replace(minPos + maxLength, digitsLength, "0");
							changed = TRUE;
						}
						else { //Update facet number
							char tmp[10];
							sprintf(tmp, "%d", (*newRefs)[facetNumber - 1]);
							expr.replace(minPos + maxLength, digitsLength, tmp);
							changed = TRUE;
						}
					}
				}
			}
		}
		if (minPos != string::npos) pos = minPos + maxLength + digitsLength;
		else pos = minPos;
	}
	strcpy(expression, expr.c_str());
	return changed;
}

int Interface::Resize(DWORD width, DWORD height, BOOL forceWindowed) {
	int r = GLApplication::Resize(width, height, forceWindowed);
	PlaceComponents();
	return r;
}