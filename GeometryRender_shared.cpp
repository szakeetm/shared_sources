#include "Geometry_shared.h"
#include "Worker.h"
#include "GLApp/MathTools.h" //Min max
#include "GLApp\GLToolkit.h"
//#include <malloc.h>
#include <string.h>
#include <math.h>
#include "GLApp/GLMatrix.h"
#ifdef MOLFLOW
#include "MolFlow.h"
#include "Interface.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp\GLList.h"
#include "SmartSelection.h"
#include "FacetCoordinates.h"
#include "VertexCoordinates.h"
#include "Facet_shared.h"

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

void Geometry::SelectFacet(size_t facetId) {
	if (!isLoaded) return;
	Facet *f = facets[facetId];
	f->selected = (viewStruct == -1) || (viewStruct == f->sh.superIdx);
	if (!f->selected) f->UnselectElem();
	nbSelectedHist = 0;
	AddToSelectionHist(facetId);
}

void Geometry::SelectArea(int x1, int y1, int x2, int y2, bool clear, bool unselect, bool vertexBound, bool circularSelection) {

	// Select a set of facet according to a 2D bounding rectangle
	// (x1,y1) and (x2,y2) are in viewport coordinates

	float rx, ry, rz, rw, r2;
	int _x1, _y1, _x2, _y2;

	_x1 = Min(x1, x2);
	_x2 = Max(x1, x2);
	_y1 = Min(y1, y2);
	_y2 = Max(y1, y2);

	if (circularSelection) {
		r2 = pow((float)(x1 - x2), 2) + pow((float)(y1 - y2), 2);
	}

	GLfloat mProj[16];
	GLfloat mView[16];
	GLVIEWPORT g;

	glGetFloatv(GL_PROJECTION_MATRIX, mProj);
	glGetFloatv(GL_MODELVIEW_MATRIX, mView);
	glGetIntegerv(GL_VIEWPORT, (GLint *)&g);

	GLMatrix proj; proj.LoadGL(mProj);
	GLMatrix view; view.LoadGL(mView);
	GLMatrix m; m.Multiply(&proj, &view);

	if (clear && !unselect) UnselectAll();
	nbSelectedHist = 0;
	int lastPaintedProgress = -1;
	char tmp[256];
	int paintStep = (int)((double)sh.nbFacet / 10.0);

	for (int i = 0; i < sh.nbFacet; i++) {
		if (sh.nbFacet > 5000) {
			if ((i - lastPaintedProgress) > paintStep) {
				lastPaintedProgress = i;;
				sprintf(tmp, "Facet search: %d%%", (int)(i*100.0 / (double)sh.nbFacet));
				mApp->SetFacetSearchPrg(true, tmp);
			}
		}
		Facet *f = facets[i];
		if (viewStruct == -1 || f->sh.superIdx == viewStruct) {

			size_t nb = facets[i]->sh.nbIndex;
			bool isInside = true;
			size_t j = 0;
			bool hasSelectedVertex = false;
			while (j < nb && isInside) {

				size_t idx = f->indices[j];
				m.TransfomVec((float)vertices3[idx].x, (float)vertices3[idx].y, (float)vertices3[idx].z, 1.0f,
					&rx, &ry, &rz, &rw);

				if (rw > 0.0f) {
					int xe = (int)(((rx / rw) + 1.0f) * (float)g.width / 2.0f);
					int ye = (int)(((-ry / rw) + 1.0f) * (float)g.height / 2.0f);
					if (!circularSelection)
						isInside = (xe >= _x1) && (xe <= _x2) && (ye >= _y1) && (ye <= _y2);
					else //circular selection
						isInside = (pow((float)(xe - x1), 2) + pow((float)(ye - y1), 2)) <= r2;
					if (vertices3[idx].selected) hasSelectedVertex = true;
				}
				else {

					isInside = false;
				}
				j++;

			}

			if (isInside && (!vertexBound || hasSelectedVertex)) {
				if (!unselect) {
					f->selected = !unselect;
				}
				else {

					f->selected = !unselect;
				}
			}

		}
	}
	mApp->SetFacetSearchPrg(false, NULL);
	UpdateSelection();
}

void Geometry::Select(int x, int y, bool clear, bool unselect, bool vertexBound, int width, int height) {

	int i;
	if (!isLoaded) return;

	// Select a facet on a mouse click in 3D perspectivce view 
	// (x,y) are in screen coordinates
	// TODO: Handle clipped polygon

	// Check intersection of the facet and a "perspective ray"
	int *allXe = (int *)malloc(sh.nbVertex * sizeof(int));
	int *allYe = (int *)malloc(sh.nbVertex * sizeof(int));

	// Transform points to screen coordinates
	bool *ok = (bool *)malloc(sh.nbVertex * sizeof(bool));
	bool *onScreen = (bool *)malloc(sh.nbVertex * sizeof(bool));
	for (i = 0; i < sh.nbVertex; i++) {//here we could speed up by choosing visible vertices only?
		ok[i] = GLToolkit::Get2DScreenCoord((float)vertices3[i].x, (float)vertices3[i].y, (float)vertices3[i].z, allXe + i, allYe + i);
		onScreen[i] = (ok[i] && (*(allXe + i) >= 0) && (*(allYe + i) >= 0) && (*(allXe + i) <= width) && (*(allYe + i) <= height));
	}

	// Check facets
	bool found = false;
	bool clipped;
	bool hasVertexOnScreen;
	bool hasSelectedVertex;
	i = 0;
	char tmp[256];
	int lastFound = -1;
	int lastPaintedProgress = -1;
	int paintStep = (int)((double)sh.nbFacet / 10.0);

	while (i < sh.nbFacet && !found) {
		if (sh.nbFacet > 5000) {
			if ((i - lastPaintedProgress) > paintStep) {
				lastPaintedProgress = i;;
				sprintf(tmp, "Facet search: %d%%", (int)(i*100.0 / (double)sh.nbFacet));
				mApp->SetFacetSearchPrg(true, tmp);
			}
		}
		if (viewStruct == -1 || facets[i]->sh.superIdx == viewStruct) {

			clipped = false;
			hasVertexOnScreen = false;
			hasSelectedVertex = false;
			size_t nb = facets[i]->sh.nbIndex;
			// Build array of 2D points
			int *xe = (int *)malloc(nb * sizeof(int));
			int *ye = (int *)malloc(nb * sizeof(int));
			for (int j = 0; j < nb && !clipped; j++) {
				size_t idx = facets[i]->indices[j];
				if (ok[idx]) {
					xe[j] = allXe[idx];
					ye[j] = allYe[idx];
					if (onScreen[idx]) hasVertexOnScreen = true;
				}
				else {

					clipped = true;
				}
			}
			if (vertexBound) { //CAPS LOCK on, select facets onyl with at least one seleted vertex
				for (size_t j = 0; j < nb && (!hasSelectedVertex); j++) {
					size_t idx = facets[i]->indices[j];
					if (vertices3[idx].selected) hasSelectedVertex = true;
				}
			}

			if (!clipped && hasVertexOnScreen && (!vertexBound || hasSelectedVertex)) {

				found = GLToolkit::IsInsidePoly(x, y, xe, ye, nb);
				free(xe);
				free(ye);

				if (found) {
					if (unselect) {
						if (!mApp->smartSelection || !mApp->smartSelection->IsSmartSelection()) {
							facets[i]->selected = false;
							found = false; //Continue looking for facets
						}
						else { //Smart selection
							double maxAngleDiff = mApp->smartSelection->GetMaxAngle();
							std::vector<size_t> connectedFacets;
							mApp->SetFacetSearchPrg(true, "Smart selecting...");
							if (maxAngleDiff >= 0.0) connectedFacets = GetConnectedFacets(i, maxAngleDiff);
							for (auto ind : connectedFacets)
								facets[ind]->selected = false;
							mApp->SetFacetSearchPrg(false, "");
						}
					} //end unselect

					if (AlreadySelected(i)) {

						lastFound = i;
						found = false; //Continue looking for facets

					}
				} //end found

			}

		}

		if (!found) i++;

	}
	mApp->SetFacetSearchPrg(false, "");
	if (clear && !unselect) UnselectAll();

	if (!found && lastFound >= 0) {
		if (!unselect) {
			// Restart
			nbSelectedHist = 0;
			AddToSelectionHist(lastFound);
		}
		facets[lastFound]->selected = !unselect;
		if (!unselect) mApp->facetList->ScrollToVisible(lastFound, 0, true); //scroll to selected facet
	}
	else {

		if (found) {
			if (!unselect) AddToSelectionHist(i);
			if (!mApp->smartSelection || !mApp->smartSelection->IsSmartSelection()) {
				facets[i]->selected = !unselect;
			}
			else { //Smart selection
				double maxAngleDiff = mApp->smartSelection->GetMaxAngle();
				std::vector<size_t> connectedFacets;
				mApp->SetFacetSearchPrg(true, "Smart selecting...");
				if (maxAngleDiff >= 0.0) connectedFacets = GetConnectedFacets(i, maxAngleDiff);
				for (auto ind : connectedFacets)
					facets[ind]->selected = !unselect;
				mApp->SetFacetSearchPrg(false, "");
			}
			if (!unselect) mApp->facetList->ScrollToVisible(i, 0, true); //scroll to selected facet
		}
		else {

			nbSelectedHist = 0;
		}
	}

	free(allXe);
	free(allYe);
	free(ok);
	free(onScreen);
	UpdateSelection();

}

void Geometry::SelectVertex(int vertexId) {
	//isVertexSelected[vertexId] = (viewStruct==-1) || (viewStruct==f->sh.superIdx);
	//here we should look through facets if vertex is member of any
	//if( !f->selected ) f->UnselectElem();
	if (!isLoaded) return;
	vertices3[vertexId].selected = true;
}

void Geometry::SelectVertex(int x1, int y1, int x2, int y2, bool shiftDown, bool ctrlDown, bool circularSelection, bool facetBound) {

	// Select a set of vertices according to a 2D bounding rectangle
	// (x1,y1) and (x2,y2) are in viewport coordinates

	float rx, ry, rz, rw, r2;
	int _x1, _y1, _x2, _y2;

	_x1 = Min(x1, x2);
	_x2 = Max(x1, x2);
	_y1 = Min(y1, y2);
	_y2 = Max(y1, y2);

	if (circularSelection) {
		r2 = pow((float)(x1 - x2), 2) + pow((float)(y1 - y2), 2);
	}

	GLfloat mProj[16];
	GLfloat mView[16];
	GLVIEWPORT g;

	glGetFloatv(GL_PROJECTION_MATRIX, mProj);
	glGetFloatv(GL_MODELVIEW_MATRIX, mView);
	glGetIntegerv(GL_VIEWPORT, (GLint *)&g);

	GLMatrix proj; proj.LoadGL(mProj);
	GLMatrix view; view.LoadGL(mView);
	GLMatrix m; m.Multiply(&proj, &view);

	if (!ctrlDown && !shiftDown) {
		UnselectAllVertex(); EmptySelectedVertexList();
		//nbSelectedHistVertex = 0;
	}

	std::vector<bool> selectedFacetsVertices;
	if (facetBound) selectedFacetsVertices = GetVertexBelongsToSelectedFacet();

	for (int i = 0; i < sh.nbVertex; i++) {
		if (facetBound && !selectedFacetsVertices[i]) continue; //doesn't belong to selected facet
		Vector3d *v = GetVertex(i);
		//if(viewStruct==-1 || f->sh.superIdx==viewStruct) {
		if (true) {

			bool isInside;
			int idx = i;
			m.TransfomVec((float)vertices3[idx].x, (float)vertices3[idx].y, (float)vertices3[idx].z, 1.0f,
				&rx, &ry, &rz, &rw);

			if (rw > 0.0f) {
				int xe = (int)(((rx / rw) + 1.0f) * (float)g.width / 2.0f);
				int ye = (int)(((-ry / rw) + 1.0f) * (float)g.height / 2.0f);
				if (!circularSelection)
					isInside = (xe >= _x1) && (xe <= _x2) && (ye >= _y1) && (ye <= _y2);
				else //circular selection
					isInside = (pow((float)(xe - x1), 2) + pow((float)(ye - y1), 2)) <= r2;
			}
			else {

				isInside = false;
			}

			if (isInside) {
				vertices3[i].selected = !ctrlDown;
				if (ctrlDown) RemoveFromSelectedVertexList(i);
				else {
					AddToSelectedVertexList(i);
					if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateId(i);
				}
			}
		}
	}

	//UpdateSelectionVertex();
	if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
}

void Geometry::SelectVertex(int x, int y, bool shiftDown, bool ctrlDown, bool facetBound) {
	int i;
	if (!isLoaded) return;

	// Select a vertex on a mouse click in 3D perspectivce view 
	// (x,y) are in screen coordinates
	// TODO: Handle clipped polygon

	// Check intersection of the facet and a "perspective ray"
	int *allXe = (int *)malloc(sh.nbVertex * sizeof(int));
	int *allYe = (int *)malloc(sh.nbVertex * sizeof(int));

	std::vector<bool> selectedFacetsVertices;
	if (facetBound) selectedFacetsVertices = GetVertexBelongsToSelectedFacet();

	// Transform points to screen coordinates
	bool *ok = (bool *)malloc(sh.nbVertex * sizeof(bool));
	for (i = 0; i < sh.nbVertex; i++) {
		if (facetBound && !selectedFacetsVertices[i]) continue; //doesn't belong to selected facet
		ok[i] = GLToolkit::Get2DScreenCoord((float)vertices3[i].x, (float)vertices3[i].y, (float)vertices3[i].z, allXe + i, allYe + i);
	}

	//Get Closest Point to click
	double minDist = 9999;
	double distance;
	int minId = -1;
	for (i = 0; i < sh.nbVertex; i++) {
		if (facetBound && !selectedFacetsVertices[i]) continue; //doesn't belong to selected facet
		if (ok[i] && !(allXe[i] < 0) && !(allYe[i] < 0)) { //calculate only for points on screen
			distance = pow((double)(allXe[i] - x), 2) + pow((double)(allYe[i] - y), 2);
			if (distance < minDist) {
				minDist = distance;
				minId = i;
			}
		}
	}

	if (!ctrlDown && !shiftDown) {
		UnselectAllVertex(); EmptySelectedVertexList();
		//nbSelectedHistVertex = 0;
	}

	if (minDist < 250.0) {
		vertices3[minId].selected = !ctrlDown;
		if (ctrlDown) RemoveFromSelectedVertexList(minId);
		else {
			AddToSelectedVertexList(minId);
			if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateId(minId);
			//nbSelectedHistVertex++;
		}
	}

	free(allXe);
	free(allYe);
	free(ok);
	//UpdateSelection();
	if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
}

void Geometry::AddToSelectionHist(size_t f) {

	if (nbSelectedHist < SEL_HISTORY) {
		selectHist[nbSelectedHist] = f;
		nbSelectedHist++;
	}

}

bool Geometry::AlreadySelected(size_t f) {

	// Check if the facet has already been selected
	bool found = false;
	size_t i = 0;
	while (!found && i < nbSelectedHist) {
		found = (selectHist[i] == f);
		if (!found) i++;
	}
	return found;

}

void Geometry::SelectAll() {
	for (int i = 0; i < sh.nbFacet; i++)
		SelectFacet(i);
	UpdateSelection();
}

void Geometry::EmptySelectedVertexList() {
	selectedVertexList_ordered.clear();
}

void Geometry::RemoveFromSelectedVertexList(size_t vertexId) {
	selectedVertexList_ordered.erase(std::remove(selectedVertexList_ordered.begin(), selectedVertexList_ordered.end(), vertexId), selectedVertexList_ordered.end());
}

void Geometry::AddToSelectedVertexList(size_t vertexId) {
	selectedVertexList_ordered.push_back(vertexId);
}

void Geometry::SelectAllVertex() {
	for (int i = 0; i < sh.nbVertex; i++)
		SelectVertex(i);
	//UpdateSelectionVertex();
}

size_t Geometry::GetNbSelectedVertex() {
	size_t nbSelectedVertex = 0;
	for (int i = 0; i < sh.nbVertex; i++) {
		if (vertices3[i].selected) nbSelectedVertex++;
	}
	return nbSelectedVertex;
}

void Geometry::UnselectAll() {
	for (int i = 0; i < sh.nbFacet; i++) {
		facets[i]->selected = false;
		facets[i]->UnselectElem();
	}
	UpdateSelection();
}

void Geometry::UnselectAllVertex() {
	for (int i = 0; i < sh.nbVertex; i++) {
		vertices3[i].selected = false;
		//facets[i]->UnselectElem(); //what is this?
	}
	//UpdateSelectionVertex();
}

std::vector<size_t> Geometry::GetSelectedVertices()
{
	std::vector<size_t> sel;
	for (size_t i = 0; i < sh.nbVertex; i++)
		if (vertices3[i].selected) sel.push_back(i);
	return sel;
}

void Geometry::DrawFacet(Facet *f, bool offset, bool showHidden, bool selOffset) {

	// Render a facet (wireframe)
	size_t nb = f->sh.nbIndex;
	size_t i1;

	if (offset) {

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		if (selOffset) {
			glPolygonOffset(0.0f, 1.0f);
		}
		else {

			glPolygonOffset(0.0f, 5.0f);
		}
		glBegin(GL_POLYGON);
		for (size_t j = 0; j < nb; j++) {
			i1 = f->indices[j];
			glEdgeFlag(f->visible[j] || showHidden);
			glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
		}
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);

	}
	else {

		if (nb < 8) {
			// No hole possible
			glBegin(GL_LINE_LOOP);
			for (size_t j = 0; j < nb; j++) {
				i1 = f->indices[j];
				glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
			}
			glEnd();
		}
		else {

			glBegin(GL_LINES);
			size_t i1, i2, j;
			for (j = 0; j < nb - 1; j++) {
				if (f->visible[j] || showHidden) {
					i1 = f->indices[j];
					i2 = f->indices[j + 1];
					glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
					glVertex3d(vertices3[i2].x, vertices3[i2].y, vertices3[i2].z);
				}
			}
			// Last segment
			if (f->visible[j] || showHidden) {
				i1 = f->indices[j];
				i2 = f->indices[0];
				glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
				glVertex3d(vertices3[i2].x, vertices3[i2].y, vertices3[i2].z);
			}
			glEnd();
		}

	}

}

void Geometry::DrawPolys() {

	size_t *f3 = (size_t *)malloc(sh.nbFacet * sizeof(size_t));
	size_t *f4 = (size_t *)malloc(sh.nbFacet * sizeof(size_t));
	size_t *fp = (size_t *)malloc(sh.nbFacet * sizeof(size_t));
	size_t nbF3 = 0;
	size_t nbF4 = 0;
	size_t nbFP = 0;

	// Group TRI,QUAD and POLY
	for (size_t i = 0; i < sh.nbFacet; i++) {
		size_t nb = facets[i]->sh.nbIndex;
		if (facets[i]->volumeVisible) {
			if (nb == 3) {
				f3[nbF3++] = i;
			}
			else if (nb == 4) {

				f4[nbF4++] = i;
			}
			else {

				fp[nbFP++] = i;
			}
		}
	}

	// Draw
	glBegin(GL_TRIANGLES);

	// Triangle
	for (size_t i = 0; i < nbF3; i++)
		FillFacet(facets[f3[i]], false);

	// Triangulate polygon
	for (size_t i = 0; i < nbFP; i++)
		Triangulate(facets[fp[i]], false);

	glEnd();

	// Quads
	glBegin(GL_QUADS);
	for (size_t i = 0; i < nbF4; i++)
		FillFacet(facets[f4[i]], false);
	glEnd();

	free(f3);
	free(f4);
	free(fp);

}

void Geometry::SetCullMode(int mode) {

	switch (mode) {
	case 1: // SHOW_FRONT
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	case 2: // SHOW_BACK
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	default: //SHOW_FRONTANDBACK
		glDisable(GL_CULL_FACE);
	}

}

void Geometry::ClearFacetTextures()
{
	GLProgress *prg = new GLProgress("Clearing texture", "Frame update");
	prg->SetBounds(5, 28, 300, 90);
	int startTime = SDL_GetTicks();
	for (int i = 0; i<sh.nbFacet; i++) {
		if (!prg->IsVisible() && ((SDL_GetTicks() - startTime) > 500)) {
			prg->SetVisible(true);
		}
		prg->SetProgress((double)i / (double)sh.nbFacet);
		DELETE_TEX(facets[i]->glTex);
		glGenTextures(1, &facets[i]->glTex);
	}
	prg->SetVisible(false);
	SAFE_DELETE(prg);
}

void Geometry::RenderArrow(GLfloat *matView, float dx, float dy, float dz, float px, float py, float pz, float d) {

	if (!arrowList) BuildShapeList();

	// Compute transformation matrix for the arrow
	GLMatrix mView;
	GLMatrix aView;
	GLMatrix mScale;
	GLMatrix mRot;
	GLMatrix mT;
	float v1x, v1y, v1z;

	mView.LoadGL(matView);

	// Direction
	float n = sqrtf(dx*dx + dy*dy + dz*dz);
	if (IsZero(n)) {

		// Isotropic (render a sphere)
		mScale._11 = (d / 4.0f);
		mScale._22 = (d / 4.0f);
		mScale._33 = (d / 4.0f);

		mT.Translate(px, py, pz);

		aView.Multiply(&mView);
		aView.Multiply(&mT);
		aView.Multiply(&mScale);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(aView.GetGL());
		glCallList(sphereList);
		return;

	}

	dx /= n;
	dy /= n;
	dz /= n;
	mRot._11 = dx;
	mRot._21 = dy;
	mRot._31 = dz;

	// A point belonging to the plane
	// normal to the direction vector
	if (!IsZero(dx)) {
		v1x = -dz / dx;
		v1y = 0.0;
		v1z = 1.0;
	}
	else if (!IsZero(dy)) {
		v1x = 0.0;
		v1y = -dz / dy;
		v1z = 1.0;
	}
	else if (!IsZero(dz)) {
		// normal to z
		v1x = 1.0;
		v1y = 0.0;
		v1z = 0.0;
	}
	else {

		// Null vector -> isotropic
	}

	float n1 = sqrtf(v1x*v1x + v1y*v1y + v1z*v1z);
	v1x /= n1;
	v1y /= n1;
	v1z /= n1;
	mRot._12 = v1x;
	mRot._22 = v1y;
	mRot._32 = v1z;

	// Cross product
	mRot._13 = (dy)*(v1z)-(dz)*(v1y);
	mRot._23 = (dz)*(v1x)-(dx)*(v1z);
	mRot._33 = (dx)*(v1y)-(dy)*(v1x);

	// Scale
	if (!autoNorme) {
		mScale._11 = (n*d*normeRatio);
		mScale._22 = (d / 4.0f);
		mScale._33 = (d / 4.0f);
	}
	else {

		// Show only direction
		mScale._11 = (d / 1.1f);
		mScale._22 = (d / 4.0f);
		mScale._33 = (d / 4.0f);
	}

	mT.Translate(px, py, pz);

	aView.Multiply(&mView);
	aView.Multiply(&mT);
	aView.Multiply(&mRot);
	aView.Multiply(&mScale);
	if (!centerNorme) {
		mT.Translate(0.5f, 0.0, 0.0);
		aView.Multiply(&mT);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(aView.GetGL());
	glCallList(arrowList);

}

// Triangulation stuff

int Geometry::FindEar(POLYGON *p) {

	int i = 0;
	bool earFound = false;
	while (i < p->nbPts && !earFound) {
		if (IsConvex(p, i))
			earFound = !ContainsConcave(p, i - 1, i, i + 1);
		if (!earFound) i++;
	}

	// REM: Theoritically, it should always find an ear (2-Ears theorem).
	// However on degenerated geometry (flat poly) it may not find one.
	// Returns first point in case of failure.
	if (earFound)
		return i;
	else
		return 0;

}

void Geometry::AddTextureCoord(Facet *f, Vector2d *p) {

	// Add texture coord with a 1 texel border (for bilinear filtering)
	double uStep = 1.0 / (double)f->texDimW;
	double vStep = 1.0 / (double)f->texDimH;

#if 1
	double fu = f->sh.texWidthD * uStep;
	double fv = f->sh.texHeightD * vStep;
	glTexCoord2f((float)(uStep + p->u*fu), (float)(vStep + p->v*fv));
#else
	// Show border (debugging purpose)
	double fu = (f->sh.texWidthD + 2.0) * uStep;
	double fv = (f->sh.texHeightD + 2.0) * vStep;
	glTexCoord2f((float)(p->u*fu), (float)(p->v*fv));
#endif

}

void Geometry::FillFacet(Facet *f, bool addTextureCoord) {

	for (size_t i = 0; i < f->sh.nbIndex; i++) {
		size_t idx = f->indices[i];
		glNormal3d(-f->sh.N.x, -f->sh.N.y, -f->sh.N.z);
		if (addTextureCoord) AddTextureCoord(f, f->vertices2 + i);
		glVertex3d(vertices3[idx].x, vertices3[idx].y, vertices3[idx].z);
	}

}

void Geometry::DrawEar(Facet *f, POLYGON *p, int ear, bool addTextureCoord) {

	Vector3d  p3D;
	Vector2d *p1;
	Vector2d *p2;
	Vector2d *p3;

	// Follow orientation
	if (p->sign > 0.0) {
		p1 = &(p->pts[Previous(ear, p->nbPts)]);
		p2 = &(p->pts[Next(ear, p->nbPts)]);
		p3 = &(p->pts[IDX(ear, p->nbPts)]);
	}
	else {

		p1 = &(p->pts[Previous(ear, p->nbPts)]);
		p2 = &(p->pts[IDX(ear, p->nbPts)]);
		p3 = &(p->pts[Next(ear, p->nbPts)]);
	}

	glNormal3d(-f->sh.N.x, -f->sh.N.y, -f->sh.N.z);
	if (addTextureCoord) AddTextureCoord(f, p1);
	// (U,V) -> (x,y,z)
	p3D.x = f->sh.O.x + p1->u*f->sh.U.x + p1->v*f->sh.V.x;
	p3D.y = f->sh.O.y + p1->u*f->sh.U.y + p1->v*f->sh.V.y;
	p3D.z = f->sh.O.z + p1->u*f->sh.U.z + p1->v*f->sh.V.z;
	glVertex3d(p3D.x, p3D.y, p3D.z);

	glNormal3d(-f->sh.N.x, -f->sh.N.y, -f->sh.N.z);
	if (addTextureCoord) AddTextureCoord(f, p2);
	// (U,V) -> (x,y,z)
	p3D.x = f->sh.O.x + p2->u*f->sh.U.x + p2->v*f->sh.V.x;
	p3D.y = f->sh.O.y + p2->u*f->sh.U.y + p2->v*f->sh.V.y;
	p3D.z = f->sh.O.z + p2->u*f->sh.U.z + p2->v*f->sh.V.z;
	glVertex3d(p3D.x, p3D.y, p3D.z);

	glNormal3d(-f->sh.N.x, -f->sh.N.y, -f->sh.N.z);
	if (addTextureCoord) AddTextureCoord(f, p3);
	// (U,V) -> (x,y,z)
	p3D.x = f->sh.O.x + p3->u*f->sh.U.x + p3->v*f->sh.V.x;
	p3D.y = f->sh.O.y + p3->u*f->sh.U.y + p3->v*f->sh.V.y;
	p3D.z = f->sh.O.z + p3->u*f->sh.U.z + p3->v*f->sh.V.z;
	glVertex3d(p3D.x, p3D.y, p3D.z);

}

void Geometry::Triangulate(Facet *f, bool addTextureCoord) {

	// Triangulate a facet (rendering purpose)
	// The facet must have at least 3 points
	// Use the very simple "Two-Ears" theorem. It computes in O(n^2).

	// Build a POLYGON
	POLYGON p;
	p.nbPts = f->sh.nbIndex;
	p.pts = (Vector2d *)malloc(p.nbPts * sizeof(Vector2d));
	memcpy(p.pts, f->vertices2, p.nbPts * sizeof(Vector2d));
	p.sign = f->sh.sign;

	if (f->sh.sign == 0.0) {
		// Not a simple polygon
		// Abort triangulation
		free(p.pts);
		return;
	}

	// Perform triangulation
	while (p.nbPts > 3) {
		int e = FindEar(&p);
		DrawEar(f, &p, e, addTextureCoord);
		// Remove the ear
		for (int i = e; i < p.nbPts - 1; i++)
			p.pts[i] = p.pts[i + 1];
		p.nbPts--; //Underrun-safe
	}

	// Draw the last ear
	DrawEar(f, &p, 0, addTextureCoord);
	free(p.pts);

}

void Geometry::Render(GLfloat *matView, bool renderVolume, bool renderTexture, int showMode, bool filter, bool showHidden, bool showMesh, bool showDir) {

	if (!isLoaded) return;

	// Render the geometry
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	
	// Render Volume
	if (renderVolume) {
		glPolygonOffset(1.0f, 4.0f);
		SetCullMode(showMode);
		GLfloat global_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHTING);
		GLToolkit::SetMaterial(&fillMaterial);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glCallList(polyList);
		glDisable(GL_POLYGON_OFFSET_FILL);
		GLToolkit::SetMaterial(&whiteMaterial);
		glDisable(GL_LIGHTING);
		//gldebug
		GLToolkit::CheckGLErrors("GLContainer::PaintComponents()");
	}
	else {

		// Default material
		GLToolkit::SetMaterial(&whiteMaterial);

		// Draw lines
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);

		float color = (mApp->whiteBg) ? 0.0f : 1.0f; //whitebg here
		if (viewStruct == -1) {
			glColor4f(color, color, color, 0.5f);
			if (mApp->antiAliasing) {
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			}
			glPolygonOffset(1.0f, 5.0f);
			glEnable(GL_POLYGON_OFFSET_LINE);
			for (int i = 0;i < sh.nbSuper;i++)
				glCallList(lineList[i]);
			glDisable(GL_POLYGON_OFFSET_LINE);
			glDisable(GL_BLEND);
			glDisable(GL_LINE_SMOOTH);
			glColor3f(1.0f, 1.0f, 1.0f);
		}
		else {

			// Draw non selectable facet in dark grey
			glColor3f(0.2f, 0.2f, 0.2f);
			if (mApp->antiAliasing) {
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			}
			for (int i = 0;i < sh.nbSuper;i++)
				if (viewStruct != i)
					glCallList(lineList[i]);
			// Selectable in white
			glColor3f(color, color, color);
			glPolygonOffset(1.0f, 5.0f);
			glEnable(GL_POLYGON_OFFSET_LINE);
			glCallList(lineList[viewStruct]);
			glDisable(GL_POLYGON_OFFSET_LINE);
			if (mApp->antiAliasing) {
				glDisable(GL_BLEND);
				glDisable(GL_LINE_SMOOTH);
			}
		}
	}

	// Paint texture
	if (renderTexture) {
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 3.0f);
		for (size_t i = 0;i < sh.nbFacet && renderTexture;i++) {
			Facet *f = facets[i];
			bool paintRegularTexture = f->sh.isTextured && f->textureVisible && (f->sh.countAbs || f->sh.countRefl || f->sh.countTrans);
#ifdef MOLFLOW
			paintRegularTexture = paintRegularTexture || (f->sh.isTextured && f->textureVisible && (f->sh.countACD || f->sh.countDes));
#endif
			if (paintRegularTexture) {
				if (f->sh.is2sided)   glDisable(GL_CULL_FACE);
				else                   SetCullMode(showMode);
				glBindTexture(GL_TEXTURE_2D, f->glTex);
				if (filter) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				}
				else {

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				}
				
				glCallList(f->glList);
			}
		}
		glDisable(GL_POLYGON_OFFSET_FILL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
	}

	// Paint mesh
	if (showMesh) {
		glColor4f(0.7f, 0.7f, 0.7f, 0.3f);
		if (mApp->antiAliasing) {
			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (int i = 0; i < sh.nbFacet;i++) {

			Facet *f = facets[i];
			if (f->cellPropertiesIds  && f->textureVisible) {
				if (!f->glElem) f->BuildMeshList();

				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(1.0f, 2.0f);
				glCallList(f->glElem);
				glDisable(GL_POLYGON_OFFSET_LINE);
			}
		}
		if (mApp->antiAliasing) {
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_BLEND);
		}
	}

	// Paint direction fields
	if (showDir) {

		GLToolkit::SetMaterial(&arrowMaterial);
		for (int i = 0;i < sh.nbFacet;i++) {
			Facet *f = facets[i];
			if (f->sh.countDirection && f->dirCache) {
				double iw = 1.0 / (double)f->sh.texWidthD;
				double ih = 1.0 / (double)f->sh.texHeightD;
				double rw = f->sh.U.Norme() * iw;
				for (int x = 0;x < f->sh.texWidth;x++) {
					for (int y = 0;y < f->sh.texHeight;y++) {
						size_t add = x + y*f->sh.texWidth;
						if (f->GetMeshArea(add) > 0.0) {
							double uC = ((double)x + 0.5) * iw;
							double vC = ((double)y + 0.5) * ih;
							float xc = (float)(f->sh.O.x + f->sh.U.x*uC + f->sh.V.x*vC);
							float yc = (float)(f->sh.O.y + f->sh.U.y*uC + f->sh.V.y*vC);
							float zc = (float)(f->sh.O.z + f->sh.U.z*uC + f->sh.V.z*vC);

							RenderArrow(matView,
								(float)f->dirCache[add].dir.x,
								(float)f->dirCache[add].dir.y,
								(float)f->dirCache[add].dir.z,
								xc, yc, zc, (float)rw);
						}
					}
				}
			}
		}

		// Restore default matrix
		glLoadMatrixf(matView);
	}

	// Paint selection
	if (GetNbSelectedFacets()>0) {
		if (mApp->antiAliasing) {
			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);
		}
		glBlendFunc(GL_ONE, GL_ZERO);
		glColor3f(1.0f, 0.0f, 0.0f);    //red
		if (showHidden) {
			glDisable(GL_DEPTH_TEST);
			glCallList(selectList3);
			glEnable(GL_DEPTH_TEST);
		}
		else {
			glCallList(selectList3);
		}
		if (mApp->antiAliasing) {
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_BLEND);
		}

	}

	// Paint selected cell on mesh
	for (int i = 0; i < sh.nbFacet;i++) {
		Facet *f = facets[i];
		f->RenderSelectedElem();
	}

}

void Geometry::DeleteGLLists(bool deletePoly, bool deleteLine) {
	if (deleteLine) {
		for (int i = 0; i < sh.nbSuper; i++)
			DELETE_LIST(lineList[i]);
	}
	if (deletePoly) DELETE_LIST(polyList);
	DELETE_LIST(selectList);
	DELETE_LIST(selectList2);
	DELETE_LIST(selectList3);
}

std::vector<bool> Geometry::GetVertexBelongsToSelectedFacet() {
	std::vector<bool> result(sh.nbVertex, false);
	std::vector<size_t> selFacetIds = GetSelectedFacets();
	for (auto facetId : selFacetIds) {
		Facet* f = facets[facetId];
		for (size_t i = 0; i < f->sh.nbIndex; i++)
			result[f->indices[i]] = true;
	}
	return result;
}