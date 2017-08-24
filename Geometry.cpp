#pragma once
//Common geometry handling/editing features, shared between Molflow and Synrad

#include "Geometry.h"
#include "Facet.h"
#include "GLApp\MathTools.h"
#include "GLApp\GLMessageBox.h"
#include "GLApp\GLToolkit.h"
#include "SplitFacet.h"
#include "BuildIntersection.h"
#include "MirrorFacet.h"
#include "MirrorVertex.h"
#include "GLApp/GLList.h"

#include "Clipper\clipper.hpp"

#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#include "ASELoader.h"
//#include <algorithm>
#include <list>

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

Geometry::Geometry() {
	facets = NULL;
	vertices3 = NULL;
	polyList = 0;
	selectList = 0;
	selectList2 = 0;
	selectList3 = 0;
	arrowList = 0;
	sphereList = 0;

	autoNorme = true;
	centerNorme = true;
	normeRatio = 1.0f;
	texAutoScale = true;
	texLogScale = false;
	texColormap = true;

	sh.nbSuper = 0;
	viewStruct = -1;
	strcpy(strPath, "");
}

Geometry::~Geometry() {
	Clear();
}

void Geometry::CheckCollinear() {
	char tmp[256];
	// Check collinear polygon
	int nbCollinear = 0;
	for (int i = 0; i < GetNbFacet(); i++) {
		if (GetFacet(i)->collinear) nbCollinear++;
	}
	bool ok = false;
	if (nbCollinear) {
		sprintf(tmp, "%d null polygon(s) found !\nThese polygons have all vertices on a single line, thus they do nothing.\nDelete them?", nbCollinear);
		ok = GLMessageBox::Display(tmp, "Info", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK;
	}
	if (ok) RemoveCollinear();
}

void Geometry::CheckNonSimple() {
	char tmp[256];
	// Check non simple polygon
	int *nonSimpleList = (int *)malloc(GetNbFacet() * sizeof(int));
	int nbNonSimple = 0;
	for (int i = 0; i < GetNbFacet(); i++) {
		if (GetFacet(i)->sh.sign == 0.0)
			nonSimpleList[nbNonSimple++] = i;
	}
	bool ok = false;
	if (nbNonSimple) {
		sprintf(tmp, "%d non simple (or null) polygon(s) found !\nSome tasks may not work properly\n"
			"Should I try to correct them (vertex shifting)?", nbNonSimple);
		ok = GLMessageBox::Display(tmp, "Warning", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK;
	}

	if (ok) CorrectNonSimple(nonSimpleList, nbNonSimple);
	SAFE_FREE(nonSimpleList);
}

void Geometry::CheckIsolatedVertex() {
	int nbI = HasIsolatedVertices();
	if (nbI) {
		char tmp[256];
		sprintf(tmp, "Remove %d isolated vertices ?", nbI);
		if (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK) {
			DeleteIsolatedVertices(false);
		}
	}
}

void Geometry::InitializeGeometry(int facet_number) {

	RecalcBoundingBox(facet_number);

	// Choose an orthogonal (U,V) 2D basis for each facet. (This method can be 
	// improved. stub). The algorithm chooses the longest vedge for the U vector.
	// then it computes V (orthogonal to U and N). Afterwards, U and V are rescaled 
	// so each facet vertex are included in the rectangle defined by uU + vV (0<=u<=1 
	// and 0<=v<=1) of origin f->sh.O, U and V are always orthogonal and (U,V,N) 
	// form a 3D left handed orthogonal basis (not necessary orthonormal).
	// This coordinates system allows to prevent from possible "almost degenerated"
	// basis on fine geometry. It also greatly eases the facet/ray instersection routine 
	// and ref/abs/des hit recording and visualization. In order to ease some calculations, 
	// nU et nV (normalized U et V) are also stored in the Facet structure.
	// The local coordinates of facet vertex are stored in (U,V) coordinates (vertices2).

	size_t fOffset = sizeof(SHGHITS);
	for (int i = 0; i < sh.nbFacet; i++) {
		//initGeoPrg->SetProgress((double)i/(double)sh.nbFacet);
		if ((facet_number == -1) || (i == facet_number)) { //permits to initialize only one facet
														   // Main facet params
			// Current facet
			Facet *f = facets[i];
			CalculateFacetParam(f);

			// Detect non visible edge
			f->InitVisibleEdge();

			// Detect orientation
			f->DetectOrientation();

			if (facet_number == -1) {
				// Hit address
				f->sh.hitOffset = fOffset;
#ifdef MOLFLOW
				fOffset += f->GetHitsSize(mApp->worker.moments.size());
#endif

#ifdef SYNRAD
				fOffset += f->GetHitsSize();
#endif
			}
		}
	}

	isLoaded = true;
	if (facet_number == -1) {
		BuildGLList();
		mApp->UpdateModelParams();
		mApp->UpdateFacetParams();
	}

	//initGeoPrg->SetVisible(false);
	//SAFE_DELETE(initGeoPrg);
	_ASSERTE(_CrtCheckMemory());
}

void Geometry::RecalcBoundingBox(int facet_number) {
	// Perform various precalculation here for a faster simulation.

	//GLProgress *initGeoPrg = new GLProgress("Initializing geometry...","Please wait");
	//initGeoPrg->SetProgress(0.0);
	//initGeoPrg->SetVisible(true);
	if (facet_number == -1) { //bounding box for all vertices
		bb.min.x = 1e100;
		bb.min.y = 1e100;
		bb.min.z = 1e100;
		bb.max.x = -1e100;
		bb.max.y = -1e100;
		bb.max.z = -1e100;

		// Axis Aligned Bounding Box
		for (int i = 0; i < sh.nbVertex; i++) {
			Vector3d p = vertices3[i];
			if (!(vertices3[i].selected == false || vertices3[i].selected == true)) vertices3[i].selected = false; //initialize selection
			if (p.x < bb.min.x) bb.min.x = p.x;
			if (p.y < bb.min.y) bb.min.y = p.y;
			if (p.z < bb.min.z) bb.min.z = p.z;
			if (p.x > bb.max.x) bb.max.x = p.x;
			if (p.y > bb.max.y) bb.max.y = p.y;
			if (p.z > bb.max.z) bb.max.z = p.z;
		}

#ifdef SYNRAD //Regions
		Worker *worker = &(mApp->worker);
		for (int i = 0; i < (int)worker->regions.size(); i++) {
			if (worker->regions[i].AABBmin.x < bb.min.x) bb.min.x = worker->regions[i].AABBmin.x;
			if (worker->regions[i].AABBmin.y < bb.min.y) bb.min.y = worker->regions[i].AABBmin.y;
			if (worker->regions[i].AABBmin.z < bb.min.z) bb.min.z = worker->regions[i].AABBmin.z;
			if (worker->regions[i].AABBmax.x > bb.max.x) bb.max.x = worker->regions[i].AABBmax.x;
			if (worker->regions[i].AABBmax.y > bb.max.y) bb.max.y = worker->regions[i].AABBmax.y;
			if (worker->regions[i].AABBmax.z > bb.max.z) bb.max.z = worker->regions[i].AABBmax.z;
		}
#endif
	}
	else { //bounding box only for the changed facet
		for (int i = 0; i < facets[facet_number]->sh.nbIndex; i++) {
			Vector3d p = vertices3[facets[facet_number]->indices[i]];
			//if(!(vertices3[i].selected==false || vertices3[i].selected==true)) vertices3[i].selected=false; //initialize selection
			if (p.x < bb.min.x) bb.min.x = p.x;
			if (p.y < bb.min.y) bb.min.y = p.y;
			if (p.z < bb.min.z) bb.min.z = p.z;
			if (p.x > bb.max.x) bb.max.x = p.x;
			if (p.y > bb.max.y) bb.max.y = p.y;
			if (p.z > bb.max.z) bb.max.z = p.z;
		}
	}

	center.x = (bb.max.x + bb.min.x) / 2.0;
	center.y = (bb.max.y + bb.min.y) / 2.0;
	center.z = (bb.max.z + bb.min.z) / 2.0;
}

void Geometry::CorrectNonSimple(int *nonSimpleList, int nbNonSimple) {
	mApp->changedSinceSave = true;
	Facet *f;
	for (int i = 0; i < nbNonSimple; i++) {
		f = GetFacet(nonSimpleList[i]);
		if (f->sh.sign == 0.0) {
			int j = 0;
			while ((j < f->sh.nbIndex) && (f->sh.sign == 0.0)) {
				f->ShiftVertex();
				InitializeGeometry(nonSimpleList[i]);
				//f->DetectOrientation();
				j++;
			}
		}
	}
	//BuildGLList();
}

size_t Geometry::AnalyzeNeighbors(Worker *work, GLProgress *prg)
{
	size_t i = 0;
	work->abortRequested = false;
	for (i = 0; i < sh.nbFacet; i++) {
		facets[i]->neighbors.clear();
	}
	for (i = 0; !work->abortRequested && i < sh.nbFacet; i++) {
		mApp->DoEvents();
		Facet *f1 = facets[i];
		prg->SetProgress(double(i) / double(sh.nbFacet));
		for (size_t j = i + 1; j < sh.nbFacet; j++) {
			Facet *f2 = facets[j];
			size_t c1, c2, l;
			if (GetCommonEdges(facets[i], facets[j], &c1, &c2, &l)) {
				double dotProduct = Dot(f1->sh.N, f2->sh.N);
				Saturate(dotProduct, -1.0, 1.0); //Rounding errors...
				double angleDiff = fabs(acos(dotProduct));
				NeighborFacet n1, n2;
				n1.id = i;
				n2.id = j;
				n1.angleDiff = n2.angleDiff = angleDiff;
				f1->neighbors.push_back(n2);
				f2->neighbors.push_back(n1);
			}
		}
	}
	return i;
}

std::vector<size_t> Geometry::GetConnectedFacets(size_t sourceFacetId, double maxAngleDiff)
{
	std::vector<size_t> connectedFacets;
	std::vector<bool> alreadyConnected(sh.nbFacet, false);

	std::vector<size_t> toCheck;

	toCheck.push_back(sourceFacetId);
	do {
		//One iteration
		std::vector<size_t> toCheckNext;
		for (auto facetId : toCheck) {
			for (auto neighbor : facets[facetId]->neighbors) {
				if (neighbor.id < sh.nbFacet) { //Protect against changed geometry
					if (neighbor.angleDiff <= maxAngleDiff) {
						if (!alreadyConnected[neighbor.id]) toCheckNext.push_back(neighbor.id);
						alreadyConnected[neighbor.id] = true;
					}
				}
			}
		}
		toCheck = toCheckNext;
	} while (toCheck.size() != 0);
	for (size_t index = 0; index < sh.nbFacet; index++) {
		if (alreadyConnected[index] || index == sourceFacetId)
			connectedFacets.push_back(index);
	}
	return connectedFacets;
}
size_t Geometry::GetNbVertex() {
	return sh.nbVertex;
}

Vector3d Geometry::GetFacetCenter(int facet) {

	return facets[facet]->sh.center;

}

size_t Geometry::GetNbStructure() {
	return sh.nbSuper;
}

char *Geometry::GetStructureName(int idx) {
	return strName[idx];
}

void Geometry::CreatePolyFromVertices_Convex() {
	//creates facet from selected vertices

	mApp->changedSinceSave = true;

	auto selectedVertices = GetSelectedVertices();

	if (selectedVertices.size() < 3) {
		char errMsg[512];
		sprintf(errMsg, "Select at least 3 vertices.");
		throw Error(errMsg);
		return;
	}//at least three vertices

	Vector3d U, V, N;
	U = (vertices3[selectedVertices[0]] - vertices3[selectedVertices[1]]).Normalized();

	int i2 = 2;
	do {
		V = (vertices3[selectedVertices[0]] - vertices3[selectedVertices[i2]]).Normalized();
		i2++;
	} while (Dot(U, V) > 0.99 && i2 < selectedVertices.size()); //if U and V are almost the same, the projection would be inaccurate

	//Now we have the U,V plane, let's define it by computing the normal vector:
	N = CrossProduct(V, U).Normalized(); //We have a normal vector

	V = CrossProduct(N, U).Normalized(); //Make V perpendicular to U and N (and still in the U,V plane)

	std::vector<Vector2d> projected;

	//Get coordinates in the U,V system
	for (auto sel : selectedVertices) {
		projected.push_back(ProjectVertex(vertices3[sel], U, V, vertices3[selectedVertices[0]]));
	}

	//Graham scan here on the projected[] array
	int *returnList = (int *)malloc(selectedVertices.size() * sizeof(int));
	grahamMain(projected.data(), selectedVertices.size(), returnList);
	int ii, loopLength;
	for (ii = 0; ii < selectedVertices.size(); ii++) {
		if (returnList[ii] == returnList[0] && ii > 0) break;
	}
	loopLength = ii;
	//End graham scan

	//a new facet
	sh.nbFacet++;
	facets = (Facet **)realloc(facets, sh.nbFacet * sizeof(Facet *));
	facets[sh.nbFacet - 1] = new Facet(loopLength);
	//facets[sh.nbFacet - 1]->sh.sticking = 0.0;
	//facets[sh.nbFacet - 1]->sh.sticking = DES_NONE;
	if (viewStruct != -1) facets[sh.nbFacet - 1]->sh.superIdx = viewStruct;
	//set selection
	UnselectAll();
	facets[sh.nbFacet - 1]->selected = true;
	for (int i = 0; i < loopLength; i++) {
		facets[sh.nbFacet - 1]->indices[i] = selectedVertices[returnList[i]];
	}
	SAFE_FREE(returnList);

	InitializeGeometry();
	mApp->UpdateFacetParams(true);
	UpdateSelection();
	mApp->facetList->SetSelectedRow((int)sh.nbFacet - 1);
	mApp->facetList->ScrollToVisible(sh.nbFacet - 1, 1, false);
}

void Geometry::CreatePolyFromVertices_Order() {
	//creates facet from selected vertices

	mApp->changedSinceSave = true;

	//a new facet
	sh.nbFacet++;
	facets = (Facet **)realloc(facets, sh.nbFacet * sizeof(Facet *));
	facets[sh.nbFacet - 1] = new Facet((int)selectedVertexList_ordered.size());
	//facets[sh.nbFacet - 1]->sh.sticking = 0.0;
	//facets[sh.nbFacet - 1]->sh.sticking = DES_NONE;
	if (viewStruct != -1) facets[sh.nbFacet - 1]->sh.superIdx = viewStruct;
	//set selection
	UnselectAll();
	facets[sh.nbFacet - 1]->selected = true;
	for (size_t i = 0; i < selectedVertexList_ordered.size(); i++) {
		facets[sh.nbFacet - 1]->indices[i] = selectedVertexList_ordered[i];
	}

	InitializeGeometry();
	mApp->UpdateFacetParams(true);
	UpdateSelection();
	mApp->facetList->SetSelectedRow((int)sh.nbFacet - 1);
	mApp->facetList->ScrollToVisible(sh.nbFacet - 1, 1, false);
}

void Geometry::CreateDifference() {
	//creates facet from selected vertices

	mApp->changedSinceSave = true;
	size_t nbSelectedVertex = 0;

	auto selectedFacets = GetSelectedFacets();
	if (selectedFacets.size() != 2) {
		char errMsg[512];
		sprintf(errMsg, "Select exactly 2 facets.");
		throw Error(errMsg);
		return;
	}//at least three vertices

	size_t firstFacet = selectedFacets[0];
	size_t secondFacet = selectedFacets[1];

	//TO DO:
	//swap if normals not collinear
	//shift vertex to make nice cut

	//a new facet
	sh.nbFacet++;
	facets = (Facet **)realloc(facets, sh.nbFacet * sizeof(Facet *));
	facets[sh.nbFacet - 1] = new Facet(facets[firstFacet]->sh.nbIndex + facets[secondFacet]->sh.nbIndex + 2);
	facets[sh.nbFacet - 1]->sh.sticking = 0.0;
	//set selection
	UnselectAll();
	facets[sh.nbFacet - 1]->selected = true;
	selectedFacets = { sh.nbFacet - 1 };
	if (viewStruct != -1) facets[sh.nbFacet - 1]->sh.superIdx = viewStruct;
	//one circle on first facet
	size_t counter = 0;
	for (size_t i = 0; i < facets[firstFacet]->sh.nbIndex; i++)
		facets[sh.nbFacet - 1]->indices[counter++] = facets[firstFacet]->indices[i];
	//close circle by adding the first vertex again
	facets[sh.nbFacet - 1]->indices[counter++] = facets[firstFacet]->indices[0];
	//reverse circle on second facet
	for (size_t i = facets[secondFacet]->sh.nbIndex - 1; i >= 0; i--)
		facets[sh.nbFacet - 1]->indices[counter++] = facets[secondFacet]->GetIndex((int)i + 1);
	//close circle by adding the first vertex again
	facets[sh.nbFacet - 1]->indices[counter++] = facets[secondFacet]->indices[0];

	InitializeGeometry();
	mApp->UpdateFacetParams(true);
	UpdateSelection();
	mApp->facetList->SetSelectedRow((int)sh.nbFacet - 1);
	mApp->facetList->ScrollToVisible(sh.nbFacet - 1, 1, false);
}

void Geometry::ClipSelectedPolygons(ClipperLib::ClipType type, int reverseOrder) {
	if (GetNbSelectedFacets() != 2) {
		char errMsg[512];
		sprintf(errMsg, "Select exactly 2 facets.");
		throw Error(errMsg);
		return;
	}//at least three vertices

	int firstFacet = -1;
	int secondFacet = -1;;
	for (int i = 0; i < sh.nbFacet && secondFacet < 0; i++)
	{
		if (facets[i]->selected) {
			if (firstFacet < 0) firstFacet = i;
			else (secondFacet = i);
		}
	}
	if (reverseOrder == 0) ClipPolygon(firstFacet, secondFacet, type);
	else if (reverseOrder == 1) ClipPolygon(secondFacet, firstFacet, type);
	else {
		//Auto
		ClipperLib::PolyTree solution;
		std::vector<ProjectedPoint> projectedPoints;

		std::vector<size_t> facet2path;
		for (size_t i = 0; i < facets[secondFacet]->sh.nbIndex; i++) {
			facet2path.push_back(facets[secondFacet]->indices[i]);
		}
		std::vector<std::vector<size_t>> clippingPaths;
		clippingPaths.push_back(facet2path);
		size_t id = (int)firstFacet;
		if (ExecuteClip(id, clippingPaths, projectedPoints, solution, type) > 0) {
			ClipPolygon(firstFacet, secondFacet, type);
		}
		else {
			facet2path.clear();
			for (size_t i = 0; i < facets[firstFacet]->sh.nbIndex; i++) {
				facet2path.push_back(facets[firstFacet]->indices[i]);
			}
			clippingPaths.clear();
			clippingPaths.push_back(facet2path);
			id = (int)secondFacet;
			if (ExecuteClip(id, clippingPaths, projectedPoints, solution, type) > 0) {
				ClipPolygon(secondFacet, firstFacet, type);
			}
		}
	}
}

void Geometry::ClipPolygon(size_t id1, std::vector<std::vector<size_t>> clippingPaths, ClipperLib::ClipType type) {
	mApp->changedSinceSave = true;
	ClipperLib::PolyTree solution;
	std::vector<ProjectedPoint> projectedPoints;
	ExecuteClip(id1, clippingPaths, projectedPoints, solution, type);

	//a new facet
	size_t nbNewFacets = solution.ChildCount();
	facets = (Facet **)realloc(facets, (sh.nbFacet + nbNewFacets) * sizeof(Facet *));
	//set selection
	UnselectAll();
	std::vector<InterfaceVertex> newVertices;
	for (size_t i = 0; i < nbNewFacets; i++) {
		bool hasHole = solution.Childs[i]->ChildCount() > 0;
		size_t closestIndexToChild, closestIndexToParent;
		double minDist = 9E99;
		if (hasHole) {
			for (size_t j = 0; j < solution.Childs[i]->Contour.size(); j++) { //Find closest parent point
				Vector2d vert;
				vert.u = 1E-6*(double)solution.Childs[i]->Contour[j].X;
				vert.v = 1E-6*(double)solution.Childs[i]->Contour[j].Y;
				for (size_t k = 0; k < solution.Childs[i]->Childs[0]->Contour.size(); k++) {//Find closest child point
					Vector2d childVert;
					childVert.u = 1E-6*(double)solution.Childs[i]->Childs[0]->Contour[k].X;
					childVert.v = 1E-6*(double)solution.Childs[i]->Childs[0]->Contour[k].Y;
					double dist = Sqr(facets[id1]->sh.U.Norme() * (vert.u - childVert.u)) + Sqr(facets[id1]->sh.V.Norme() * (vert.v - childVert.v));
					if (dist < minDist) {
						minDist = dist;
						closestIndexToChild = j;
						closestIndexToParent = k;
					}
				}
			}
		}
		size_t nbRegistered = 0;
		size_t nbVertex;
		if (!hasHole)
			nbVertex = solution.Childs[i]->Contour.size();
		else
			nbVertex = solution.Childs[i]->Contour.size() + 2 + solution.Childs[i]->Childs[0]->Contour.size();
		Facet *f = new Facet(nbVertex);
		for (size_t j = 0; j < solution.Childs[i]->Contour.size(); j++) {
			Vector2d vert;
			if (hasHole && j == closestIndexToChild) { //Create hole
				vert.u = 1E-6*(double)solution.Childs[i]->Contour[j].X;
				vert.v = 1E-6*(double)solution.Childs[i]->Contour[j].Y;
				RegisterVertex(f, vert, id1, projectedPoints, newVertices, nbRegistered++);//Register entry from parent
				for (size_t k = 0; k < solution.Childs[i]->Childs[0]->Contour.size(); k++) { //Register hole
					vert.u = 1E-6*(double)solution.Childs[i]->Childs[0]->Contour[(k + closestIndexToParent) % solution.Childs[i]->Childs[0]->Contour.size()].X;
					vert.v = 1E-6*(double)solution.Childs[i]->Childs[0]->Contour[(k + closestIndexToParent) % solution.Childs[i]->Childs[0]->Contour.size()].Y;
					RegisterVertex(f, vert, id1, projectedPoints, newVertices, nbRegistered++);
				}
				vert.u = 1E-6*(double)solution.Childs[i]->Childs[0]->Contour[closestIndexToParent].X;
				vert.v = 1E-6*(double)solution.Childs[i]->Childs[0]->Contour[closestIndexToParent].Y;
				RegisterVertex(f, vert, id1, projectedPoints, newVertices, nbRegistered++); //Re-register hole entry point before exit
																								 //re-register parent entry
				vert.u = 1E-6*(double)solution.Childs[i]->Contour[j].X;
				vert.v = 1E-6*(double)solution.Childs[i]->Contour[j].Y;
				RegisterVertex(f, vert, id1, projectedPoints, newVertices, nbRegistered++);

			}
			else {
				vert.u = 1E-6*(double)solution.Childs[i]->Contour[j].X;
				vert.v = 1E-6*(double)solution.Childs[i]->Contour[j].Y;
				RegisterVertex(f, vert, id1, projectedPoints, newVertices, nbRegistered++);
			}

		}
		f->selected = true;
		if (viewStruct != -1) f->sh.superIdx = viewStruct;
		facets[sh.nbFacet + i] = f;
	}
	sh.nbFacet += nbNewFacets;
	vertices3 = (InterfaceVertex*)realloc(vertices3, sizeof(InterfaceVertex)*(sh.nbVertex + newVertices.size()));
	for (InterfaceVertex newVert : newVertices)
		vertices3[sh.nbVertex++] = newVert;

	InitializeGeometry();
	mApp->UpdateFacetParams(true);
	UpdateSelection();
}

size_t Geometry::ExecuteClip(size_t& id1, std::vector<std::vector<size_t>>& clippingPaths, std::vector<ProjectedPoint>& projectedPoints, ClipperLib::PolyTree& solution, ClipperLib::ClipType& type) {
	ClipperLib::Paths subj(1), clip(clippingPaths.size());

	for (size_t i1 = 0; i1 < facets[id1]->sh.nbIndex; i1++) {
		subj[0] << ClipperLib::IntPoint((int)(facets[id1]->vertices2[i1].u*1E6), (int)(facets[id1]->vertices2[i1].v*1E6));
	}

	for (size_t i3 = 0; i3 < clippingPaths.size(); i3++) {
		for (size_t i2 = 0; i2 < clippingPaths[i3].size(); i2++) {
			ProjectedPoint proj;
			proj.globalId = clippingPaths[i3][i2];
			proj.vertex2d = ProjectVertex(vertices3[clippingPaths[i3][i2]], facets[id1]->sh.U, facets[id1]->sh.V, facets[id1]->sh.O);
			clip[0] << ClipperLib::IntPoint((int)(proj.vertex2d.u*1E6), (int)(proj.vertex2d.v*1E6));
			projectedPoints.push_back(proj);
		}
	}
	ClipperLib::Clipper c;
	c.AddPaths(subj, ClipperLib::ptSubject, true);
	c.AddPaths(clip, ClipperLib::ptClip, true);
	c.Execute(type, solution, ClipperLib::pftNonZero, ClipperLib::pftNonZero);
	return solution.ChildCount();
}

void Geometry::ClipPolygon(size_t id1, size_t id2, ClipperLib::ClipType type) {
	std::vector<size_t> facet2path;
	for (size_t i = 0; i < facets[id2]->sh.nbIndex; i++) {
		facet2path.push_back(facets[id2]->indices[i]);
	}
	std::vector<std::vector<size_t>> clippingPaths;
	clippingPaths.push_back(facet2path);
	ClipPolygon(id1, clippingPaths, type);
}

void Geometry::RegisterVertex(Facet *f, const Vector2d &vert, size_t id1, const std::vector<ProjectedPoint> &projectedPoints, std::vector<InterfaceVertex> &newVertices, size_t registerLocation) {
	int foundId = -1;
	for (size_t k = 0; foundId == -1 && k < facets[id1]->sh.nbIndex; k++) { //Check if part of facet 1
		double dist = (vert - facets[id1]->vertices2[k]).Norme();
		foundId = (dist < 1E-5) ? (int)facets[id1]->indices[k] : -1;
	}
	for (size_t k = 0; foundId == -1 && k < projectedPoints.size(); k++) { //Check if part of facet 2
		double dist = (vert - projectedPoints[k].vertex2d).Norme();
		foundId = (dist < 1E-5) ? (int)projectedPoints[k].globalId : -1;
	}
	if (foundId == -1) { //Create new vertex
		InterfaceVertex newVertex;
		newVertex.selected = true;
		newVertex.SetLocation(facets[id1]->sh.O + vert.u*facets[id1]->sh.U + vert.v*facets[id1]->sh.V);
		f->indices[registerLocation] = sh.nbVertex + newVertices.size();
		newVertices.push_back(newVertex);
	}
	else { //Vertex already exists
		f->indices[registerLocation] = foundId;
	}
}

void Geometry::SelectCoplanar(int width, int height, double tolerance) {

	auto selectedVertices = GetSelectedVertices();
	if (selectedVertices.size() < 3) {
		GLMessageBox::Display("Select at least 3 vertices", "Can't define plane", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}

	Vector3d U = (vertices3[selectedVertices[0]] - vertices3[selectedVertices[1]]).Normalized();
	Vector3d V = (vertices3[selectedVertices[0]] - vertices3[selectedVertices[2]]).Normalized();
	Vector3d N = CrossProduct(V, U);
	double nN = N.Norme();
	if (nN < 1e-8) {
		GLMessageBox::Display("Sorry, the 3 selected vertices are on a line.", "Can't define plane", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	N = 1.0 / nN * N; // Normalize N

	// Plane equation
	double A = N.x;
	double B = N.y;
	double C = N.z;
	Vector3d p0 = vertices3[selectedVertices[0]];
	double D = -Dot(N, p0);

	//double denominator=sqrt(pow(A,2)+pow(B,2)+pow(C,2));
	double distance;

	int outX, outY;

	for (int i = 0; i < sh.nbVertex; i++) {
		Vector3d *v = GetVertex(i);
		bool onScreen = GLToolkit::Get2DScreenCoord((float)v->x, (float)v->y, (float)v->z, &outX, &outY); //To improve
		onScreen = (onScreen && outX >= 0 && outY >= 0 && outX <= (width) && (outY <= height));
		if (onScreen) {
			distance = abs(A*v->x + B*v->y + C*v->z + D);
			if (distance < tolerance) { //vertex is on the plane

				vertices3[i].selected = true;

			}
			else {
				vertices3[i].selected = false;
			}
		}
		else {
			vertices3[i].selected = false;
		}
	}
}

InterfaceVertex* Geometry::GetVertex(size_t idx) {
	return vertices3 + idx;
}

Facet *Geometry::GetFacet(size_t facet) {
	if (facet >= sh.nbFacet || facet < 0) {
		char errMsg[512];
		sprintf(errMsg, "Geometry::GetFacet()\nA process tried to access facet #%zd that doesn't exist.\nAutoSaving and probably crashing...", facet + 1);
		GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
		mApp->AutoSave(true);
		throw Error(errMsg);
	}
	return facets[facet];
}

size_t Geometry::GetNbFacet() {
	return sh.nbFacet;
}

AABB Geometry::GetBB() {

	if (viewStruct < 0) {

		return bb;

	}
	else {
		// BB of selected struture
		AABB sbb;

		sbb.min.x = 1e100;
		sbb.min.y = 1e100;
		sbb.min.z = 1e100;
		sbb.max.x = -1e100;
		sbb.max.y = -1e100;
		sbb.max.z = -1e100;

		// Axis Aligned Bounding Box
		for (int i = 0; i < sh.nbFacet; i++) {
			Facet *f = facets[i];
			if (f->sh.superIdx == viewStruct) {
				for (int j = 0; j < f->sh.nbIndex; j++) {
					Vector3d p = vertices3[f->indices[j]];
					if (p.x < sbb.min.x) sbb.min.x = p.x;
					if (p.y < sbb.min.y) sbb.min.y = p.y;
					if (p.z < sbb.min.z) sbb.min.z = p.z;
					if (p.x > sbb.max.x) sbb.max.x = p.x;
					if (p.y > sbb.max.y) sbb.max.y = p.y;
					if (p.z > sbb.max.z) sbb.max.z = p.z;
				}
			}
		}

#ifdef SYNRAD
		//Regions
		Worker *worker = &(mApp->worker);
		for (int i = 0; i < (int)worker->regions.size(); i++) {
			if (worker->regions[i].AABBmin.x < sbb.min.x) sbb.min.x = worker->regions[i].AABBmin.x;
			if (worker->regions[i].AABBmin.y < sbb.min.y) sbb.min.y = worker->regions[i].AABBmin.y;
			if (worker->regions[i].AABBmin.z < sbb.min.z) sbb.min.z = worker->regions[i].AABBmin.z;
			if (worker->regions[i].AABBmax.x > sbb.max.x) sbb.max.x = worker->regions[i].AABBmax.x;
			if (worker->regions[i].AABBmax.y > sbb.max.y) sbb.max.y = worker->regions[i].AABBmax.y;
			if (worker->regions[i].AABBmax.z > sbb.max.z) sbb.max.z = worker->regions[i].AABBmax.z;
		}
#endif

		return sbb;
	}

}

Vector3d Geometry::GetCenter() {

	if (viewStruct < 0) {

		return center;

	}
	else {

		Vector3d r;
		AABB sbb = GetBB();

		r.x = (sbb.max.x + sbb.min.x) / 2.0;
		r.y = (sbb.max.y + sbb.min.y) / 2.0;
		r.z = (sbb.max.z + sbb.min.z) / 2.0;

		return r;

	}
}

int Geometry::AddRefVertex(InterfaceVertex *p, InterfaceVertex *refs, int *nbRef, double vT) {

	bool found = false;
	int i = 0;
	//Vector3d n;
	double v2 = vT*vT;

	while (i < *nbRef && !found) {
		//Sub(&n,p,refs + i);
		double dx = abs(p->x - (refs + i)->x);
		if (dx < vT) {
			double dy = abs(p->y - (refs + i)->y);
			if (dy < vT) {
				double dz = abs(p->z - (refs + i)->z);
				if (dz < vT) {
					found = (dx*dx + dy*dy + dz*dz < v2);
				}
			}
		}
		if (!found) i++;
	}

	if (!found) {
		// Add a new reference vertex
		refs[*nbRef] = *p;
		*nbRef = *nbRef + 1;
	}

	return i;

}

void Geometry::CollapseVertex(Worker *work, GLProgress *prg, double totalWork, double vT) {
	mApp->changedSinceSave = true;
	if (!isLoaded) return;
	// Collapse neighbor vertices
	InterfaceVertex *refs = (InterfaceVertex *)malloc(sh.nbVertex * sizeof(InterfaceVertex));
	if (!refs) throw Error("Out of memory: CollapseVertex");
	int      *idx = (int *)malloc(sh.nbVertex * sizeof(int));
	if (!idx) throw Error("Out of memory: CollapseVertex");
	int       nbRef = 0;

	// Collapse
	prg->SetMessage("Collapsing vertices...");
	for (int i = 0; !work->abortRequested && i < sh.nbVertex; i++) {
		mApp->DoEvents();  //Catch abort request
		prg->SetProgress(((double)i / (double)sh.nbVertex) / totalWork);
		idx[i] = AddRefVertex(vertices3 + i, refs, &nbRef, vT);
	}

	if (work->abortRequested) {
		delete refs;
		delete idx;
		return;
	}

	// Create the new vertex array
	SAFE_FREE(vertices3);
	vertices3 = (InterfaceVertex *)malloc(nbRef * sizeof(InterfaceVertex));
	if (!vertices3) throw Error("Out of memory: CollapseVertex");
	//UnselectAllVertex();

	memcpy(vertices3, refs, nbRef * sizeof(InterfaceVertex));
	sh.nbVertex = nbRef;

	// Update facets indices
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		prg->SetProgress(((double)i / (double)sh.nbFacet) * 0.05 + 0.45);
		for (int j = 0; j < f->sh.nbIndex; j++)
			f->indices[j] = idx[f->indices[j]];
	}

	free(idx);
	free(refs);

}

bool Geometry::GetCommonEdges(Facet *f1, Facet *f2, size_t * c1, size_t * c2, size_t * chainLength) {

	// Detect common edge between facet
	size_t p11, p12, p21, p22, lgth, si, sj;
	size_t maxLength = 0;

	for (size_t i = 0; i < f1->sh.nbIndex; i++) {

		p11 = f1->GetIndex(i);
		p12 = f1->GetIndex(i + 1);

		for (size_t j = 0; j < f2->sh.nbIndex; j++) {

			p21 = f2->GetIndex(j);
			p22 = f2->GetIndex(j + 1);

			if (p11 == p22 && p12 == p21) {

				// Common edge found
				si = i;
				sj = j;
				lgth = 1;

				// Loop until the end of the common edge chain
				i += 2;
				j -= 1;
				p12 = f1->GetIndex(i);
				p21 = f2->GetIndex(j);
				bool ok = (p12 == p21);
				while (lgth < f1->sh.nbIndex && lgth < f2->sh.nbIndex && ok) {
					p12 = f1->GetIndex(i);
					p21 = f2->GetIndex(j);
					ok = (p12 == p21);
					if (ok) {
						i++; j--;
						lgth++;
					}
				}

				if (lgth > maxLength) {
					*c1 = si;
					*c2 = sj;
					maxLength = lgth;
				}

			}

		}

	}

	if (maxLength > 0) {
		*chainLength = maxLength;
		return true;
	}

	return false;

}

void Geometry::MoveVertexTo(size_t idx, double x, double y, double z) {
	vertices3[idx].x = x;
	vertices3[idx].y = y;
	vertices3[idx].z = z;

}

void Geometry::SwapNormal() {

	if (!IsLoaded()) {
		GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	if (GetNbSelectedFacets() <= 0) return;
	mApp->changedSinceSave = true;
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		if (f->selected) {
			f->SwapNormal();
			InitializeGeometry(i);
			try {
				SetFacetTexture(i, f->tRatio, f->hasMesh);
			}
			catch (Error &e) {
				GLMessageBox::Display((char *)e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
			}
		}
	}

	DeleteGLLists(true, true);
	BuildGLList();

}

void Geometry::Extrude(int mode, Vector3d radiusBase, Vector3d offsetORradiusdir, bool againstNormal, double distanceORradius, double totalAngle, size_t steps) {

	//creates facet from selected vertices

	mApp->changedSinceSave = true;
	auto selectedFacets = GetSelectedFacets();

	for (auto sel:selectedFacets)
	{
			size_t sourceFacetId = sel;
			facets[sourceFacetId]->selected = false;
			//Update direction if necessary
			Vector3d dir2, axisBase, axis;

			if (mode == 1) { //Use facet normal to determine offset
				dir2 = facets[sourceFacetId]->sh.N * distanceORradius;
				if (againstNormal) dir2 = -1.0 * dir2;
			}
			else if (mode == 2) { //Use provided offset
				dir2 = offsetORradiusdir;
			}
			else if (mode == 3) {
				offsetORradiusdir = offsetORradiusdir.Normalized();
				axisBase = radiusBase + offsetORradiusdir * distanceORradius;
				axis = CrossProduct(facets[sourceFacetId]->sh.N, offsetORradiusdir).Normalized();
			}

			//Resize vertex and facet arrays
			size_t nbNewVertices = facets[sourceFacetId]->sh.nbIndex;
			if (mode == 3) nbNewVertices *= (steps);
			vertices3 = (InterfaceVertex*)realloc(vertices3, (sh.nbVertex + nbNewVertices) * sizeof(InterfaceVertex));

			size_t nbNewFacets = facets[sourceFacetId]->sh.nbIndex;
			if (mode == 3) nbNewFacets *= (steps);
			nbNewFacets++; //End cap facet
			facets = (Facet **)realloc(facets, (sh.nbFacet + nbNewFacets) * sizeof(Facet *));

			//create new vertices
			if (mode == 1 || mode == 2) {
				for (size_t j = 0; j < facets[sourceFacetId]->sh.nbIndex; j++) {
					vertices3[sh.nbVertex + j].SetLocation(vertices3[facets[sourceFacetId]->indices[j]] + dir2);
					vertices3[sh.nbVertex + j].selected = false;
				}
			}
			else if (mode == 3) {
				for (size_t step = 0; step < steps; step++) {
					for (size_t j = 0; j < facets[sourceFacetId]->sh.nbIndex; j++) {
						vertices3[sh.nbVertex + step*facets[sourceFacetId]->sh.nbIndex + j].SetLocation(
							Rotate(vertices3[facets[sourceFacetId]->indices[j]], axisBase, axis, (step + 1)*totalAngle*(againstNormal ? -1.0 : 1.0) / (double)steps)
						);
						vertices3[sh.nbVertex + step*facets[sourceFacetId]->sh.nbIndex + j].selected = false;
					}
				}
			}

			//Create end cap
			size_t endCapId = sh.nbFacet + nbNewFacets - 1; //last facet
			facets[endCapId] = new Facet(facets[sourceFacetId]->sh.nbIndex);
			facets[endCapId]->selected = true;
			if (viewStruct != -1) facets[endCapId]->sh.superIdx = viewStruct;
			for (int j = 0; j < facets[sourceFacetId]->sh.nbIndex; j++)
				facets[endCapId]->indices[facets[sourceFacetId]->sh.nbIndex - 1 - j] = sh.nbVertex + j + ((mode == 3) ? (steps - 1)*facets[sourceFacetId]->sh.nbIndex : 0); //assign new vertices to new facet in inverse order

			//Construct sides
			//int direction = 1;
			//if (Dot(&dir2, &facets[sourceFacetId]->sh.N) * distanceORradius < 0.0) direction *= -1; //extrusion towards normal or opposite?
			for (size_t step = 0; step < ((mode == 3) ? steps : 1); step++) {
				for (size_t j = 0; j < facets[sourceFacetId]->sh.nbIndex; j++) {
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j] = new Facet(4);
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->indices[0] = (step == 0) ? facets[sourceFacetId]->indices[j] : sh.nbVertex + (step - 1)*facets[sourceFacetId]->sh.nbIndex + j;
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->indices[1] = sh.nbVertex + j + (step)*facets[sourceFacetId]->sh.nbIndex;
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->indices[2] = sh.nbVertex + (j + 1) % facets[sourceFacetId]->sh.nbIndex + (step)*facets[sourceFacetId]->sh.nbIndex;
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->indices[3] = (step == 0) ? facets[sourceFacetId]->GetIndex(j + 1) : sh.nbVertex + (j + 1) % facets[sourceFacetId]->sh.nbIndex + (step - 1)*facets[sourceFacetId]->sh.nbIndex;
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->selected = true;
					if (viewStruct != -1) facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->sh.superIdx = viewStruct;
				}
			}
			sh.nbVertex += nbNewVertices; //update number of vertices
			sh.nbFacet += nbNewFacets;
	}
	InitializeGeometry();
	mApp->UpdateFacetParams(true);
	UpdateSelection();
	mApp->facetList->SetSelectedRow((int)sh.nbFacet - 1);
	mApp->facetList->ScrollToVisible(sh.nbFacet - 1, 1, false);
}

void Geometry::ShiftVertex() {

	if (!IsLoaded()) {
		GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	if (GetNbSelectedFacets() <= 0) return;
	mApp->changedSinceSave = true;
	DeleteGLLists(true, true);
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		if (f->selected) {
			f->ShiftVertex();
			InitializeGeometry(i);// Reinitialise geom
			try {
				SetFacetTexture(i, f->tRatio, f->hasMesh);
			}
			catch (Error &e) {
				GLMessageBox::Display((char *)e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
			}

		}
	}
	// Delete old resource
	BuildGLList();
}

void Geometry::Merge(size_t nbV, size_t nbF, Vector3d *nV, Facet **nF) {
	mApp->changedSinceSave = true;
	// Merge the current geometry with the specified one
	if (!nbV || !nbF) return;

	// Reallocate mem
	Facet   **nFacets = (Facet **)malloc((sh.nbFacet + nbF) * sizeof(Facet *));
	InterfaceVertex *nVertices3 = (InterfaceVertex *)malloc((sh.nbVertex + nbV) * sizeof(InterfaceVertex));

	if (sh.nbFacet) memcpy(nFacets, facets, sizeof(Facet *) * sh.nbFacet);
	memcpy(nFacets + sh.nbFacet, nF, sizeof(Facet *) * nbF);

	if (sh.nbVertex) memcpy(nVertices3, vertices3, sizeof(InterfaceVertex) * sh.nbVertex);
	memcpy(nVertices3 + sh.nbVertex, nV, sizeof(InterfaceVertex) * nbV);

	SAFE_FREE(facets);
	SAFE_FREE(vertices3);
	facets = nFacets;
	vertices3 = nVertices3;
	//UnselectAllVertex();

	// Shift indices
	for (int i = 0; i < nbF; i++) {
		Facet *f = facets[sh.nbFacet + i];
		for (int j = 0; j < f->sh.nbIndex; j++)
			f->indices[j] += sh.nbVertex;
	}

	sh.nbVertex += nbV;
	sh.nbFacet += nbF;

}

int Geometry::HasIsolatedVertices() {

	// Check if there are unused vertices
	int *check = (int *)malloc(sh.nbVertex * sizeof(int));
	memset(check, 0, sh.nbVertex * sizeof(int));

	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		for (int j = 0; j < f->sh.nbIndex; j++) {
			check[f->indices[j]]++;
		}
	}

	int nbUnused = 0;
	for (int i = 0; i < sh.nbVertex; i++) {
		if (!check[i]) nbUnused++;
	}

	SAFE_FREE(check);
	return nbUnused;

}

void  Geometry::DeleteIsolatedVertices(bool selectedOnly) {
	mApp->changedSinceSave = true;
	// Remove unused vertices
	std::vector<bool> isUsed(sh.nbVertex);

	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		for (int j = 0; j < f->sh.nbIndex; j++) {
			isUsed[f->indices[j]]=true;
		}
	}

	size_t nbUnused = 0;
	for (size_t i = 0; i < sh.nbVertex; i++) {
		if (!isUsed[i] && !(selectedOnly && !vertices3[i].selected)) nbUnused++;
	}

	size_t nbVert = sh.nbVertex - nbUnused;

	if (nbVert == 0) {
		// Remove all
		Clear();
		return;
	}

	// Update facet indices
	std::vector<size_t> newIndex(sh.nbVertex);
	for (size_t i = 0, n = 0; i < sh.nbVertex; i++) {
		if (isUsed[i] || (selectedOnly && !vertices3[i].selected)) {
			newIndex[i] = n;
			n++;
		}
	}
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		for (int j = 0; j < f->sh.nbIndex; j++) {
			f->indices[j] = newIndex[f->indices[j]];
		}
	}

	InterfaceVertex *nVert = (InterfaceVertex *)malloc(nbVert * sizeof(InterfaceVertex));

	for (int i = 0, n = 0; i < sh.nbVertex; i++) {
		if (isUsed[i] || (selectedOnly && !vertices3[i].selected)) {
			nVert[n] = vertices3[i];
			n++;
		}
	}

	SAFE_FREE(vertices3);
	vertices3 = nVert;
	sh.nbVertex = nbVert;
}

void Geometry::Clear() {
	viewStruct = -1; //otherwise a nonexistent structure could stay selected
					 // Free memory
	if (facets) {
		for (int i = 0; i < sh.nbFacet; i++)
			SAFE_DELETE(facets[i]);
		free(facets);
	}
	if (vertices3) free(vertices3);
	for (int i = 0; i < sh.nbSuper; i++) {
		SAFE_FREE(strName[i]);
		SAFE_FREE(strFileName[i]);
	}
	memset(strName, 0, MAX_SUPERSTR * sizeof(char *));
	memset(strFileName, 0, MAX_SUPERSTR * sizeof(char *));
	DeleteGLLists(true, true);

	if (mApp && mApp->splitFacet) mApp->splitFacet->ClearUndoFacets();
	if (mApp && mApp->buildIntersection) mApp->buildIntersection->ClearUndoFacets();
	if (mApp && mApp->mirrorFacet) mApp->mirrorFacet->ClearUndoVertices();
	if (mApp && mApp->mirrorVertex) mApp->mirrorVertex->ClearUndoVertices();

	// Init default
	facets = NULL;         // Facets array
	vertices3 = NULL;      // Facets vertices in (x,y,z) space
	sh.nbFacet = 0;        // Number of facets
	sh.nbVertex = 0;       // Number of vertex
	isLoaded = false;      // isLoaded flag
	sh.nbSuper = 0;          // Structure number
	ResetTextureLimits();
	EmptySelectedVertexList();

	memset(lineList, 0, sizeof(lineList));
	//memset(strName, 0, sizeof(strName));
	//memset(strFileName, 0, sizeof(strFileName));

	// Init OpenGL material
	memset(&whiteMaterial, 0, sizeof(GLMATERIAL));
	whiteMaterial.Diffuse.r = 0.9f;
	whiteMaterial.Diffuse.g = 0.9f;
	whiteMaterial.Diffuse.b = 0.9f;
	whiteMaterial.Ambient.r = 0.9f;
	whiteMaterial.Ambient.g = 0.9f;
	whiteMaterial.Ambient.b = 0.9f;

	memset(&fillMaterial, 0, sizeof(GLMATERIAL));

	fillMaterial.Diffuse.r = 0.6f;
	fillMaterial.Diffuse.g = 0.65f;
	fillMaterial.Diffuse.b = 0.65f;
	fillMaterial.Ambient.r = 0.45f;
	fillMaterial.Ambient.g = 0.41f;
	fillMaterial.Ambient.b = 0.41f;

	memset(&arrowMaterial, 0, sizeof(GLMATERIAL));
	arrowMaterial.Diffuse.r = 0.4f;
	arrowMaterial.Diffuse.g = 0.2f;
	arrowMaterial.Diffuse.b = 0.0f;
	arrowMaterial.Ambient.r = 0.6f;
	arrowMaterial.Ambient.g = 0.3f;
	arrowMaterial.Ambient.b = 0.0f;

	nbSelectedHist = 0;    // Selection history
}

void  Geometry::SelectIsolatedVertices() {

	UnselectAllVertex();
	// Select unused vertices
	int *check = (int *)malloc(sh.nbVertex * sizeof(int));
	memset(check, 0, sh.nbVertex * sizeof(int));

	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		for (int j = 0; j < f->sh.nbIndex; j++) {
			check[f->indices[j]]++;
		}
	}

	for (int i = 0; i < sh.nbVertex; i++) {
		if (!check[i]) vertices3[i].selected = true;
	}

	SAFE_FREE(check);
}

bool Geometry::RemoveCollinear() {
	std::vector<size_t> facetsToDelete;
	for (int i = 0; i < sh.nbFacet; i++)
		if (facets[i]->collinear) facetsToDelete.push_back(i);
	RemoveFacets(facetsToDelete);
	return (facetsToDelete.size() > 0);
}
void Geometry::RemoveSelectedVertex() {

	mApp->changedSinceSave = true;

	//Analyze facets
	std::vector<size_t> facetsToRemove, facetsToChange;
	for (int f = 0; f < sh.nbFacet; f++) {
		int nbSelVertex = 0;
		for (int i = 0; i < facets[f]->sh.nbIndex; i++)
			if (vertices3[facets[f]->indices[i]].selected)
				nbSelVertex++;
		if (nbSelVertex) {
			facetsToChange.push_back(f);
			if ((facets[f]->sh.nbIndex - nbSelVertex) <= 2)
				facetsToRemove.push_back(f);
		}
	}

	for (size_t c = 0; c < facetsToChange.size(); c++) {
		Facet* f = facets[facetsToChange[c]];
		int nbRemove = 0;
		for (size_t i = 0; (int)i < f->sh.nbIndex; i++) //count how many to remove			
			if (vertices3[f->indices[i]].selected)
				nbRemove++;
		size_t *newIndices = (size_t *)malloc((f->sh.nbIndex - nbRemove) * sizeof(size_t));
		int nb = 0;
		for (size_t i = 0; (int)i < f->sh.nbIndex; i++)
			if (!vertices3[f->indices[i]].selected) newIndices[nb++] = f->indices[i];

		SAFE_FREE(f->indices); f->indices = newIndices;
		SAFE_FREE(f->vertices2);
		SAFE_FREE(f->visible);
		f->sh.nbIndex -= nbRemove;
		f->vertices2 = (Vector2d *)malloc(f->sh.nbIndex * sizeof(Vector2d));
		memset(f->vertices2, 0, f->sh.nbIndex * sizeof(Vector2d));
		f->visible = (bool *)malloc(f->sh.nbIndex * sizeof(bool));
		_ASSERTE(f->visible);
		memset(f->visible, 0xFF, f->sh.nbIndex * sizeof(bool));
	}

	RemoveFacets(facetsToRemove);
	DeleteIsolatedVertices(true);
}

void Geometry::RemoveSelected() {
	std::vector<size_t> facetIdList;

	for (int i = 0; i < sh.nbFacet; i++)
		if (facets[i]->selected) facetIdList.push_back(i);
	RemoveFacets(facetIdList);
}

void Geometry::RemoveFacets(const std::vector<size_t> &facetIdList, bool doNotDestroy) {
	if (facetIdList.size() == 0) return;
	mApp->changedSinceSave = true;
	Facet   **f = (Facet **)malloc((sh.nbFacet - facetIdList.size()) * sizeof(Facet *));
	std::vector<bool> facetSelected(sh.nbFacet, false);
	std::vector<int> newRefs(sh.nbFacet, -1);
	for (size_t toRemove : facetIdList) {
		facetSelected[toRemove] = true;
	}

	size_t nb = 0;

	for (size_t i = 0; i < sh.nbFacet; i++) {
		if (facetSelected[i]) {
			if (!doNotDestroy) SAFE_DELETE(facets[i]); //Otherwise it's referenced by an Undo list
		}

		else {
			f[nb] = facets[i];
			newRefs[i] = (int)nb;
			nb++;
		}
	}

	SAFE_FREE(facets);
	facets = f;
	sh.nbFacet = nb;

	mApp->RenumberSelections(newRefs);
	mApp->RenumberFormulas(&newRefs);
	RenumberNeighbors(newRefs);

	// Delete old resources
	DeleteGLLists(true, true);
	BuildGLList();
}

void Geometry::RestoreFacets(std::vector<DeletedFacet> deletedFacetList, bool toEnd) {
	//size_t nbNew = 0;
	std::vector<int> newRefs(sh.nbFacet, -1);
	/*for (auto restoreFacet : deletedFacetList)
		if (restoreFacet.ori_pos >= sh.nbFacet || toEnd) nbNew++;*/
	Facet** tempFacets = (Facet**)malloc(sizeof(Facet*)*(sh.nbFacet + /*nbNew*/ deletedFacetList.size()));
	size_t pos = 0;
	size_t nbInsert = 0;
	if (toEnd) { //insert to end
		for (size_t insertPos = 0; insertPos < sh.nbFacet; insertPos++) { //Original facets
			tempFacets[insertPos] = facets[insertPos];
			newRefs[insertPos] = (int)insertPos;
		}
		for (auto restoreFacet : deletedFacetList) {
			tempFacets[sh.nbFacet + nbInsert] = restoreFacet.f;
			tempFacets[sh.nbFacet + nbInsert]->selected = true;
			nbInsert++;
		}
	}
	else { //Insert to original locations
		for (auto restoreFacet : deletedFacetList) {
			for (size_t insertPos = pos; insertPos < restoreFacet.ori_pos; insertPos++) {
				tempFacets[insertPos] = facets[insertPos - nbInsert];
				newRefs[insertPos - nbInsert] = (int)insertPos;
				pos++;
			}
			//if (restoreFacet.replaceOri) pos--;
			tempFacets[pos] = restoreFacet.f;
			tempFacets[pos]->selected = true;
			pos++;
			if (!restoreFacet.replaceOri) nbInsert++;
		}
		//Remaining facets
		for (size_t insertPos = pos; insertPos < (sh.nbFacet + nbInsert); insertPos++) {
			tempFacets[insertPos] = facets[insertPos - nbInsert];
			newRefs[insertPos - nbInsert] = (int)insertPos;
		}
		_ASSERTE(_CrtCheckMemory());
		//Renumber things;
		RenumberNeighbors(newRefs);
		mApp->RenumberFormulas(&newRefs);
		mApp->RenumberSelections(newRefs);
	}

	sh.nbFacet += nbInsert;
	SAFE_FREE(facets);
	facets = tempFacets;
	InitializeGeometry();
}

int Geometry::ExplodeSelected(bool toMap, int desType, double exponent, double *values) {

	auto selectedFacets = GetSelectedFacets();
	mApp->changedSinceSave = true;
	if (selectedFacets.size() == 0) return -1;

	// Check that all facet has a mesh
	bool ok = true;
	int idx = 0;
	while (ok && idx < sh.nbFacet) {
		if (facets[idx]->selected)
			ok = facets[idx]->hasMesh;
		idx++;
	}
	if (!ok) return -2;

	size_t nb = 0;
	size_t FtoAdd = 0;
	size_t VtoAdd = 0;
	Facet::FACETGROUP *blocks = (Facet::FACETGROUP *)malloc(selectedFacets.size() * sizeof(Facet::FACETGROUP));

	for (auto sel : selectedFacets) {
		facets[sel]->Explode(blocks + nb);
		FtoAdd += blocks[nb].nbF;
		VtoAdd += blocks[nb].nbV;
		nb++;
	}

	// Update vertex array
	InterfaceVertex *ptrVert;
	size_t       vIdx;
	InterfaceVertex *nVert = (InterfaceVertex *)malloc((sh.nbVertex + VtoAdd) * sizeof(InterfaceVertex));
	memcpy(nVert, vertices3, sh.nbVertex * sizeof(InterfaceVertex));

	ptrVert = nVert + sh.nbVertex;
	vIdx = sh.nbVertex;
	nb = 0;
	for (int i = 0; i < sh.nbFacet; i++) {
		if (facets[i]->selected) {
			facets[i]->FillVertexArray(ptrVert);
			for (int j = 0; j < blocks[nb].nbF; j++) {
				for (int k = 0; k < blocks[nb].facets[j]->sh.nbIndex; k++) {
					blocks[nb].facets[j]->indices[k] = vIdx + k;
				}
				vIdx += blocks[nb].facets[j]->sh.nbIndex;
			}
			ptrVert += blocks[nb].nbV;
			nb++;
		}
	}
	SAFE_FREE(vertices3);
	vertices3 = nVert;
	for (size_t i = sh.nbVertex; i < sh.nbVertex + VtoAdd; i++)
		vertices3[i].selected = false;
	sh.nbVertex += VtoAdd;

	// Update facet
	Facet   **f = (Facet **)malloc((sh.nbFacet + FtoAdd - selectedFacets.size()) * sizeof(Facet *));

	auto nbS = selectedFacets.size(); //to remember it after RemveSelected() routine
	// Delete selected facets
	RemoveSelected();

	//Fill old facets
	for (nb = 0; nb < sh.nbFacet; nb++)
		f[nb] = facets[nb];

	// Add new facets
	int count = 0;
	for (int i = 0; i < nbS; i++) {
		for (int j = 0; j < blocks[i].nbF; j++) {
			f[nb++] = blocks[i].facets[j];
#ifdef MOLFLOW
			if (toMap) { //set outgassing values
				f[nb - 1]->sh.outgassing = *(values + count++) *0.100; //0.1: mbar*l/s->Pa*m3/s
				if (f[nb - 1]->sh.outgassing > 0.0) {
					f[nb - 1]->sh.desorbType = desType + 1;
					f[nb - 1]->selected = true;
					if (f[nb - 1]->sh.desorbType == DES_COSINE_N) f[nb - 1]->sh.desorbTypeN = exponent;
				}
				else {
					f[nb - 1]->sh.desorbType = DES_NONE;
					f[nb - 1]->selected = false;
				}
			}
#endif
		}
	}

	// Free allocated memory
	for (int i = 0; i < nbS; i++) {
		SAFE_FREE(blocks[i].facets);
	}
	SAFE_FREE(blocks);

	SAFE_FREE(facets);
	facets = f;
	sh.nbFacet = nb;

	// Delete old resources
	DeleteGLLists(true, true);

	InitializeGeometry();

	return 0;

}

bool Geometry::RemoveNullFacet() {

	// Remove degenerated facet (area~0.0)
	std::vector<size_t> facetsToDelete;

	double areaMin = 1E-10;
	for (int i = 0; i < sh.nbFacet; i++)
		if (facets[i]->sh.area < areaMin) facetsToDelete.push_back(i);
	RemoveFacets(facetsToDelete);

	return (facetsToDelete.size() > 0);
}

void Geometry::AlignFacets(std::vector<size_t> memorizedSelection, size_t sourceFacetId, size_t destFacetId, size_t anchorSourceVertexId, size_t anchorDestVertexId,
	size_t alignerSourceVertexId, size_t alignerDestVertexId, bool invertNormal, bool invertDir1, bool invertDir2, bool copy, Worker *worker) {

	double counter = 0.0;
	GLProgress *prgAlign = new GLProgress("Aligning facets...", "Please wait");
	prgAlign->SetProgress(0.0);
	prgAlign->SetVisible(true);
	if (!mApp->AskToReset(worker)) return;
	//if (copy) CloneSelectedFacets(); //Causes problems
	bool *alreadyMoved = (bool*)malloc(sh.nbVertex * sizeof(bool*));
	memset(alreadyMoved, false, sh.nbVertex * sizeof(bool*));

	//Translating facets to align anchors
	size_t temp;
	if (invertDir1) { //change anchor and direction on source
		temp = alignerSourceVertexId;
		alignerSourceVertexId = anchorSourceVertexId;
		anchorSourceVertexId = temp;
	}
	if (invertDir2) { //change anchor and direction on destination
		temp = alignerDestVertexId;
		alignerDestVertexId = anchorDestVertexId;
		anchorDestVertexId = temp;
	}
	Vector3d Translation = vertices3[anchorDestVertexId] - vertices3[anchorSourceVertexId];

	int nb = 0;
	for (auto sel : memorizedSelection) {
		counter += 0.333;
		prgAlign->SetProgress(counter / (double)memorizedSelection.size());
		for (int j = 0; j < facets[sel]->sh.nbIndex; j++) {
			if (!alreadyMoved[facets[sel]->indices[j]]) {
				vertices3[facets[sel]->indices[j]].SetLocation(vertices3[facets[sel]->indices[j]] + Translation);
				alreadyMoved[facets[sel]->indices[j]] = true;
			}
		}
	}

	SAFE_FREE(alreadyMoved);

	//Rotating to match normal vectors
	Vector3d Axis;
	Vector3d Normal;
	double angle;
	Normal = facets[destFacetId]->sh.N;
	if (invertNormal) Normal = Normal * -1.0;
	Axis = CrossProduct(facets[sourceFacetId]->sh.N, Normal);
	if (Axis.Norme() < 1e-5) { //The two normals are either collinear or the opposite
		if ((Dot(facets[destFacetId]->sh.N, facets[sourceFacetId]->sh.N) > 0.99999 && (!invertNormal)) ||
			(Dot(facets[destFacetId]->sh.N, facets[sourceFacetId]->sh.N) < 0.00001 && invertNormal)) { //no rotation needed
			Axis.x = 1.0;
			Axis.y = 0.0;
			Axis.z = 0.0;
			angle = 0.0;
		}
		else { //180deg rotation needed
			Axis = CrossProduct(facets[sourceFacetId]->sh.U, facets[sourceFacetId]->sh.N).Normalized();
			angle = PI;
		}
	}
	else {
		Axis = Axis.Normalized();
		angle = acos(Dot(facets[sourceFacetId]->sh.N, facets[destFacetId]->sh.N) / (facets[sourceFacetId]->sh.N.Norme() * facets[destFacetId]->sh.N.Norme()));
		//angle = angle / PI * 180;
		if (invertNormal) angle = PI - angle;
	}

	bool *alreadyRotated = (bool*)malloc(sh.nbVertex * sizeof(bool*));
	memset(alreadyRotated, false, sh.nbVertex * sizeof(bool*));

	nb = 0;
	for (auto sel : memorizedSelection) {
		counter += 0.333;
		prgAlign->SetProgress(counter / (double)memorizedSelection.size());
		for (int j = 0; j < facets[sel]->sh.nbIndex; j++) {
			if (!alreadyRotated[facets[sel]->indices[j]]) {
				//rotation comes here
				vertices3[facets[sel]->indices[j]].SetLocation(Rotate(vertices3[facets[sel]->indices[j]], vertices3[anchorDestVertexId], Axis, angle));
				alreadyRotated[facets[sel]->indices[j]] = true;
			}
		}
	}

	SAFE_FREE(alreadyRotated);

	//Rotating to match direction points

	Vector3d Dir1 = vertices3[alignerDestVertexId] - vertices3[anchorDestVertexId];
	Vector3d Dir2 = vertices3[alignerSourceVertexId] - vertices3[anchorSourceVertexId];
	Axis = CrossProduct(Dir2, Dir1);
	if (Axis.Norme() < 1e-5) { //The two directions are either collinear or the opposite
		if (Dot(Dir1, Dir2) > 0.99999) { //no rotation needed
			Axis.x = 1.0;
			Axis.y = 0.0;
			Axis.z = 0.0;
			angle = 0.0;
		}
		else { //180deg rotation needed
			//construct a vector perpendicular to the normal
			Axis = facets[sourceFacetId]->sh.N.Normalized();
			angle = PI;
		}
	}
	else {
		Axis = Axis.Normalized();
		angle = Dot(Dir1, Dir2) / (Dir1.Norme() * Dir2.Norme());
		//bool opposite=(angle<0.0);
		angle = acos(angle);
		//angle = angle / PI * 180;
		//if (invertNormal) angle=180.0-angle;
	}

	bool *alreadyRotated2 = (bool*)malloc(sh.nbVertex * sizeof(bool*));
	memset(alreadyRotated2, false, sh.nbVertex * sizeof(bool*));

	nb = 0;
	for (auto sel : memorizedSelection) {
		counter += 0.333;
		prgAlign->SetProgress(counter / memorizedSelection.size());
		for (int j = 0; j < facets[sel]->sh.nbIndex; j++) {
			if (!alreadyRotated2[facets[sel]->indices[j]]) {
				//rotation comes here
				vertices3[facets[sel]->indices[j]].SetLocation(Rotate(vertices3[facets[sel]->indices[j]], vertices3[anchorDestVertexId], Axis, angle));
				alreadyRotated2[facets[sel]->indices[j]] = true;
			}
		}
	}

	SAFE_FREE(alreadyRotated2);

	InitializeGeometry();
	//update textures
	/*try {
		for (int i = 0; i < nbSelected; i++)
			SetFacetTexture(selection[i], facets[selection[i]]->tRatio, facets[selection[i]]->hasMesh);
	}
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}*/
	prgAlign->SetVisible(false);
	SAFE_DELETE(prgAlign);
}

void Geometry::MoveSelectedFacets(double dX, double dY, double dZ, bool copy, Worker *worker) {

	GLProgress *prgMove = new GLProgress("Moving selected facets...", "Please wait");
	prgMove->SetProgress(0.0);
	prgMove->SetVisible(true);
	auto selectedFacets = GetSelectedFacets();
	if (!(dX == 0.0&&dY == 0.0&&dZ == 0.0)) {
		if (!mApp->AskToReset(worker)) return;
		int nbSelFacet = 0;
		if (copy) CloneSelectedFacets(); //move
		selectedFacets = GetSelectedFacets(); //Update selection to cloned
		double counter = 1.0;
		if (selectedFacets.size() == 0) return;

		bool *alreadyMoved = (bool*)malloc(sh.nbVertex * sizeof(bool*));
		memset(alreadyMoved, false, sh.nbVertex * sizeof(bool*));

		int nb = 0;
		for (auto sel : selectedFacets) {
			counter += 1.0;
			prgMove->SetProgress(counter / selectedFacets.size());
			for (int j = 0; j < facets[sel]->sh.nbIndex; j++) {
				if (!alreadyMoved[facets[sel]->indices[j]]) {
					vertices3[facets[sel]->indices[j]].x += dX;
					vertices3[facets[sel]->indices[j]].y += dY;
					vertices3[facets[sel]->indices[j]].z += dZ;
					alreadyMoved[facets[sel]->indices[j]] = true;
				}
			}
		}

		SAFE_FREE(alreadyMoved);

		InitializeGeometry();
		//update textures
		/*try {
			for (int i = 0; i < sh.nbFacet; i++) if (facets[i]->selected) SetFacetTexture(i, facets[i]->tRatio, facets[i]->hasMesh);
		}
		catch (Error &e) {
			GLMessageBox::Display((char *)e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}*/
	}
	prgMove->SetVisible(false);
	SAFE_DELETE(prgMove);
}

std::vector<UndoPoint> Geometry::MirrorProjectSelectedFacets(Vector3d P0, Vector3d N, bool project, bool copy, Worker *worker) {
	std::vector<UndoPoint> undoPoints;
	double counter = 0.0;
	auto selectedFacets = GetSelectedFacets();
	if (selectedFacets.size() == 0) return undoPoints;
	GLProgress *prgMirror = new GLProgress("Mirroring selected facets...", "Please wait");
	prgMirror->SetProgress(0.0);
	prgMirror->SetVisible(true);

	if (!mApp->AskToReset(worker)) return undoPoints;
	int nbSelFacet = 0;
	if (copy) CloneSelectedFacets();
	selectedFacets = GetSelectedFacets(); //Update selection to cloned
	bool *alreadyMirrored = (bool*)malloc(sh.nbVertex * sizeof(bool*));
	memset(alreadyMirrored, false, sh.nbVertex * sizeof(bool*));

	int nb = 0;
	for (auto sel : selectedFacets) {
		counter += 1.0;
		prgMirror->SetProgress(counter / selectedFacets.size());
		nbSelFacet++;
		for (int j = 0; j < facets[sel]->sh.nbIndex; j++) {
			if (!alreadyMirrored[facets[sel]->indices[j]]) {
				Vector3d newPosition;
				if (project) {
					newPosition = Project(vertices3[facets[sel]->indices[j]], P0, N);
					if (!copy) {
						UndoPoint oriPoint;
						oriPoint.oriPos = vertices3[facets[sel]->indices[j]];
						oriPoint.oriId = facets[sel]->indices[j];
						undoPoints.push_back(oriPoint);
					}
				}
				else {
					//Mirror
					newPosition = Mirror(vertices3[facets[sel]->indices[j]], P0, N);
				}
				vertices3[facets[sel]->indices[j]].SetLocation(newPosition);
				alreadyMirrored[facets[sel]->indices[j]] = true;
			}

		}
	}

	SAFE_FREE(alreadyMirrored);
	if (nbSelFacet == 0) return undoPoints;
	if (!project) SwapNormal();
	InitializeGeometry();
	//update textures
	/*try {
		for (int i = 0; i < sh.nbFacet; i++) if (facets[i]->selected) SetFacetTexture(i, facets[i]->tRatio, facets[i]->hasMesh);
	}
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}*/

	prgMirror->SetVisible(false);
	SAFE_DELETE(prgMirror);
	return undoPoints;
}

std::vector<UndoPoint> Geometry::MirrorProjectSelectedVertices(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, bool project, bool copy, Worker *worker) {
	std::vector<UndoPoint> undoPoints;
	size_t nbVertexOri = sh.nbVertex;
	for (size_t i = 0; i < nbVertexOri; i++) {
		if (vertices3[i].selected) {
			Vector3d newPosition;
			if (!project) {
				newPosition = Mirror(vertices3[i], AXIS_P0, AXIS_DIR);
			}
			else {
				newPosition = Project(vertices3[i], AXIS_P0, AXIS_DIR);
				if (!copy) {
					UndoPoint oriPoint;
					oriPoint.oriPos = vertices3[i];
					oriPoint.oriId = i;
					undoPoints.push_back(oriPoint);
				}
			}
			if (!copy) {
				vertices3[i].SetLocation(newPosition);
			}
			else {
				AddVertex(newPosition);
			}
		}
	}
	InitializeGeometry();
	return undoPoints;
}

void Geometry::RotateSelectedFacets(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, double theta, bool copy, Worker *worker) {

	auto selectedFacets = GetSelectedFacets();
	double counter = 0.0;
	if (selectedFacets.size() == 0) return;
	GLProgress *prgRotate = new GLProgress("Rotating selected facets...", "Please wait");
	prgRotate->SetProgress(0.0);
	prgRotate->SetVisible(true);

	if (theta != 0.0) {
		if (!mApp->AskToReset(worker)) return;
		if (copy) CloneSelectedFacets();
		selectedFacets = GetSelectedFacets(); //Update selection to cloned
		bool *alreadyRotated = (bool*)malloc(sh.nbVertex * sizeof(bool*));
		memset(alreadyRotated, false, sh.nbVertex * sizeof(bool*));

		int nb = 0;
		for (auto sel : selectedFacets) {
			counter += 1.0;
			prgRotate->SetProgress(counter / selectedFacets.size());
			for (int j = 0; j < facets[sel]->sh.nbIndex; j++) {
				if (!alreadyRotated[facets[sel]->indices[j]]) {
					//rotation comes here
					vertices3[facets[sel]->indices[j]].SetLocation(Rotate(vertices3[facets[sel]->indices[j]], AXIS_P0, AXIS_DIR, theta));
					alreadyRotated[facets[sel]->indices[j]] = true;
				}
			}
		}

		SAFE_FREE(alreadyRotated);
		InitializeGeometry();
		//update textures
		/*try {
			for (int i = 0; i < sh.nbFacet; i++) if (facets[i]->selected) SetFacetTexture(i, facets[i]->tRatio, facets[i]->hasMesh);

		}
		catch (Error &e) {
			GLMessageBox::Display((char *)e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}*/

	}
	prgRotate->SetVisible(false);
	SAFE_DELETE(prgRotate);
}

void Geometry::RotateSelectedVertices(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, double theta, bool copy, Worker *worker) {

	if (!copy) { //move
		for (int i = 0; i < sh.nbVertex; i++) {
			if (vertices3[i].selected) {
				vertices3[i].SetLocation(Rotate(vertices3[i], AXIS_P0, AXIS_DIR, theta));
			}
		}
		InitializeGeometry();
	}

	else { //copy
		size_t nbVertexOri = sh.nbVertex;
		for (size_t i = 0; i < nbVertexOri; i++) {
			if (vertices3[i].selected) {
				AddVertex(Rotate(vertices3[i], AXIS_P0, AXIS_DIR, theta));
				//vertices3[i].selected = false; //Unselect original
			}
		}
	}
}

void Geometry::CloneSelectedFacets() { //create clone of selected facets
	auto selectedFacetIds = GetSelectedFacets();
	std::vector<bool> isCopied(sh.nbVertex, false); //we keep log of what has been copied to prevent creating duplicates
	std::vector<InterfaceVertex> newVertices;		//vertices that we create
	std::vector<size_t> newIndices(sh.nbVertex);    //which new vertex was created from this old one

	for (auto sel : selectedFacetIds) {
		for (size_t ind = 0; ind < facets[sel]->sh.nbIndex; ind++) {
			size_t vertexId = facets[sel]->indices[ind];
			if (!isCopied[vertexId]) {
				isCopied[vertexId] = true; //mark as copied
				newIndices[vertexId] = sh.nbVertex + newVertices.size(); //remember clone's index
				newVertices.push_back(InterfaceVertex(vertices3[vertexId])); //create clone
			}
		}
	}

	vertices3 = (InterfaceVertex*)realloc(vertices3, (sh.nbVertex + newVertices.size()) * sizeof(InterfaceVertex)); //Increase vertices3 array

	//fill the new vertices with references to the old ones
	for (size_t newVertexId = 0; newVertexId < newVertices.size(); newVertexId++) {
		vertices3[sh.nbVertex + newVertexId] = newVertices[newVertexId];
	}
	sh.nbVertex += newVertices.size(); //update number of vertices

	facets = (Facet **)realloc(facets, (sh.nbFacet + selectedFacetIds.size()) * sizeof(Facet *)); //make space for new facets
	for (size_t i = 0; i < selectedFacetIds.size(); i++) {
		facets[sh.nbFacet + i] = new Facet(facets[selectedFacetIds[i]]->sh.nbIndex); //create new facets
		facets[sh.nbFacet + i]->CopyFacetProperties(facets[selectedFacetIds[i]], false); //get properties
		//replace indices with clones
		for (size_t j = 0; j < facets[selectedFacetIds[i]]->sh.nbIndex; j++) {
			facets[sh.nbFacet + i]->indices[j] = newIndices[facets[selectedFacetIds[i]]->indices[j]];
		}
		facets[sh.nbFacet + i]->selected = true; //Select new facets
		facets[selectedFacetIds[i]]->selected = false; //Deselect original facets
	}
	sh.nbFacet += selectedFacetIds.size();
}

void Geometry::MoveSelectedVertex(double dX, double dY, double dZ, bool copy, Worker *worker) {

	if (!(dX == 0.0&&dY == 0.0&&dZ == 0.0)) {
		if (!mApp->AskToReset(worker)) return;
		mApp->changedSinceSave = true;
		if (!copy) { //move
			for (int i = 0; i < sh.nbVertex; i++) {
				if (vertices3[i].selected) {
					vertices3[i].x += dX;
					vertices3[i].y += dY;
					vertices3[i].z += dZ;
				}
			}
			InitializeGeometry();
		}

		else { //copy
			size_t nbVertexOri = sh.nbVertex;
			for (size_t i = 0; i < nbVertexOri; i++) {
				if (vertices3[i].selected) {
					AddVertex(vertices3[i].x + dX, vertices3[i].y + dY, vertices3[i].z + dZ);
				}
			}
		}

	}
}

void Geometry::AddVertex(const Vector3d& location, bool selected) {
	mApp->changedSinceSave = true;

	//a new vertex
	sh.nbVertex++;
	InterfaceVertex *verticesNew = (InterfaceVertex *)malloc(sh.nbVertex * sizeof(InterfaceVertex));
	memcpy(verticesNew, vertices3, (sh.nbVertex - 1) * sizeof(InterfaceVertex)); //copy old vertices
	SAFE_FREE(vertices3);
	verticesNew[sh.nbVertex - 1].SetLocation(location);
	verticesNew[sh.nbVertex - 1].selected = selected;
	vertices3 = verticesNew;
}

void Geometry::AddVertex(double X, double Y, double Z, bool selected) {
	AddVertex(Vector3d(X, Y, Z), selected);
}

std::vector<size_t> Geometry::GetSelectedFacets() {
	std::vector<size_t> selection;
	for (size_t i = 0; i < sh.nbFacet; i++)
		if (facets[i]->selected) selection.push_back(i);
	return selection;
}

size_t Geometry::GetNbSelectedFacets()
{
	size_t nb = 0;
	for (size_t i = 0; i < sh.nbFacet; i++)
		if (facets[i]->selected) nb++;
	return nb;
}

void Geometry::SetSelection(std::vector<size_t> selectedFacets, bool isShiftDown, bool isCtrlDown) {
	if (!isShiftDown && !isCtrlDown) UnselectAll(); //Set selection
	for (auto sel : selectedFacets) {
		if (sel < sh.nbFacet) facets[sel]->selected = !isCtrlDown;
	}
	UpdateSelection();
	if (selectedFacets.size()) mApp->facetList->ScrollToVisible(selectedFacets.back(), 0, true); //in facet list, select the last facet of selection group
	mApp->UpdateFacetParams(true);
}

void Geometry::AddStruct(const char *name) {
	strName[sh.nbSuper++] = _strdup(name);
	BuildGLList();
}

void Geometry::DelStruct(int numToDel) {

	RemoveFromStruct(numToDel);
	CheckIsolatedVertex();
	mApp->UpdateModelParams();

	for (int i = 0; i < sh.nbFacet; i++) {
		if (facets[i]->sh.superIdx > numToDel) facets[i]->sh.superIdx--;
		if (facets[i]->sh.superDest > numToDel) facets[i]->sh.superDest--;
	}
	SAFE_FREE(strName[numToDel]);
	SAFE_FREE(strFileName[numToDel]);
	for (int j = numToDel; j < (sh.nbSuper - 1); j++)
	{
		//strName[j] = _strdup(strName[j + 1]);
		strName[j] = strName[j + 1];
		strFileName[j] = strFileName[j + 1];
	}
	sh.nbSuper--;
	BuildGLList();
}

void Geometry::ScaleSelectedVertices(Vector3d invariant, double factorX, double factorY, double factorZ, bool copy, Worker *worker) {

	if (!mApp->AskToReset(worker)) return;
	mApp->changedSinceSave = true;

	size_t nbVertexOri = sh.nbVertex;

	for (size_t i = 0; i < nbVertexOri; i++) {
		if (vertices3[i].selected) {
			Vector3d newPosition;
			newPosition.x = invariant.x + factorX*(vertices3[i].x - invariant.x);
			newPosition.y = invariant.y + factorY*(vertices3[i].y - invariant.y);
			newPosition.z = invariant.z + factorZ*(vertices3[i].z - invariant.z);
			if (!copy) {
				vertices3[i].SetLocation(newPosition); //Move
			}
			else {
				AddVertex(newPosition, true);
			}
		}
	}

	InitializeGeometry();
}

void Geometry::ScaleSelectedFacets(Vector3d invariant, double factorX, double factorY, double factorZ, bool copy, Worker *worker) {

	GLProgress *prgMove = new GLProgress("Scaling selected facets...", "Please wait");
	prgMove->SetProgress(0.0);
	prgMove->SetVisible(true);

	if (!mApp->AskToReset(worker)) return;
	if (copy) CloneSelectedFacets();
	auto selectedFacets = GetSelectedFacets(); //Update selection to cloned
	double counter = 1.0;
	double selected = (double)GetNbSelectedFacets();
	if (selected == 0.0) return;

	bool *alreadyMoved = (bool*)malloc(sh.nbVertex * sizeof(bool*));
	memset(alreadyMoved, false, sh.nbVertex * sizeof(bool*));

	int nb = 0;
	for (auto i:selectedFacets) {
			counter += 1.0;
			prgMove->SetProgress(counter / selected);
			for (int j = 0; j < facets[i]->sh.nbIndex; j++) {
				if (!alreadyMoved[facets[i]->indices[j]]) {
					vertices3[facets[i]->indices[j]].x = invariant.x + factorX*(vertices3[facets[i]->indices[j]].x - invariant.x);
					vertices3[facets[i]->indices[j]].y = invariant.y + factorY*(vertices3[facets[i]->indices[j]].y - invariant.y);
					vertices3[facets[i]->indices[j]].z = invariant.z + factorZ*(vertices3[facets[i]->indices[j]].z - invariant.z);
					alreadyMoved[facets[i]->indices[j]] = true;
				}
			}
	}

	SAFE_FREE(alreadyMoved);

	InitializeGeometry();   

	prgMove->SetVisible(false);
	SAFE_DELETE(prgMove);
}

ClippingVertex::ClippingVertex() {
	visited = false;
	isLink = false;
}

bool operator<(const std::list<ClippingVertex>::iterator& a, const std::list<ClippingVertex>::iterator& b) {
	return (a->distance < b->distance);
}

bool Geometry::IntersectingPlaneWithLine(const Vector3d &P0, const Vector3d &u, const Vector3d &V0, const Vector3d &n, Vector3d *intersectPoint, bool withinSection) {
	//Notations from http://geomalgorithms.com/a05-_intersect-1.html
	//At this point, intersecting ray is L=P0+s*u
	if (IsZero(Dot(n, u))) return false; //Check for parallelness
	Vector3d w = P0 - V0;
	if (IsZero(Dot(n, w))) return false; //Check for inclusion
	//Intersection point: P(s_i)-V0=w+s_i*u -> s_i=(-n*w)/(n*u)
	double s_i = -Dot(n, w) / Dot(n, u);
	if (withinSection && ((s_i < 0) || (s_i > 1.0))) return false;
	//P(s_i)=V0+w+s*u
	*intersectPoint = V0 + w + s_i * u;
	return true;
}

struct IntersectPoint {
	size_t vertexId; //global Id
	size_t withFacetId; //With which facet did it intersect (global Id)
};

struct IntersectFacet {
	size_t id;
	Facet* f;
	//std::vector<std::vector<size_t>> visitedFromThisIndice;
	std::vector<std::vector<IntersectPoint>> intersectionPointId;
	std::vector<IntersectPoint> intersectingPoints; //Intersection points with other facets, not on own edge
};

struct EdgePoint {
	size_t vertexId;
	int onEdge;
	bool visited;
};

std::vector<DeletedFacet> Geometry::BuildIntersection(size_t *nbCreated) {
	mApp->changedSinceSave = true;
	//UnselectAllVertex();
	//std::vector<size_t> result;
	std::vector<InterfaceVertex> newVertices;
	std::vector<IntersectFacet> selectedFacets;
	std::vector<DeletedFacet> deletedFacetList;
	*nbCreated = 0; //Total number of new facets created

	//Populate selected facets
	for (size_t i = 0; i < sh.nbFacet; i++) {
		if (facets[i]->selected) {
			IntersectFacet facet;
			facet.id = i;
			facet.f = facets[i];
			//facet.visitedFromThisIndice.resize(facet.f->sh.nbIndex);
			facet.intersectionPointId.resize(facet.f->sh.nbIndex);
			selectedFacets.push_back(facet);
		}
	}
	for (size_t i = 0; i < selectedFacets.size(); i++) {
		Facet* f1 = selectedFacets[i].f;
		for (size_t j = 0; j < selectedFacets.size(); j++) {
			Facet* f2 = selectedFacets[j].f;
			if (i != j) {
				size_t c1, c2, l;
				if (!GetCommonEdges(f1, f2, &c1, &c2, &l)) {
					for (size_t index = 0; index < f1->sh.nbIndex; index++) { //Go through all indexes of edge-finding facet
						InterfaceVertex intersectionPoint;
						InterfaceVertex base = vertices3[f1->indices[index]];
						Vector3d side = vertices3[f1->GetIndex(index + 1)] - base;

						/*
						//Check if this edge was already checked
						bool found = false;
						for (size_t f_other = 0;found == false && f_other < selectedFacets.size();f_other++) { //Compare with all facets
							if (i != f_other) {
								for (size_t v_other = 0;found == false && v_other < selectedFacets[f_other].f->sh.nbIndex;v_other++) { //Compare with all other facets' all other vertices
									for (size_t visited_v_other = 0;found == false && visited_v_other < selectedFacets[f_other].visitedFromThisIndice[v_other].size();visited_v_other++) //Check if we visited ourselves from an other vertex
										if (selectedFacets[f_other].visitedFromThisIndice[v_other][visited_v_other] == f1->indices[index]) //Found a common point
											found = selectedFacets[f_other].f->indices[v_other] == f1->GetIndex(index + 1);
								}
							}
						}
						if (found) continue; //Skip checking this edge, already checked
						*/
						//selectedFacets[i].visitedFromThisIndice[index].push_back(f1->GetIndex(index + 1));
						if (IntersectingPlaneWithLine(base, side, f2->sh.O, f2->sh.N, &intersectionPoint, true)) {
							Vector2d projected = ProjectVertex(intersectionPoint, f2->sh.U, f2->sh.V, f2->sh.O);
							bool inPoly = IsInPoly(projected.u, projected.v, f2->vertices2, f2->sh.nbIndex);
							bool onEdge = IsOnPolyEdge(projected.u, projected.v, f2->vertices2, f2->sh.nbIndex, 1E-6);
							//onEdge = false;
							if (inPoly || onEdge) {
								//Intersection found. First check if we already created this point
								int foundId = -1;
								for (size_t v = 0; foundId == -1 && v < newVertices.size(); v++) {
									if (IsZero((newVertices[v] - intersectionPoint).Norme()))
										foundId = (int)v;
								}
								IntersectPoint newPoint, newPointOtherFacet;
								if (foundId == -1) { //Register new intersection point
									newPoint.vertexId = newPointOtherFacet.vertexId = sh.nbVertex + newVertices.size();

									intersectionPoint.selected = false;
									newVertices.push_back(intersectionPoint);
								}
								else { //Refer to existing intersection point
									newPoint.vertexId = newPointOtherFacet.vertexId = foundId + sh.nbVertex;
								}
								newPoint.withFacetId = j;
								selectedFacets[i].intersectionPointId[index].push_back(newPoint);
								newPointOtherFacet.withFacetId = i; //With my edge
								if (!onEdge) selectedFacets[j].intersectingPoints.push_back(newPointOtherFacet); //Other facet's plane intersected
							}
						}
					}
				}
			}
		}
	}
	vertices3 = (InterfaceVertex*)realloc(vertices3, sizeof(InterfaceVertex)*(sh.nbVertex + newVertices.size()));
	for (InterfaceVertex vertex : newVertices) {
		vertices3[sh.nbVertex] = vertex;
		//result.push_back(sh.nbVertex);
		sh.nbVertex++;
	}
	UnselectAll();
	for (size_t facetId = 0; facetId < selectedFacets.size(); facetId++) {
		std::vector<std::vector<EdgePoint>> clipPaths;
		Facet *f = selectedFacets[facetId].f;
		for (size_t vertexId = 0; vertexId < selectedFacets[facetId].f->sh.nbIndex; vertexId++) { //Go through indices
			//testPath.push_back(f->indices[vertexId]);
			for (size_t v = 0; v < selectedFacets[facetId].intersectionPointId[vertexId].size(); v++) { //If there are intersection points on this edge, go through them 
				//Check if not the end of an already registered clipping path
				bool found = false;
				for (size_t i = 0; found == false && i < clipPaths.size(); i++) {
					found = clipPaths[i].back().vertexId == selectedFacets[facetId].intersectionPointId[vertexId][v].vertexId;
				}
				if (!found) { //Register a new clip path
					std::vector<EdgePoint> path;
					EdgePoint p;
					p.vertexId = selectedFacets[facetId].intersectionPointId[vertexId][v].vertexId;
					p.onEdge = (int)vertexId;
					path.push_back(p); //Register intersection point
					size_t searchId = selectedFacets[facetId].intersectionPointId[vertexId][v].vertexId; //Current point, by global Id
					size_t searchFacetId = selectedFacets[facetId].intersectionPointId[vertexId][v].withFacetId; // Current facet with which we intersected
					//v++;
					int foundId, foundId2;
					do {
						foundId = -1;
						for (size_t p1 = 0; foundId == -1 && p1 < selectedFacets[facetId].intersectingPoints.size(); p1++) { //Get the next intersection point with same facet
							if (searchId == p1) continue;
							foundId = ((selectedFacets[facetId].intersectingPoints[p1].withFacetId == searchFacetId) && (selectedFacets[facetId].intersectingPoints[p1].vertexId != searchId)) ? (int)p1 : -1;
						}
						if (foundId != -1) {
							EdgePoint p;
							p.vertexId = selectedFacets[facetId].intersectingPoints[foundId].vertexId;
							p.onEdge = -1;
							path.push_back(p);
							//Get next point which is the same
							searchId = selectedFacets[facetId].intersectingPoints[foundId].vertexId;
							searchFacetId = selectedFacets[facetId].intersectingPoints[foundId].withFacetId;
							foundId2 = -1;
							for (size_t p2 = 0; foundId2 == -1 && p2 < selectedFacets[facetId].intersectingPoints.size(); p2++) { //Search next intersection point which is same vertex
								if (p2 == foundId) continue;
								foundId2 = ((selectedFacets[facetId].intersectingPoints[p2].vertexId == searchId) && (selectedFacets[facetId].intersectingPoints[p2].withFacetId != searchFacetId)) ? (int)p2 : -1;
							}
							if (foundId2 != -1) {
								searchFacetId = selectedFacets[facetId].intersectingPoints[foundId2].withFacetId;
								searchId = foundId2;
							}
						}
					} while (foundId != -1 && foundId2 != -1);
					//No more intersection points on the middle of the facet. Need to find closing point, which is on an edge
					foundId = -1;
					for (size_t v2 = v; foundId == -1 && v2 < selectedFacets[facetId].intersectionPointId[vertexId].size(); v2++) { //Check if on same edge
						if (selectedFacets[facetId].intersectionPointId[vertexId][v2].withFacetId == searchFacetId && selectedFacets[facetId].intersectionPointId[vertexId][v2].vertexId != path.front().vertexId) foundId = (int)selectedFacets[facetId].intersectionPointId[vertexId][v2].vertexId;
					}
					if (foundId != -1) {
						EdgePoint p;
						p.vertexId = foundId;
						p.onEdge = (int)vertexId;
						path.push_back(p); //Found on same edge, close
					}
					else { //Search on other edges
						for (size_t v3 = 0; foundId == -1 && v3 < selectedFacets[facetId].f->sh.nbIndex; v3++) {
							if (v3 == vertexId) continue; //Already checked on same edge
							for (size_t v2 = v; foundId == -1 && v2 < selectedFacets[facetId].intersectionPointId[v3].size(); v2++) {
								if (selectedFacets[facetId].intersectionPointId[v3][v2].withFacetId == searchFacetId) {
									foundId = (int)selectedFacets[facetId].intersectionPointId[v3][v2].vertexId;
									EdgePoint p;
									p.vertexId = foundId;
									p.onEdge = (int)v3;
									path.push_back(p);
								}
							}
						}
					}
					if (path.size() > 1) clipPaths.push_back(path);
				}
			}
		}

		if (clipPaths.size() > 0) {
			//Construct clipped facet, having a selected vertex
			std::vector<bool> isIndexSelected(f->sh.nbIndex);
			for (size_t v = 0; v < f->sh.nbIndex; v++) {
				isIndexSelected[v] = vertices3[f->indices[v]].selected; //Make a copy, we don't want to deselect vertices
			}
			size_t nbNewfacet = 0; //Number of new facets created for this particular clipping path
			for (size_t v = 0; v < f->sh.nbIndex; v++) {
				size_t currentVertex = v;
				if (isIndexSelected[currentVertex]) {
					//Restore visited state for all clip paths
					for (size_t i = 0; i < clipPaths.size(); i++) {
						for (size_t j = 0; j < clipPaths[i].size(); j++) {
							clipPaths[i][j].visited = false;
						}
					}
					std::vector<size_t> clipPath;
					nbNewfacet++;
					do { //Build points of facet
						clipPath.push_back(f->indices[currentVertex]);
						isIndexSelected[currentVertex] = false;
						//Get closest path end
						double minDist = 9E99;
						int clipId = -1;
						bool front;

						for (size_t i = 0; i < clipPaths.size(); i++) {
							if (clipPaths[i].front().onEdge == currentVertex && clipPaths[i].back().onEdge != -1) { //If a full clipping path is found on the scanned edge, go through it
								double d = (vertices3[f->indices[currentVertex]] - vertices3[clipPaths[i].front().vertexId]).Norme();
								if (d < minDist) {
									minDist = d;
									clipId = (int)i;
									front = true;
								}
							}
							if (clipPaths[i].back().onEdge == currentVertex && clipPaths[i].front().onEdge != -1) { //If a full clipping path is found on the scanned edge, go through it
								double d = (vertices3[f->indices[currentVertex]] - vertices3[clipPaths[i].back().vertexId]).Norme();
								if (d < minDist) {
									minDist = d;
									clipId = (int)i;
									front = false;
								}
							}
						}
						if (clipId != -1) {
							if ((front && !clipPaths[clipId].front().visited) || (!front && !clipPaths[clipId].back().visited)) {
								for (int cp = front ? 0 : (int)clipPaths[clipId].size() - 1; cp >= 0 && cp < (int)clipPaths[clipId].size(); cp += front ? 1 : -1) {
									clipPath.push_back(clipPaths[clipId][cp].vertexId);
									clipPaths[clipId][cp].visited = true;
								}
								currentVertex = front ? clipPaths[clipId].back().onEdge : clipPaths[clipId].front().onEdge;
							}
						}
						currentVertex = (currentVertex + 1) % f->sh.nbIndex;
					} while (currentVertex != v);
					if (clipPath.size() > 2) {
						Facet *f = new Facet(clipPath.size());
						for (size_t i = 0; i < clipPath.size(); i++)
							f->indices[i] = clipPath[i];
						f->selected = true;

						if (nbNewfacet == 1) {//replace original
							DeletedFacet df;
							df.f = facets[selectedFacets[facetId].id];
							df.ori_pos = selectedFacets[facetId].id;
							df.replaceOri = true;
							deletedFacetList.push_back(df);
							facets[selectedFacets[facetId].id] = f; //replace original
						}
						else {//create new
							facets = (Facet**)realloc(facets, sizeof(Facet*)*(sh.nbFacet + 1));
							facets[sh.nbFacet++] = f;
							(*nbCreated)++;
						}
					}
				}
			}
		}
		/*
		//Clip facet
		std::vector<std::vector<size_t>> clippingPaths;
		for (auto path : clipPaths) {
			std::vector<size_t> newPath;
			for (auto point : path) {
				newPath.push_back(point.vertexId);
			}
			clippingPaths.push_back(newPath);
		}
		ClipPolygon(selectedFacets[facetId].id, clippingPaths,ClipperLib::ctIntersection);
		*/
	}

	//Rebuild facet
	/*
	f->sh.nbIndex = (int)testPath.size();
	f->indices = (int*)realloc(f->indices, sizeof(int)*testPath.size());
	f->vertices2 = (Vector2d*)realloc(f->vertices2, sizeof(Vector2d)*testPath.size());
	f->visible = (bool*)realloc(f->visible, sizeof(bool)*testPath.size());
	for (size_t i = 0; i < testPath.size(); i++)
		f->indices[i] = testPath[i];
	Rebuild();
	*/

	/*for (auto path : clipPaths)
		for (auto vertexId : path)
			vertices3[vertexId].selected = true;*/

	Rebuild();
	return deletedFacetList;
}

std::vector<DeletedFacet> Geometry::SplitSelectedFacets(const Vector3d &base, const Vector3d &normal, size_t *nbCreated,/*Worker *worker,*/GLProgress *prg) {
	mApp->changedSinceSave = true;
	std::vector<DeletedFacet> deletedFacetList;
	size_t oldNbFacets = sh.nbFacet;
	for (size_t i = 0; i < oldNbFacets; i++) {
		Facet *f = facets[i];
		if (f->selected) {
			if (prg) prg->SetProgress(double(i) / double(sh.nbFacet));
			Vector3d intersectionPoint, intersectLineDir;
			if (!IntersectingPlaneWithLine(f->sh.O, f->sh.U, base, normal, &intersectionPoint))
				if (!IntersectingPlaneWithLine(f->sh.O, f->sh.V, base, normal, &intersectionPoint))
					if (!IntersectingPlaneWithLine(f->sh.O + f->sh.U, -1.0*f->sh.U, base, normal, &intersectionPoint)) //If origin on cutting plane
						if (!IntersectingPlaneWithLine(f->sh.O + f->sh.V, -1.0*f->sh.V, base, normal, &intersectionPoint)) //If origin on cutting plane
						{
							f->selected = false;
							continue;
						}
			//Reduce to a 2D problem in the facet's plane
			Vector2d intPoint2D, intDir2D, intDirOrt2D;
			intPoint2D = ProjectVertex(intersectionPoint, f->sh.U, f->sh.V, f->sh.O);
			intersectLineDir = CrossProduct(normal, f->sh.N);
			intDir2D.u = Dot(f->sh.U, intersectLineDir) / Dot(f->sh.U, f->sh.U);
			intDir2D.v = Dot(f->sh.V, intersectLineDir) / Dot(f->sh.V, f->sh.V);
			//Construct orthogonal vector to decide inside/outside
			intDirOrt2D.u = intDir2D.v;
			intDirOrt2D.v = -intDir2D.u;
			//Do the clipping. Algorithm following pseudocode from Graphic Gems V: "Clipping a Concave Polygon", Andrew S. Glassner
			std::list<ClippingVertex> clipVertices;
			//Assure that we begin on either side of the clipping line
			int currentPos;
			size_t startIndex = 0;
			do {
				double a = Dot(intDirOrt2D, f->vertices2[startIndex] - intPoint2D);
				if (a > 1E-10) {
					currentPos = 1;
				}
				else if (a < -1E-10) {
					currentPos = -1;
				}
				else {
					currentPos = 0;
				}
				startIndex++;
			} while (startIndex < f->sh.nbIndex && currentPos == 0);

			if (startIndex == f->sh.nbIndex) continue; //Whole null facet on clipping line...

			startIndex--; //First vertex not on clipping line
			bool areWeInside = (currentPos == 1);

			size_t v = startIndex;
			do { //Make a circle on the facet
				ClippingVertex V;
				V.vertex = f->vertices2[v];
				V.globalId = f->indices[v];

				double a = Dot(intDirOrt2D, V.vertex - intPoint2D);
				if (a > 1E-10) {
					V.inside = areWeInside = true;
					V.onClippingLine = false;
				}
				else if (a < -1E-10) {
					V.inside = areWeInside = false;
					V.onClippingLine = false;
				}
				else {
					V.inside = areWeInside; //Previous point
					V.onClippingLine = true;
				}
				clipVertices.push_back(V);
				v = (v + 1) % f->sh.nbIndex;
			} while (v != startIndex);
			//At this point the original vertices are prepared
			std::list<std::list<ClippingVertex>::iterator> createdList; //Will contain the clipping points
			std::list<ClippingVertex>::iterator V = clipVertices.begin();
			size_t nbNewPoints = 0;
			do {
				std::list<ClippingVertex>::iterator N = std::next(V);
				if (N == clipVertices.end()) N = clipVertices.begin();
				if (V->inside != N->inside) { //side change, or leaving (not arriving to!) clipping line
					if (V->onClippingLine) {
						createdList.push_back(V); //Just mark V as clipping point, no new vertex required
					}
					else  //New vertex
					{
						//Compute location of intersection point P
						Vector2d v = N->vertex - V->vertex;
						Vector2d w = intPoint2D - V->vertex;
						double s_i = (v.v*w.u - v.u*w.v) / (v.u*intDir2D.v - v.v*intDir2D.u);
						ClippingVertex P;
						P.vertex = intPoint2D + intDir2D*s_i;
						P.globalId = sh.nbVertex + nbNewPoints;
						nbNewPoints++;
						createdList.push_back(clipVertices.insert(N, P)); //Insert P in clippingVertices between V and N
						V++; //iterate to P or end of list
					}
				}
				if (V != clipVertices.end()) V++; //iterate to N
			} while (V != clipVertices.end());
			//Register new vertices and calc distance from clipping line
			if (createdList.size() > 0) { //If there was a cut
				_ASSERTE(createdList.size() % 2 == 0);
				if (nbNewPoints) vertices3 = (InterfaceVertex*)realloc(vertices3, sizeof(InterfaceVertex)*(sh.nbVertex + nbNewPoints));
				for (std::list<std::list<ClippingVertex>::iterator>::iterator newVertexIterator = createdList.begin(); newVertexIterator != createdList.end(); newVertexIterator++) {
					if ((*newVertexIterator)->globalId >= sh.nbVertex) {
						InterfaceVertex newCoord3D;
						newCoord3D.SetLocation(f->sh.O + f->sh.U*(*newVertexIterator)->vertex.u + f->sh.V*(*newVertexIterator)->vertex.v);
						newCoord3D.selected = false;
						vertices3[sh.nbVertex] = newCoord3D;
						sh.nbVertex++;
					}
					Vector2d diff = (*newVertexIterator)->vertex - intPoint2D;
					(*newVertexIterator)->distance = Dot(diff, intDir2D);
				}
				createdList.sort();
				for (std::list<std::list<ClippingVertex>::iterator>::iterator pairFirst = createdList.begin(); pairFirst != createdList.end(); pairFirst++, pairFirst++) {
					std::list<std::list<ClippingVertex>::iterator>::iterator pairSecond = std::next(pairFirst);
					(*pairFirst)->isLink = (*pairSecond)->isLink = true;
					(*pairFirst)->link = *pairSecond;
					(*pairSecond)->link = *pairFirst;
				}
				std::list<ClippingVertex>::iterator U = clipVertices.begin();
				std::list<std::vector<size_t>> newFacetsIndices;
				do {
					std::vector<size_t> newPolyIndices;
					if (U->visited == false) {
						std::list<ClippingVertex>::iterator V = U;
						do {
							V->visited = true;
							newPolyIndices.push_back(V->globalId);
							if (V->isLink) {
								V = V->link;
								V->visited = true;
								newPolyIndices.push_back(V->globalId);
							}
							V++; if (V == clipVertices.end()) V = clipVertices.begin();
						} while (V != U);
					}
					U++;
					//Register new facet
					if (newPolyIndices.size() > 0) {
						newFacetsIndices.push_back(newPolyIndices);
					}
				} while (U != clipVertices.end());
				if (newFacetsIndices.size() > 0)
					facets = (Facet**)realloc(facets, sizeof(Facet*)*(sh.nbFacet + newFacetsIndices.size()));
				for (auto newPolyIndices : newFacetsIndices) {
					_ASSERTE(newPolyIndices.size() >= 3);
					Facet *newFacet = new Facet((int)newPolyIndices.size());
					(*nbCreated)++;
					for (size_t i = 0; i < newPolyIndices.size(); i++) {
						newFacet->indices[i] = newPolyIndices[i];
					}
					newFacet->CopyFacetProperties(f); //Copy physical parameters, structure, etc. - will cause problems with outgassing, though
					CalculateFacetParam(newFacet);
					/*if (f->sh.area > 0.0) {*/
					if (Dot(f->sh.N, newFacet->sh.N) < 0) {
						newFacet->SwapNormal();
					}
					newFacet->selected = true;
					facets[sh.nbFacet] = newFacet;
					sh.nbFacet++;
					/*}*/
				}
				DeletedFacet df;
				df.ori_pos = i;
				df.f = f; //Keep the pointer in memory
				df.replaceOri = false;
				deletedFacetList.push_back(df);
			} //end if there was a cut
		}
		f->selected = false;
	}

	std::vector<size_t> deletedFacetIds;
	for (auto deletedFacet : deletedFacetList)
		deletedFacetIds.push_back(deletedFacet.ori_pos);
	RemoveFacets(deletedFacetIds, true); //We just renumber, keeping the facets in memory

	// Delete old resources
	DeleteGLLists(true, true);
	InitializeGeometry();
	return deletedFacetList;
}

Facet *Geometry::MergeFacet(Facet *f1, Facet *f2) {
	mApp->changedSinceSave = true;
	// Merge 2 facets into 1 when possible and create a new facet
	// otherwise return NULL.
	size_t  c1,c2,l;
	bool end = false;
	Facet *nF = NULL;

	if (GetCommonEdges(f1, f2, &c1, &c2, &l)) {
		size_t commonNo = f1->sh.nbIndex + f2->sh.nbIndex - 2 * l;
		if (commonNo == 0) { //two identical facets, so return a copy of f1
			nF = new Facet(f1->sh.nbIndex);
			nF->CopyFacetProperties(f1);
			for (int i = 0; i < f1->sh.nbIndex; i++)
				nF->indices[i] = f1->GetIndex(i);
			return nF;
		}

		int nbI = 0;
		nF = new Facet(commonNo);
		// Copy params from f1
		//nF->CopyFacetProperties(f1);
		nF->CopyFacetProperties(f1);

		if (l == f1->sh.nbIndex) {

			// f1 absorbed, copy indices from f2
			for (int i = 0; i < f2->sh.nbIndex - l; i++)
				nF->indices[nbI++] = f2->GetIndex(c2 + 2 + i);

		}
		else if (l == f2->sh.nbIndex) {

			// f2 absorbed, copy indices from f1
			for (int i = 0; i < f1->sh.nbIndex - l; i++)
				nF->indices[nbI++] = f1->GetIndex(c1 + l + i);

		}
		else {

			// Copy indices from f1
			for (int i = 0; i < f1->sh.nbIndex - (l - 1); i++)
				nF->indices[nbI++] = f1->GetIndex(c1 + l + i);
			// Copy indices from f2
			for (int i = 0; i < f2->sh.nbIndex - (l + 1); i++)
				nF->indices[nbI++] = f2->GetIndex(c2 + 2 + i);

		}

	}

	return nF;

}

void Geometry::Collapse(double vT, double fT, double lT, bool doSelectedOnly, Worker *work, GLProgress *prg) {
	mApp->changedSinceSave = true;
	work->abortRequested = false;
	Facet *fi, *fj;
	Facet *merged;

	double totalWork = (1.0 + (double)(fT > 0.0) + (double)(lT > 0.0)); //for progress indicator
																	  // Collapse vertex
	if (vT > 0.0) {
		CollapseVertex(work, prg, totalWork, vT);
		InitializeGeometry(); //Find collinear facets
		if (RemoveCollinear() || RemoveNullFacet()) InitializeGeometry(); //If  facets were removed, update geom.
	}

	if (fT > 0.0 && !work->abortRequested) {

		// Collapse facets
		prg->SetMessage("Collapsing facets...");
		std::vector<int> newRef(sh.nbFacet);
		for (int i = 0; i < sh.nbFacet; i++) {
			newRef[i] = i; //Default: reference doesn't change
		}
		for (int i = 0; !work->abortRequested && i < sh.nbFacet; i++) {
			prg->SetProgress((1.0 + ((double)i / (double)sh.nbFacet)) / totalWork);
			mApp->DoEvents(); //To catch eventual abort button click
			fi = facets[i];
			// Search a coplanar facet
			int j = i + 1;
			while ((!doSelectedOnly || fi->selected) && j < sh.nbFacet) {
				fj = facets[j];
				merged = NULL;
				if ((!doSelectedOnly || fj->selected) && fi->IsCoplanarAndEqual(fj, fT)) {
					// Collapse
					merged = MergeFacet(fi, fj);

					if (merged) {
						// Replace the old 2 facets by the new one
						SAFE_DELETE(fi);
						SAFE_DELETE(fj);
						newRef[i] = newRef[j] = -1;
						for (int k = j; k < sh.nbFacet - 1; k++) {
							facets[k] = facets[k + 1];
						}
						for (int k = j + 1; k < newRef.size(); k++) {
							newRef[k] --; //Renumber references
						}
						sh.nbFacet--;
						facets[i] = merged;
						//InitializeGeometry(i);
						//SetFacetTexture(i,facets[i]->tRatio,facets[i]->hasMesh);  //rebuild mesh
						fi = facets[i];
						j = i + 1;

					}
				}
				if (!merged) j++;
			}
		}
		mApp->RenumberSelections(newRef);
		mApp->RenumberFormulas(&newRef);
		RenumberNeighbors(newRef);
	}
	//Collapse collinear sides. Takes some time, so only if threshold>0
	prg->SetMessage("Collapsing collinear sides...");
	if (lT > 0.0 && !work->abortRequested) {
		for (int i = 0; i < sh.nbFacet; i++) {
			prg->SetProgress((1.0 + (double)(fT > 0.0) + ((double)i / (double)sh.nbFacet)) / totalWork);
			if (!doSelectedOnly || facets[i]->selected)
				MergecollinearSides(facets[i], lT);
		}
	}
	prg->SetMessage("Rebuilding geometry...");
	for (int i = 0; i < sh.nbFacet; i++) {

		Facet *f = facets[i];

		// Revert orientation if normal has been swapped
		// This happens when the second vertex is no longer convex
		Vector3d n, v1, v2;
		double   d;
		size_t i0 = facets[i]->indices[0];
		size_t i1 = facets[i]->indices[1];
		size_t i2 = facets[i]->indices[2];

		v1 = vertices3[i1] - vertices3[i0]; // v1 = P0P1
		v2 = vertices3[i2] - vertices3[i1]; // v2 = P1P2
		n = CrossProduct(v1, v2);           // Cross product
		d = Dot(n, f->sh.N);
		if (d < 0.0) f->SwapNormal();

	}

	// Delete old resources
	for (int i = 0; i < sh.nbSuper; i++)
		DeleteGLLists(true, true);

	// Reinitialise geom
	InitializeGeometry();

}

void Geometry::RenumberNeighbors(const std::vector<int> &newRefs) {
	for (size_t i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		for (size_t j = 0; j < f->neighbors.size(); j++) {
			size_t oriId = f->neighbors[j].id;
			if (oriId >= newRefs.size()) { //Refers to a facet that didn't exist even before
				f->neighbors.erase(f->neighbors.begin() + j);
				j--; //Do this index again as it's now the next
			}
			else if (newRefs[oriId] == -1) { //Refers to a facet that we just deleted now
				f->neighbors.erase(f->neighbors.begin() + j);
				j--;  //Do this index again as it's now the next
			}
			else { //Update id
				f->neighbors[j].id = newRefs[oriId];
			}
		}
	}
}

void Geometry::MergecollinearSides(Facet *f, double lT) {
	mApp->changedSinceSave = true;
	bool collinear;
	double linTreshold = cos(lT*PI / 180);
	// Merge collinear sides
	for (int k = 0; (k < f->sh.nbIndex&&f->sh.nbIndex>3); k++) {
		k = k;
		do {
			//collinear=false;
			size_t p0 = f->indices[k];
			size_t p1 = f->indices[(k + 1) % f->sh.nbIndex];
			size_t p2 = f->indices[(k + 2) % f->sh.nbIndex]; //to compare last side with first too
			Vector3d p0p1 = (vertices3[p1] - vertices3[p0]).Normalized();
			Vector3d p0p2 = (vertices3[p2] - vertices3[p1]).Normalized();
			collinear = (Dot(p0p1, p0p2) >= linTreshold);
			if (collinear&&f->sh.nbIndex > 3) { //collinear
				for (int l = (k + 1) % f->sh.nbIndex; l < f->sh.nbIndex - 1; l++) {
					f->indices[l] = f->indices[l + 1];
				}
				f->sh.nbIndex--;
			}
		} while (collinear&&f->sh.nbIndex > 3);
	}
}

void Geometry::CalculateFacetParam(Facet* f) {
	// Calculate facet normal
	Vector3d p0 = vertices3[f->indices[0]];
	Vector3d v1;
	Vector3d v2;
	bool consecutive = true;
	int ind = 2;

	// TODO: Handle possible collinear consequtive vectors
	size_t i0 = f->indices[0];
	size_t i1 = f->indices[1];
	while (ind < f->sh.nbIndex && consecutive) {
		size_t i2 = f->indices[ind++];

		v1 = vertices3[i1] - vertices3[i0]; // v1 = P0P1
		v2 = vertices3[i2] - vertices3[i1]; // v2 = P1P2
		f->sh.N = CrossProduct(v1, v2);              // Cross product
		consecutive = (f->sh.N.Norme() < 1e-11);
	}
	f->collinear = consecutive; //mark for later that this facet was on a line
	f->sh.N = f->sh.N.Normalized();                  // Normalize

											// Calculate Axis Aligned Bounding Box
	f->sh.bb.min.x = 1e100;
	f->sh.bb.min.y = 1e100;
	f->sh.bb.min.z = 1e100;
	f->sh.bb.max.x = -1e100;
	f->sh.bb.max.y = -1e100;
	f->sh.bb.max.z = -1e100;

	for (size_t i = 0; i < f->sh.nbIndex; i++) {
		Vector3d p = vertices3[f->indices[i]];
		if (p.x < f->sh.bb.min.x) f->sh.bb.min.x = p.x;
		if (p.y < f->sh.bb.min.y) f->sh.bb.min.y = p.y;
		if (p.z < f->sh.bb.min.z) f->sh.bb.min.z = p.z;
		if (p.x > f->sh.bb.max.x) f->sh.bb.max.x = p.x;
		if (p.y > f->sh.bb.max.y) f->sh.bb.max.y = p.y;
		if (p.z > f->sh.bb.max.z) f->sh.bb.max.z = p.z;
	}

	// Facet center (AABB center)
	f->sh.center.x = (f->sh.bb.max.x + f->sh.bb.min.x) / 2.0;
	f->sh.center.y = (f->sh.bb.max.y + f->sh.bb.min.y) / 2.0;
	f->sh.center.z = (f->sh.bb.max.z + f->sh.bb.min.z) / 2.0;

	// Plane equation
	double A = f->sh.N.x;
	double B = f->sh.N.y;
	double C = f->sh.N.z;
	double D = -Dot(f->sh.N, p0);

	// Facet planeity
	size_t nb = f->sh.nbIndex;
	double max = 0.0;
	for (size_t i = 1; i < nb; i++) {
		Vector3d p = vertices3[f->indices[i]];
		double d = A * p.x + B * p.y + C * p.z + D;
		if (fabs(d) > fabs(max)) max = d;
	}

	// Plane coef
	f->a = A;
	f->b = B;
	f->c = C;
	f->d = D;
	f->err = max;

	//new part copied from InitGeometry

	//Vector3d p0 = vertices3[f->indices[0]];
	Vector3d p1 = vertices3[f->indices[1]];

	Vector3d U, V;

	U.x = p1.x - p0.x;
	U.y = p1.y - p0.y;
	U.z = p1.z - p0.z;

	U = U.Normalized();
	// Construct a normal vector V:
	V = CrossProduct(f->sh.N, U); // |U|=1 and |N|=1 => |V|=1

							   // u,v vertices (we start with p0 at 0,0)
	f->vertices2[0].u = 0.0;
	f->vertices2[0].v = 0.0;
	Vector2d BBmin; BBmin.u = 0.0; BBmin.v = 0.0;
	Vector2d BBmax; BBmax.u = 0.0; BBmax.v = 0.0;

	for (size_t j = 1; j < f->sh.nbIndex; j++) {

		Vector3d p = vertices3[f->indices[j]];
		Vector3d v = p - p0;
		f->vertices2[j].u = Dot(U, v);  // Project p on U along the V direction
		f->vertices2[j].v = Dot(V, v);  // Project p on V along the U direction

										  // Bounds
		if (f->vertices2[j].u > BBmax.u) BBmax.u = f->vertices2[j].u;
		if (f->vertices2[j].v > BBmax.v) BBmax.v = f->vertices2[j].v;
		if (f->vertices2[j].u < BBmin.u) BBmin.u = f->vertices2[j].u;
		if (f->vertices2[j].v < BBmin.v) BBmin.v = f->vertices2[j].v;

	}

	// Calculate facet area (Meister/Gauss formula)
	double area = 0.0;
	for (size_t j = 0; j < f->sh.nbIndex; j++) {
		size_t j1 = IDX(j + 1,f->sh.nbIndex);
		area += f->vertices2[j].u*f->vertices2[j1].v - f->vertices2[j1].u*f->vertices2[j].v;
	}
	f->sh.area = fabs(0.5 * area);

	// Compute the 2D basis (O,U,V)
	double uD = (BBmax.u - BBmin.u);
	double vD = (BBmax.v - BBmin.v);

	// Origin
	f->sh.O.x = BBmin.u * U.x + BBmin.v * V.x + p0.x;
	f->sh.O.y = BBmin.u * U.y + BBmin.v * V.y + p0.y;
	f->sh.O.z = BBmin.u * U.z + BBmin.v * V.z + p0.z;

	// Rescale U and V vector
	f->sh.nU = U;
	U = U * uD;
	f->sh.U = U;

	f->sh.nV = V;
	V = V * vD;
	f->sh.V = V;

	//Center might not be on the facet's plane
	Vector2d projectedCenter = ProjectVertex(f->sh.center, f->sh.U, f->sh.V, f->sh.O);
	f->sh.center = f->sh.O + projectedCenter.u*f->sh.U + projectedCenter.v*f->sh.V;

	f->sh.Nuv = CrossProduct(U, V);

	// Rescale u,v coordinates
	for (int j = 0; j < f->sh.nbIndex; j++) {

		Vector2d p = f->vertices2[j];
		f->vertices2[j].u = (p.u - BBmin.u) / uD;
		f->vertices2[j].v = (p.v - BBmin.v) / vD;

	}
#ifdef MOLFLOW
	f->sh.maxSpeed = 4.0 * sqrt(2.0*8.31*f->sh.temperature / 0.001 / mApp->worker.gasMass);
#endif
}

void Geometry::RemoveFromStruct(int numToDel) {
	std::vector<size_t> facetsToDelete;
	for (int i = 0; i < sh.nbFacet; i++)
		if (facets[i]->sh.superIdx == numToDel) facetsToDelete.push_back(i);
	RemoveFacets(facetsToDelete);
}

void Geometry::CreateLoft() {
	struct loftIndex {
		size_t index;
		bool visited;
		bool boundary;
	};
	//Find first two selected facets
	int nbFound = 0;
	Facet *f1, *f2;
	for (size_t i = 0; nbFound < 2 && i < sh.nbFacet; i++) {
		if (facets[i]->selected) {
			if (nbFound == 0) f1 = facets[i];
			else f2 = facets[i];
			facets[i]->selected = false;
			nbFound++;
		}
	}
	if (!(nbFound == 2)) return;
	int incrementDir = (Dot(f1->sh.N, f2->sh.N) > 0) ? -1 : 1;

	std::vector<loftIndex> closestIndices1; closestIndices1.resize(f1->sh.nbIndex);
	std::vector<loftIndex> closestIndices2; closestIndices2.resize(f2->sh.nbIndex);
	for (auto closest : closestIndices1)
		closest.visited = false;
	for (auto closest : closestIndices2)
		closest.visited = false;

	double u1Length = f1->sh.U.Norme();
	double v1Length = f1->sh.V.Norme();
	double u2Length = f2->sh.U.Norme();
	double v2Length = f2->sh.V.Norme();

	Vector2d center2Pos = ProjectVertex(f2->sh.center, f1->sh.U, f1->sh.V, f1->sh.O);
	Vector2d center1Pos = ProjectVertex(f1->sh.center, f1->sh.U, f1->sh.V, f1->sh.O);
	Vector2d centerOffset = center1Pos - center2Pos; //aligns centers

	for (size_t i1 = 0; i1 < f1->sh.nbIndex; i1++) {
		//Find closest point on other facet
		double min = 9E99;
		size_t minPos;
		for (size_t i2 = 0; i2 < f2->sh.nbIndex; i2++) {
			Vector2d projection = ProjectVertex(vertices3[f2->indices[i2]], f1->sh.U, f1->sh.V, f1->sh.O);
			projection = projection + centerOffset;
			double dist = pow(u1Length*(projection.u - f1->vertices2[i1].u), 2.0) + pow(v1Length*(projection.v - f1->vertices2[i1].v), 2.0); //We need the absolute distance
			if (dist < min) {
				min = dist;
				minPos = i2;
			}
		}
		//Make pair
		closestIndices1[i1].index = minPos;
		closestIndices2[minPos].index = i1;
		closestIndices1[i1].visited = closestIndices2[minPos].visited = true;
	}

	//Find boundaries of regions on the first facet that are the closest to the same point on facet 2
	for (size_t i = 0; i < closestIndices1.size(); i++) {
		size_t previousId = (i + closestIndices1.size() - 1) % closestIndices1.size();
		size_t nextId = (i + 1) % closestIndices1.size();
		closestIndices1[i].boundary = (closestIndices1[i].index != closestIndices1[nextId].index) || (closestIndices1[i].index != closestIndices1[previousId].index);
	}

	center2Pos = ProjectVertex(f2->sh.center, f2->sh.U, f2->sh.V, f2->sh.O);
	center1Pos = ProjectVertex(f1->sh.center, f2->sh.U, f2->sh.V, f2->sh.O);
	centerOffset = center2Pos - center1Pos;

	//Revisit those on f2 without a link
	for (size_t i2 = 0; i2 < f2->sh.nbIndex; i2++) {
		//Find closest point on other facet
		if (!closestIndices2[i2].visited) {
			double min = 9E99;
			size_t minPos;
			for (size_t i1 = 0; i1 < f1->sh.nbIndex; i1++) {
				Vector2d projection = ProjectVertex(vertices3[f1->indices[i1]], f2->sh.U, f2->sh.V, f2->sh.O);
				projection = projection + centerOffset;
				double dist = pow(u2Length*(projection.u - f2->vertices2[i2].u), 2.0) + pow(v2Length*(projection.v - f2->vertices2[i2].v), 2.0); //We need the absolute distance
				if (!closestIndices1[i1].boundary) dist += 1E6; //penalty -> try to connect with boundaries
				if (dist < min) {
					min = dist;
					minPos = i1;
				}
			}
			//Make pair
			closestIndices2[i2].index = minPos;
			//closestIndices1[minPos].index = i2; //Alerady assigned
			closestIndices2[i2].visited = true;
		}
	}

	//Same for facet 2
	for (size_t i = 0; i < closestIndices2.size(); i++) {
		size_t previousId = (i + closestIndices2.size() - 1) % closestIndices2.size();
		size_t nextId = (i + 1) % closestIndices2.size();
		closestIndices2[i].boundary = (closestIndices2[i].index != closestIndices2[nextId].index) || (closestIndices2[i].index != closestIndices2[previousId].index);
		closestIndices2[i].visited = false; //Reset this flag, will use to make sure we don't miss anything
	}

	//Links created
	std::vector<Facet*> newFacets;
	for (size_t i1 = 0; i1 < f1->sh.nbIndex; i1++) {
		//Look for smaller points in increasing direction
		for (size_t i2 = (closestIndices1[i1].index + 1) % f2->sh.nbIndex; closestIndices2[i2].index == i1; i2 = (i2 + 1) % f2->sh.nbIndex) {
			//Create triangle
			Facet *newFacet = new Facet(3);
			newFacet->indices[0] = f1->indices[i1];
			newFacet->indices[1] = f2->indices[IDX(i2 - 1, f2->sh.nbIndex)]; closestIndices2[IDX(i2 - 1, f2->sh.nbIndex)].visited = true;
			newFacet->indices[2] = f2->indices[i2]; closestIndices2[i2].visited = true;
			newFacet->selected = true;
			if (viewStruct != -1) newFacet->sh.superIdx = viewStruct;
			newFacet->SwapNormal();
			newFacets.push_back(newFacet);
		}
		//Look for smaller points in decreasing direction
		for (size_t i2 = IDX(closestIndices1[i1].index - 1, f2->sh.nbIndex); closestIndices2[i2].index == i1; i2 = IDX(i2 - 1, f2->sh.nbIndex)) {
			//Create triangle
			Facet *newFacet = new Facet(3);
			newFacet->indices[0] = f1->indices[i1];
			newFacet->indices[1] = f2->indices[i2]; closestIndices2[i2].visited = true;
			newFacet->indices[2] = f2->indices[(i2 + 1) % f2->sh.nbIndex]; closestIndices2[(i2 + 1) % f2->sh.nbIndex].visited = true;
			newFacet->selected = true;
			if (viewStruct != -1) newFacet->sh.superIdx = viewStruct;
			newFacet->SwapNormal();
			newFacets.push_back(newFacet);
		}
		//Create rectangle
		bool triangle = f2->indices[closestIndices1[(i1 + 1) % f1->sh.nbIndex].index] == f2->indices[closestIndices1[i1].index];
		Facet *newFacet = new Facet(triangle ? 3 : 4);
		newFacet->indices[0] = f1->indices[i1];
		newFacet->indices[1] = f1->indices[(i1 + 1) % f1->sh.nbIndex];
		//Find last vertex on other facet that's closest to us
		size_t increment;
		for (increment = 0; closestIndices2[IDX(closestIndices1[(i1 + 1) % f1->sh.nbIndex].index + increment + incrementDir, f2->sh.nbIndex)].index == ((i1 + 1) % f1->sh.nbIndex); increment += incrementDir);
		newFacet->indices[2] = f2->indices[IDX(closestIndices1[(i1 + 1) % f1->sh.nbIndex].index + increment, f2->sh.nbIndex)]; closestIndices2[IDX(closestIndices1[(i1 + 1) % f1->sh.nbIndex].index + increment, f2->sh.nbIndex)].visited = true;
		if (!triangle) newFacet->indices[3] = f2->indices[IDX(closestIndices1[(i1 + 1) % f1->sh.nbIndex].index + incrementDir + increment, f2->sh.nbIndex)]; closestIndices2[IDX(closestIndices1[(i1 + 1) % f1->sh.nbIndex].index + incrementDir + increment, f2->sh.nbIndex)].visited = true;
		if (incrementDir == -1) newFacet->SwapNormal();
		CalculateFacetParam(newFacet);
		if (abs(newFacet->err) > 1E-5) {
			//Split to two triangles
			size_t ind4[] = { newFacet->indices[0],newFacet->indices[1], newFacet->indices[2], newFacet->indices[3] };
			delete newFacet;
			newFacet = new Facet(3);
			Vector3d diff_0_2 = vertices3[ind4[0]] - vertices3[ind4[2]];
			Vector3d diff_1_3 = vertices3[ind4[1]] - vertices3[ind4[3]];
			bool connect_0_2 = diff_0_2.Norme() < diff_1_3.Norme(); //Split rectangle to two triangles along shorter side
			newFacet->indices[0] = ind4[0];
			newFacet->indices[1] = ind4[1];
			newFacet->indices[2] = ind4[connect_0_2 ? 2 : 3];
			newFacet->selected = true;
			newFacet->SwapNormal();
			newFacets.push_back(newFacet);

			newFacet = new Facet(3);
			newFacet->indices[0] = ind4[connect_0_2 ? 0 : 1];
			newFacet->indices[1] = ind4[2];
			newFacet->indices[2] = ind4[3];
		}
		newFacet->SwapNormal();
		newFacet->selected = true;
		if (viewStruct != -1) newFacet->sh.superIdx = viewStruct;
		newFacets.push_back(newFacet);
	}
	//Go through leftover vertices on facet 2
	for (size_t i2 = 0; i2 < f2->sh.nbIndex; i2++) {
		if (closestIndices2[i2].visited == false) {
			size_t targetIndex = closestIndices2[IDX(i2 - 1, f2->sh.nbIndex)].index; //Previous node

			do {
				//Connect with previous
				Facet *newFacet = new Facet(3);
				newFacet->indices[0] = f1->indices[targetIndex];
				newFacet->indices[1] = f2->indices[i2]; closestIndices2[i2].visited = true;
				newFacet->indices[2] = f2->indices[IDX(i2 - 1, f2->sh.nbIndex)]; closestIndices2[IDX(i2 - 1, f2->sh.nbIndex)].visited = true;
				newFacet->selected = true;
				if (viewStruct != -1) newFacet->sh.superIdx = viewStruct;
				newFacets.push_back(newFacet);
				i2 = IDX(i2 + 1, f2->sh.nbIndex);
			} while (closestIndices2[i2].visited == false);
			//last
				//Connect with next for the last unvisited
			Facet *newFacet = new Facet(3);
			newFacet->indices[0] = f1->indices[targetIndex];
			newFacet->indices[1] = f2->indices[i2]; closestIndices2[i2].visited = true;
			newFacet->indices[2] = f2->indices[IDX(i2 - 1, f2->sh.nbIndex)]; closestIndices2[IDX(i2 - 1, f2->sh.nbIndex)].visited = true;
			newFacet->selected = true;
			if (viewStruct != -1) newFacet->sh.superIdx = viewStruct;
			newFacets.push_back(newFacet);
		}
	}
	//Register new facets
	if (newFacets.size() > 0) facets = (Facet**)realloc(facets, sizeof(Facet*)*(sh.nbFacet + newFacets.size()));
	for (size_t i = 0; i < newFacets.size(); i++)
		facets[sh.nbFacet + i] = newFacets[i];
	sh.nbFacet += newFacets.size();
	InitializeGeometry();
}

void Geometry::SetAutoNorme(bool enable) {
	autoNorme = enable;
}

bool Geometry::GetAutoNorme() {
	return autoNorme;
}

void Geometry::SetCenterNorme(bool enable) {
	centerNorme = enable;
}

bool Geometry::GetCenterNorme() {
	return centerNorme;
}

void Geometry::SetNormeRatio(float r) {
	normeRatio = r;
}

float Geometry::GetNormeRatio() {
	return normeRatio;
}

void Geometry::BuildShapeList() {

	// Shapes used for direction field rendering

	// 3D arrow (direction field)
	int nbDiv = 10;
	double alpha = 2.0*PI / (double)nbDiv;

	arrowList = glGenLists(1);
	glNewList(arrowList, GL_COMPILE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glBegin(GL_TRIANGLES);

	// Arrow
	for (int i = 0; i < nbDiv; i++) {

		double y1 = sin(alpha*(double)i);
		double z1 = cos(alpha*(double)i);
		double y2 = sin(alpha*(double)((i + 1) % nbDiv));
		double z2 = cos(alpha*(double)((i + 1) % nbDiv));

		glNormal3d(0.0, y1, z1);
		glVertex3d(-0.5, 0.5*y1, 0.5*z1);
		glNormal3d(1.0, 0.0, 0.0);
		glVertex3d(0.5, 0.0, 0.0);
		glNormal3d(0.0, y2, z2);
		glVertex3d(-0.5, 0.5*y2, 0.5*z2);

	}

	// Cap facets
	for (int i = 0; i < nbDiv; i++) {

		double y1 = sin(alpha*(double)i);
		double z1 = cos(alpha*(double)i);
		double y2 = sin(alpha*(double)((i + 1) % nbDiv));
		double z2 = cos(alpha*(double)((i + 1) % nbDiv));

		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(-0.5, 0.5*y1, 0.5*z1);
		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(-0.5, 0.5*y2, 0.5*z2);
		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(-0.5, 0.0, 0.0);

	}

	glEnd();
	glEndList();

	// Shpere list (isotropic case)
	int nbPhi = 16;
	int nbTetha = 7;
	double dphi = 2.0*PI / (double)(nbPhi);
	double dtetha = PI / (double)(nbTetha + 1);

	sphereList = glGenLists(1);
	glNewList(sphereList, GL_COMPILE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glBegin(GL_TRIANGLES);

	for (int i = 0; i <= nbTetha; i++) {
		for (int j = 0; j < nbPhi; j++) {

			Vector3d v1, v2, v3, v4;

			v1.x = sin(dtetha*(double)i)*cos(dphi*(double)j);
			v1.y = sin(dtetha*(double)i)*sin(dphi*(double)j);
			v1.z = cos(dtetha*(double)i);

			v2.x = sin(dtetha*(double)(i + 1))*cos(dphi*(double)j);
			v2.y = sin(dtetha*(double)(i + 1))*sin(dphi*(double)j);
			v2.z = cos(dtetha*(double)(i + 1));

			v3.x = sin(dtetha*(double)(i + 1))*cos(dphi*(double)(j + 1));
			v3.y = sin(dtetha*(double)(i + 1))*sin(dphi*(double)(j + 1));
			v3.z = cos(dtetha*(double)(i + 1));

			v4.x = sin(dtetha*(double)i)*cos(dphi*(double)(j + 1));
			v4.y = sin(dtetha*(double)i)*sin(dphi*(double)(j + 1));
			v4.z = cos(dtetha*(double)i);

			if (i < nbTetha) {
				glNormal3d(v1.x, v1.y, v1.z);
				glVertex3d(v1.x, v1.y, v1.z);
				glNormal3d(v2.x, v2.y, v2.z);
				glVertex3d(v2.x, v2.y, v2.z);
				glNormal3d(v3.x, v3.y, v3.z);
				glVertex3d(v3.x, v3.y, v3.z);
			}

			if (i > 0) {
				glNormal3d(v1.x, v1.y, v1.z);
				glVertex3d(v1.x, v1.y, v1.z);
				glNormal3d(v3.x, v3.y, v3.z);
				glVertex3d(v3.x, v3.y, v3.z);
				glNormal3d(v4.x, v4.y, v4.z);
				glVertex3d(v4.x, v4.y, v4.z);
			}

		}
	}

	glEnd();
	glEndList();

}

void Geometry::BuildSelectList() {

	selectList = glGenLists(1);
	glNewList(selectList, GL_COMPILE);
	/*
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	if (antiAliasing){
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	//glBlendFunc(GL_ONE,GL_ZERO);
	}
	glLineWidth(2.0f);

	for(int i=0;i<sh.nbFacet;i++ ) {
	Facet *f = facets[i];
	if( f->selected ) {
	//DrawFacet(f,false);
	DrawFacet(f,1,1,1);
	nbSelected++;
	}
	}
	glLineWidth(1.0f);
	if (antiAliasing) {
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	}*/
	glEndList();

	// Second list for usage with POLYGON_OFFSET
	selectList2 = glGenLists(1);
	glNewList(selectList2, GL_COMPILE);
	/*
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	if (antiAliasing){
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}
	glLineWidth(2.0f);

	for(int i=0;i<sh.nbFacet;i++ ) {
	Facet *f = facets[i];
	if( f->selected )
	{
	//DrawFacet(f,true,false,true);
	DrawFacet(f,1,1,1);
	}
	}
	glLineWidth(1.0f);
	if (antiAliasing) {
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	}*/
	glEndList();

	// Third list with hidden (hole join) edge visible
	selectList3 = glGenLists(1);
	glNewList(selectList3, GL_COMPILE);

	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	if (mApp->antiAliasing) {
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}
	glLineWidth(2.0f);

	auto selectedFacets = GetSelectedFacets();
	for (auto sel : selectedFacets) {
		Facet *f = facets[sel];
		//DrawFacet(f,false,true,true);
		DrawFacet(f, false, true, false); //Faster than true true true, without noticeable glitches
	}
	glLineWidth(1.0f);
	if (mApp->antiAliasing) {
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}
	glEndList();

}

void Geometry::UpdateSelection() {

	DeleteGLLists();
	BuildSelectList();

}

void Geometry::BuildGLList() {

	// Compile geometry for OpenGL
	for (int j = 0; j < sh.nbSuper; j++) {
		lineList[j] = glGenLists(1);
		glNewList(lineList[j], GL_COMPILE);
		for (int i = 0; i < sh.nbFacet; i++) {
			if (facets[i]->sh.superIdx == j)
				DrawFacet(facets[i], false, true, false);
		}
		glEndList();
	}

	polyList = glGenLists(1);
	glNewList(polyList, GL_COMPILE);
	DrawPolys();
	glEndList();

	BuildSelectList();

}

void Geometry::Rebuild() {

	// Rebuild internal structure on geometry change

	// Remove texture (improvement TODO)
	/*for (int i = 0; i < sh.nbFacet; i++)
		if (facets[i]->sh.isTextured)
			facets[i]->SetTexture(0.0, 0.0, false);*/

			// Delete old resources
	DeleteGLLists(true, true);

	// Reinitialise geom
	InitializeGeometry();

}

int Geometry::InvalidateDeviceObjects() {

	DeleteGLLists(true, true);
	DELETE_LIST(arrowList);
	DELETE_LIST(sphereList);
	for (int i = 0; i < sh.nbFacet; i++)
		facets[i]->InvalidateDeviceObjects();

	return GL_OK;

}

int Geometry::RestoreDeviceObjects() {

	if (!IsLoaded()) return GL_OK;

	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		f->RestoreDeviceObjects();
		BuildFacetList(f);
	}

	BuildGLList();

	return GL_OK;

}

void Geometry::BuildFacetList(Facet *f) {

	// Rebuild OpenGL geomtetry with texture

	if (f->sh.isTextured) {

		// Facet geometry
		glNewList(f->glList, GL_COMPILE);
		if (f->sh.nbIndex == 3) {
			glBegin(GL_TRIANGLES);
			FillFacet(f, true);
			glEnd();
		}
		else if (f->sh.nbIndex == 4) {

			glBegin(GL_QUADS);
			FillFacet(f, true);
			glEnd();
		}
		else {

			glBegin(GL_TRIANGLES);
			Triangulate(f, true);
			glEnd();
		}
		glEndList();
	}
}

void Geometry::SetFacetTexture(size_t facetId, double ratio, bool mesh) {

	Facet *f = facets[facetId];
	double nU = f->sh.U.Norme();
	double nV = f->sh.V.Norme();

	if (!f->SetTexture(nU*ratio, nV*ratio, mesh)) {
		char errMsg[512];
		sprintf(errMsg, "Not enough memory to build mesh on Facet %zd. ", facetId + 1);
		throw Error(errMsg);
	}
	f->tRatio = ratio;
	BuildFacetList(f);

}

// File handling

void Geometry::UpdateName(FileReader *file) {
	UpdateName(file->GetName());
}

void Geometry::UpdateName(const char *fileName) {
	strncpy(sh.name, FileUtils::GetFilename(fileName).c_str(),64);
	sh.name[63] = 0;
}

void Geometry::AdjustProfile() {

	// Backward compatibily with TXT profile (To be improved)
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		if (f->sh.profileType == REC_PRESSUREU) {
			Vector3d v0 = vertices3[f->indices[1]] - vertices3[f->indices[0]];
			double n0 = v0.Norme();
			double nU = f->sh.U.Norme();
			if (IsZero(n0 - nU)) f->sh.profileType = REC_PRESSUREU; // Select U
			else               f->sh.profileType = REC_PRESSUREV; // Select V
		}
	}

}

void Geometry::ResetTextureLimits() {
#ifdef MOLFLOW
	texture_limits[0].autoscale.min.all = texture_limits[0].autoscale.min.moments_only =
		texture_limits[1].autoscale.min.all = texture_limits[1].autoscale.min.moments_only =
		texture_limits[2].autoscale.min.all = texture_limits[2].autoscale.min.moments_only =
		texture_limits[0].manual.min.all = texture_limits[0].manual.min.moments_only =
		texture_limits[1].manual.min.all = texture_limits[1].manual.min.moments_only =
		texture_limits[2].manual.min.all = texture_limits[2].manual.min.moments_only = 0.0;
	texture_limits[0].autoscale.max.all = texture_limits[0].autoscale.max.moments_only =
		texture_limits[1].autoscale.max.all = texture_limits[1].autoscale.max.moments_only =
		texture_limits[2].autoscale.max.all = texture_limits[2].autoscale.max.moments_only =
		texture_limits[0].manual.max.all = texture_limits[0].manual.max.moments_only =
		texture_limits[1].manual.max.all = texture_limits[1].manual.max.moments_only =
		texture_limits[2].manual.max.all = texture_limits[2].manual.max.moments_only = 1.0;
#endif
#ifdef SYNRAD
	texCMin_MC = 0;             // Current minimum
	texCMax_MC = 1;             // Current maximum
	texCMin_flux = texCMin_power = 0.0;
	texCMax_flux = texCMax_power = 1.0;
#endif
}

void Geometry::LoadASE(FileReader *file, GLProgress *prg) {

	Clear();

	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	ASELoader ase(file);
	ase.Load();

	// Compute total of facet
	sh.nbFacet = 0;
	for (int i = 0; i < ase.nbObj; i++) sh.nbFacet += ase.OBJ[i].nb_face;

	// Allocate mem
	sh.nbVertex = 3 * sh.nbFacet;
	facets = (Facet **)malloc(sh.nbFacet * sizeof(Facet *));
	memset(facets, 0, sh.nbFacet * sizeof(Facet *));
	vertices3 = (InterfaceVertex *)malloc(sh.nbVertex * sizeof(InterfaceVertex));
	memset(vertices3, 0, sh.nbVertex * sizeof(InterfaceVertex));

	// Fill 
	int nb = 0;
	for (int i = 0; i < ase.nbObj; i++) {

		for (int j = 0; j < ase.OBJ[i].nb_face; j++) {
			vertices3[3 * nb + 0].SetLocation(ase.OBJ[i].pts[ase.OBJ[i].face[j].v1]);
			vertices3[3 * nb + 1].SetLocation(ase.OBJ[i].pts[ase.OBJ[i].face[j].v2]);
			vertices3[3 * nb + 2].SetLocation(ase.OBJ[i].pts[ase.OBJ[i].face[j].v3]);
			facets[nb] = new Facet(3);
			facets[nb]->indices[0] = 3 * nb + 0;
			facets[nb]->indices[1] = 3 * nb + 1;
			facets[nb]->indices[2] = 3 * nb + 2;
			nb++;
		}

	}

	UpdateName(file);
	sh.nbSuper = 1;
	strName[0] = _strdup(sh.name);
	strFileName[0] = _strdup(file->GetName());
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::LoadSTR(FileReader *file, GLProgress *prg) {

	char nPath[512];
	char fPath[512];
	char fName[512];
	char sName[512];
	size_t nF, nV;
	Facet **F;
	InterfaceVertex *V;
	FileReader *fr;

	Clear();

	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	// Load multiple structure file
	sh.nbSuper = file->ReadInt();

	strcpy(fPath, file->ReadLine());
	strcpy(nPath, FileUtils::GetPath(file->GetName()).c_str());

	for (int n = 0; n < sh.nbSuper; n++) {

		int i1 = file->ReadInt();
		int i2 = file->ReadInt();
		fr = NULL;
		strcpy(sName, file->ReadWord());
		strName[n] = _strdup(sName);
		char *e = strrchr(strName[n], '.');
		if (e) *e = 0;

		sprintf(fName, "%s%s", nPath, sName);
		if (FileUtils::Exist(fName)) {
			fr = new FileReader(fName);
			strcpy(strPath, nPath);

		}
		else {

			sprintf(fName, "%s\\%s", fPath, sName);
			if (FileUtils::Exist(fName)) {
				fr = new FileReader(fName);
				strcpy(strPath, fPath);
			}
		}

		if (!fr) {
			char errMsg[512];
			sprintf(errMsg, "Cannot find %s", sName);
			throw Error(errMsg);
		}

		strFileName[n] = _strdup(sName);
		LoadTXTGeom(fr, &nV, &nF, &V, &F, n);
		Merge(nV, nF, V, F);
		SAFE_FREE(V);
		SAFE_FREE(F);
		delete fr;

	}

	UpdateName(file);
	InitializeGeometry();
	AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::LoadSTL(FileReader *file, GLProgress *prg, double scaleFactor) {

	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	char *w;

	prg->SetMessage("Clearing current geometry...");
	Clear();

	// First pass
	prg->SetMessage("Counting facets in STL file...");
	//file->ReadKeyword("solid");
	file->ReadLine(); // solid name
	w = file->ReadWord();
	while (strcmp(w, "facet") == 0) {
		sh.nbFacet++;
		file->JumpSection("endfacet");
		w = file->ReadWord();
	}
	if (strcmp(w, "endsolid") != 0) throw Error("Unexpected or not supported STL keyword, 'endsolid' required\nMaybe the STL file was saved in binary instead of ASCII format?");

	// Allocate mem
	sh.nbVertex = 3 * sh.nbFacet;
	facets = (Facet **)malloc(sh.nbFacet * sizeof(Facet *));
	if (!facets) throw Error("Out of memory: LoadSTL");
	memset(facets, 0, sh.nbFacet * sizeof(Facet *));
	vertices3 = (InterfaceVertex *)malloc(sh.nbVertex * sizeof(InterfaceVertex));
	if (!vertices3) throw Error("Out of memory: LoadSTL");
	memset(vertices3, 0, sh.nbVertex * sizeof(InterfaceVertex));

	// Second pass
	prg->SetMessage("Reading facets...");
	file->SeekStart();
	//file->ReadKeyword("solid");
	file->ReadLine();
	for (int i = 0; i < sh.nbFacet; i++) {

		double p = (double)i / (double)(sh.nbFacet);
		prg->SetProgress(p);

		file->ReadKeyword("facet");
		file->ReadKeyword("normal");
		file->ReadDouble();
		file->ReadDouble();
		file->ReadDouble();
		file->ReadKeyword("outer");
		file->ReadKeyword("loop");

		file->ReadKeyword("vertex");
		vertices3[3 * i + 0].x = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 0].y = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 0].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("vertex");
		vertices3[3 * i + 1].x = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 1].y = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 1].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("vertex");
		vertices3[3 * i + 2].x = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 2].y = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 2].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("endloop");
		file->ReadKeyword("endfacet");

		try {
			facets[i] = new Facet(3);
		}
		catch (...) {
			throw Error("Out of memory");
		}
		facets[i]->indices[0] = 3 * i + 0;
		facets[i]->indices[1] = 3 * i + 2;
		facets[i]->indices[2] = 3 * i + 1;

	}

	sh.nbSuper = 1;
	UpdateName(file);
	strName[0] = _strdup(sh.name);
	strFileName[0] = _strdup(file->GetName());
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	prg->SetMessage("Initializing geometry...");
	InitializeGeometry();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::LoadTXT(FileReader *file, GLProgress *prg) {

	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	Clear();
	LoadTXTGeom(file, &(sh.nbVertex), &(sh.nbFacet), &vertices3, &facets);
	UpdateName(file);
	sh.nbSuper = 1;
	strName[0] = _strdup(sh.name);
	strFileName[0] = _strdup(file->GetName());

	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::InsertTXT(FileReader *file, GLProgress *prg, bool newStr) {

	//Clear();
	int structId = viewStruct;
	if (structId == -1) structId = 0;
	InsertTXTGeom(file, &(sh.nbVertex), &(sh.nbFacet), &vertices3, &facets, structId, newStr);
	//UpdateName(file);
	//sh.nbSuper = 1;
	//strName[0] = _strdup(sh.name);
	//strFileName[0] = _strdup(file->GetName());
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::InsertSTL(FileReader *file, GLProgress *prg, double scaleFactor, bool newStr) {

	//Clear();
	int structId = viewStruct;
	if (structId == -1) structId = 0;
	InsertSTLGeom(file, &(sh.nbVertex), &(sh.nbFacet), &vertices3, &facets, structId, scaleFactor, newStr);
	//UpdateName(file);
	//sh.nbSuper = 1;
	//strName[0] = _strdup(sh.name);
	//strFileName[0] = _strdup(file->GetName());
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	//AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::InsertGEO(FileReader *file, GLProgress *prg, bool newStr) {

	//Clear();
	int structId = viewStruct;
	if (structId == -1) structId = 0;
	InsertGEOGeom(file, &(sh.nbVertex), &(sh.nbFacet), &vertices3, &facets, structId, newStr);
	//UpdateName(file);
	//sh.nbSuper = 1;
	//strName[0] = _strdup(sh.name);
	//strFileName[0] = _strdup(file->GetName());
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	//AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::LoadTXTGeom(FileReader *file, size_t *nbV, size_t *nbF, InterfaceVertex **V, Facet ***F, size_t strIdx) {

	file->ReadInt(); // Unused
	loaded_nbHit = file->ReadLLong();
	loaded_nbLeak = file->ReadLLong();
	loaded_nbDesorption = file->ReadLLong();
	loaded_desorptionLimit = file->ReadLLong();

	int nV = file->ReadInt();
	int nF = file->ReadInt();

	// Allocate memory
	Facet   **f = (Facet **)malloc(nF * sizeof(Facet *));
	memset(f, 0, nF * sizeof(Facet *));
	InterfaceVertex *v = (InterfaceVertex *)malloc(nV * sizeof(InterfaceVertex));
	memset(v, 0, nV * sizeof(InterfaceVertex)); //avoid selected flag

	// Read geometry vertices
	for (int i = 0; i < nV; i++) {
		v[i].x = file->ReadDouble();
		v[i].y = file->ReadDouble();
		v[i].z = file->ReadDouble();
	}

	// Read geometry facets (indexed from 1)
	for (int i = 0; i < nF; i++) {
		int nb = file->ReadInt();
		f[i] = new Facet(nb);
		for (int j = 0; j < nb; j++)
			f[i]->indices[j] = file->ReadInt() - 1;
	}

	// Read facets params
	for (int i = 0; i < nF; i++) {
		f[i]->LoadTXT(file);
		while ((f[i]->sh.superDest) > sh.nbSuper) { //If facet refers to a structure that doesn't exist, create it
			AddStruct("TXT linked");
		}
		f[i]->sh.superIdx = strIdx;
	}

	SAFE_FREE(*V);
	SAFE_FREE(*F);

	*nbV = nV;
	*nbF = nF;
	*V = v;
	*F = f;

}

void Geometry::InsertTXTGeom(FileReader *file, size_t *nbVertex, size_t *nbFacet, InterfaceVertex **vertices3, Facet ***facets, size_t strIdx, bool newStruct) {

	UnselectAll();

	//loaded_nbHit = file->ReadLLong();
	//loaded_nbLeak = file->ReadInt();
	//loaded_nbDesorption = file->ReadLLong();
	//loaded_desorptionLimit = file->ReadLLong(); 
	for (int i = 0; i < 5; i++) file->ReadInt(); //leading lines

	int nbNewVertex = file->ReadInt();
	int nbNewFacets = file->ReadInt();

	// Allocate memory
	*facets = (Facet **)realloc(*facets, (nbNewFacets + *nbFacet) * sizeof(Facet **));
	memset(*facets + *nbFacet, 0, nbNewFacets * sizeof(Facet *));
	//*vertices3 = (Vector3d*)realloc(*vertices3,(nbNewVertex+*nbVertex) * sizeof(Vector3d));
	InterfaceVertex *tmp_vertices3 = (InterfaceVertex *)malloc((nbNewVertex + *nbVertex) * sizeof(InterfaceVertex));
	memmove(tmp_vertices3, *vertices3, (*nbVertex) * sizeof(InterfaceVertex));
	memset(tmp_vertices3 + *nbVertex, 0, nbNewVertex * sizeof(InterfaceVertex));
	SAFE_FREE(*vertices3);
	*vertices3 = tmp_vertices3;

	// Read geometry vertices
	for (size_t i = *nbVertex; i < (*nbVertex + nbNewVertex); i++) {
		(*vertices3 + i)->x = file->ReadDouble();
		(*vertices3 + i)->y = file->ReadDouble();
		(*vertices3 + i)->z = file->ReadDouble();
		(*vertices3 + i)->selected = false;
	}

	// Read geometry facets (indexed from 1)
	for (size_t i = *nbFacet; i < (*nbFacet + nbNewFacets); i++) {
		size_t nb = file->ReadLLong();
		*(*facets + i) = new Facet(nb);
		(*facets)[i]->selected = true;
		for (size_t j = 0; j < nb; j++)
			(*facets)[i]->indices[j] = file->ReadLLong() - 1 + *nbVertex;
	}

	// Read facets params
	for (size_t i = *nbFacet; i < (*nbFacet + nbNewFacets); i++) {
		(*facets)[i]->LoadTXT(file);
		while (((*facets)[i]->sh.superDest) > sh.nbSuper) { //If facet refers to a structure that doesn't exist, create it
			AddStruct("TXT linked");
		}
		if (newStruct) {
			(*facets)[i]->sh.superIdx = sh.nbSuper;
		}
		else {

			(*facets)[i]->sh.superIdx = strIdx;
		}
	}

	*nbVertex += nbNewVertex;
	*nbFacet += nbNewFacets;
	if (newStruct) AddStruct("Inserted TXT file");

}

void Geometry::InsertGEOGeom(FileReader *file, size_t *nbVertex, size_t *nbFacet, InterfaceVertex **vertices3, Facet ***facets, size_t strIdx, bool newStruct) {

	UnselectAll();

	file->ReadKeyword("version"); file->ReadKeyword(":");
	int version2;
	version2 = file->ReadInt();
	if (version2 > GEOVERSION) {
		char errMsg[512];
		sprintf(errMsg, "Unsupported GEO version V%d", version2);
		throw Error(errMsg);
	}

	file->ReadKeyword("totalHit"); file->ReadKeyword(":");
	file->ReadLLong();
	file->ReadKeyword("totalDes"); file->ReadKeyword(":");
	file->ReadLLong();
	file->ReadKeyword("totalLeak"); file->ReadKeyword(":");
	file->ReadLLong();
	if (version2 >= 12) {
		file->ReadKeyword("totalAbs"); file->ReadKeyword(":");
		file->ReadLLong();
		if (version2 >= 15) {
			file->ReadKeyword("totalDist_total");
		}
		else { //between versions 12 and 15
			file->ReadKeyword("totalDist");
		}
		file->ReadKeyword(":");
		file->ReadDouble();
		if (version2 >= 15) {
			file->ReadKeyword("totalDist_fullHitsOnly"); file->ReadKeyword(":");
			file->ReadDouble();
		}
	}
	file->ReadKeyword("maxDes"); file->ReadKeyword(":");
	file->ReadLLong();
	file->ReadKeyword("nbVertex"); file->ReadKeyword(":");
	int nbNewVertex = file->ReadInt();
	file->ReadKeyword("nbFacet"); file->ReadKeyword(":");
	int nbNewFacets = file->ReadInt();
	file->ReadKeyword("nbSuper"); file->ReadKeyword(":");
	int nbNewSuper = file->ReadInt();
	int nbF = 0; std::vector<std::vector<string>> loadFormulas;
	int nbV = 0;
	if (version2 >= 2) {
		file->ReadKeyword("nbFormula"); file->ReadKeyword(":");
		nbF = file->ReadInt();
		file->ReadKeyword("nbView"); file->ReadKeyword(":");
		nbV = file->ReadInt();
	}
	int nbS = 0;
	if (version2 >= 8) {
		file->ReadKeyword("nbSelection"); file->ReadKeyword(":");
		nbS = file->ReadInt();
	}
	if (version2 >= 7) {
		file->ReadKeyword("gasMass"); file->ReadKeyword(":");
		/*gasMass = */file->ReadDouble();
	}
	if (version2 >= 10) { //time-dependent version
		file->ReadKeyword("userMoments"); file->ReadKeyword("{");
		file->ReadKeyword("nb"); file->ReadKeyword(":");
		int nb = file->ReadInt();

		for (int i = 0; i < nb; i++)
			file->ReadString();
		file->ReadKeyword("}");
	}
	if (version2 >= 11) { //gas pulse parameters
		file->ReadKeyword("desorptionStart"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("desorptionStop"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("timeWindow"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("useMaxwellian"); file->ReadKeyword(":");
		file->ReadInt();
	}

	if (version2 >= 12) { //2013.aug.22
		file->ReadKeyword("calcConstantFlow"); file->ReadKeyword(":");
		file->ReadInt();
	}

	if (version2 >= 2) {
		file->ReadKeyword("formulas"); file->ReadKeyword("{");
		for (int i = 0; i < nbF; i++) {
			char tmpName[256];
			char tmpExpr[512];
			strcpy(tmpName, file->ReadString());
			strcpy(tmpExpr, file->ReadString());
			//mApp->OffsetFormula(tmpExpr, sh.nbFacet);
			//mApp->AddFormula(tmpName, tmpExpr); //parse after selection groups are loaded
#ifdef MOLFLOW
			std::vector<string> newFormula;
			newFormula.push_back(tmpName);
			mApp->OffsetFormula(tmpExpr, (int)sh.nbFacet); //offset formula
			newFormula.push_back(tmpExpr);
			loadFormulas.push_back(newFormula);
#endif
		}
		file->ReadKeyword("}");

		file->ReadKeyword("views"); file->ReadKeyword("{");
		for (int i = 0; i < nbV; i++) {
			char tmpName[256];
			AVIEW v;
			strcpy(tmpName, file->ReadString());
			v.projMode = file->ReadInt();
			v.camAngleOx = file->ReadDouble();
			v.camAngleOy = file->ReadDouble();
			v.camDist = file->ReadDouble();
			v.camOffset.x = file->ReadDouble();
			v.camOffset.y = file->ReadDouble();
			v.camOffset.z = file->ReadDouble();
			v.performXY = file->ReadInt();

			v.vLeft = file->ReadDouble();
			v.vRight = file->ReadDouble();
			v.vTop = file->ReadDouble();
			v.vBottom = file->ReadDouble();
			mApp->AddView(tmpName, v);
		}
		file->ReadKeyword("}");
	}

	if (version2 >= 8) {
		file->ReadKeyword("selections"); file->ReadKeyword("{");
		for (int i = 0; i < nbS; i++) {
			SelectionGroup s;
			char tmpName[256];
			strcpy(tmpName, file->ReadString());
			s.name = _strdup(tmpName);
			int nbFS = file->ReadInt();

			for (int j = 0; j < nbFS; j++) {
				s.selection.push_back((size_t)file->ReadInt() + sh.nbFacet); //offset facet number by current number of facets
			}
			mApp->AddSelection(s);
		}
		file->ReadKeyword("}");
	}
#ifdef MOLFLOW
	for (int i = 0; i < nbF; i++) { //parse formulas now that selection groups are loaded
		mApp->AddFormula(loadFormulas[i][0].c_str(), loadFormulas[i][1].c_str());
	}
#endif
	file->ReadKeyword("structures"); file->ReadKeyword("{");
	for (int i = 0; i < nbNewSuper; i++) {
		strName[sh.nbSuper + i] = _strdup(file->ReadString());
		// For backward compatibilty with STR
		/* //Commented out for GEO
		sprintf(tmp, "%s.txt", strName[i]);
		strFileName[i] = _strdup(tmp);
		*/
	}
	file->ReadKeyword("}");

	// Reallocate memory
	*facets = (Facet **)realloc(*facets, (nbNewFacets + *nbFacet) * sizeof(Facet **));
	memset(*facets + *nbFacet, 0, nbNewFacets * sizeof(Facet *));
	//*vertices3 = (Vector3d*)realloc(*vertices3,(nbNewVertex+*nbVertex) * sizeof(Vector3d));
	InterfaceVertex *tmp_vertices3 = (InterfaceVertex *)malloc((nbNewVertex + *nbVertex) * sizeof(InterfaceVertex));
	if (!tmp_vertices3) throw Error("Out of memory: InsertGEOGeom");
	memmove(tmp_vertices3, *vertices3, (*nbVertex) * sizeof(InterfaceVertex));
	memset(tmp_vertices3 + *nbVertex, 0, nbNewVertex * sizeof(InterfaceVertex));
	SAFE_FREE(*vertices3);
	*vertices3 = tmp_vertices3;

	// Read geometry vertices
	file->ReadKeyword("vertices"); file->ReadKeyword("{");
	for (size_t i = *nbVertex; i < (*nbVertex + nbNewVertex); i++) {
		// Check idx
		size_t idx = file->ReadLLong();
		if (idx != i - *nbVertex + 1) throw Error(file->MakeError("Wrong vertex index !"));
		(*vertices3 + i)->x = file->ReadDouble();
		(*vertices3 + i)->y = file->ReadDouble();
		(*vertices3 + i)->z = file->ReadDouble();
		(*vertices3 + i)->selected = false;
	}
	file->ReadKeyword("}");

	if (version2 >= 6) {
		// Read leaks
		file->ReadKeyword("leaks"); file->ReadKeyword("{");
		file->ReadKeyword("nbLeak"); file->ReadKeyword(":");
		int nbleak2 = file->ReadInt();
		for (int i = 0; i < nbleak2; i++) {
			int idx = file->ReadInt();
			//if( idx != i ) throw Error(file->MakeError("Wrong leak index !"));
			file->ReadDouble();
			file->ReadDouble();
			file->ReadDouble();

			file->ReadDouble();
			file->ReadDouble();
			file->ReadDouble();
		}
		file->ReadKeyword("}");

		// Read hit cache
		file->ReadKeyword("hits"); file->ReadKeyword("{");
		file->ReadKeyword("nbHHit"); file->ReadKeyword(":");
		int nbHHit2 = file->ReadInt();
		for (int i = 0; i < nbHHit2; i++) {
			int idx = file->ReadInt();
			//if( idx != i ) throw Error(file->MakeError("Wrong hit cache index !"));
			file->ReadDouble();
			file->ReadDouble();
			file->ReadDouble();

			file->ReadInt();
		}
		file->ReadKeyword("}");
	}

	// Read geometry facets (indexed from 1)
	for (size_t i = *nbFacet; i < (*nbFacet + nbNewFacets); i++) {
		file->ReadKeyword("facet");
		// Check idx
		size_t idx = file->ReadLLong();
		if (idx != i + 1 - *nbFacet) throw Error(file->MakeError("Wrong facet index !"));
		file->ReadKeyword("{");
		file->ReadKeyword("nbIndex");
		file->ReadKeyword(":");
		size_t nb = file->ReadLLong();

		if (nb < 3) {
			char errMsg[512];
			sprintf(errMsg, "Facet %zd has only %zd vertices. ", i, nb);
			throw Error(errMsg);
		}

		*(*facets + i) = new Facet(nb);
		(*facets)[i]->LoadGEO(file, version2, nbNewVertex);
		(*facets)[i]->selected = true;
		for (int j = 0; j < nb; j++)
			(*facets)[i]->indices[j] += *nbVertex;
		file->ReadKeyword("}");
		if (newStruct) {
			(*facets)[i]->sh.superIdx += sh.nbSuper;
			if ((*facets)[i]->sh.superDest > 0) (*facets)[i]->sh.superDest += sh.nbSuper;
		}
		else {

			(*facets)[i]->sh.superIdx += strIdx;
			if ((*facets)[i]->sh.superDest > 0) (*facets)[i]->sh.superDest += strIdx;
		}
	}

	*nbVertex += nbNewVertex;
	*nbFacet += nbNewFacets;
	if (newStruct) sh.nbSuper += nbNewSuper;
	else if (sh.nbSuper < strIdx + nbNewSuper) sh.nbSuper = strIdx + nbNewSuper;

}

void Geometry::InsertSTLGeom(FileReader *file, size_t *nbVertex, size_t *nbFacet, InterfaceVertex **vertices3, Facet ***facets, size_t strIdx, double scaleFactor, bool newStruct) {

	UnselectAll();
	char *w;

	int nbNewFacets = 0;
	// First pass
	file->ReadKeyword("solid");
	file->ReadLine(); // solid name
	w = file->ReadWord();
	while (strcmp(w, "facet") == 0) {
		nbNewFacets++;
		file->JumpSection("endfacet");
		w = file->ReadWord();
	}
	if (strcmp(w, "endsolid") != 0) throw Error("Unexpected or not supported STL keyword, 'endsolid' required");

	// Allocate memory
	int nbNewVertex = 3 * nbNewFacets;
	*facets = (Facet **)realloc(*facets, (nbNewFacets + *nbFacet) * sizeof(Facet **));
	memset(*facets + *nbFacet, 0, nbNewFacets * sizeof(Facet *));
	//*vertices3 = (Vector3d*)realloc(*vertices3,(nbNewVertex+*nbVertex) * sizeof(Vector3d));
	InterfaceVertex *tmp_vertices3 = (InterfaceVertex *)malloc((nbNewVertex + *nbVertex) * sizeof(InterfaceVertex));
	memmove(tmp_vertices3, *vertices3, (*nbVertex) * sizeof(InterfaceVertex));
	memset(tmp_vertices3 + *nbVertex, 0, nbNewVertex * sizeof(InterfaceVertex));
	SAFE_FREE(*vertices3);
	*vertices3 = tmp_vertices3;

	// Second pass
	file->SeekStart();
	file->ReadKeyword("solid");
	file->ReadLine();
	for (int i = 0; i < nbNewFacets; i++) {

		file->ReadKeyword("facet");
		file->ReadKeyword("normal");
		file->ReadDouble(); //ignoring normal vector, will be calculated from triangle orientation
		file->ReadDouble();
		file->ReadDouble();
		file->ReadKeyword("outer");
		file->ReadKeyword("loop");

		file->ReadKeyword("vertex");
		(*vertices3)[*nbVertex + 3 * i + 0].x = file->ReadDouble()*scaleFactor;
		(*vertices3)[*nbVertex + 3 * i + 0].y = file->ReadDouble()*scaleFactor;
		(*vertices3)[*nbVertex + 3 * i + 0].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("vertex");
		(*vertices3)[*nbVertex + 3 * i + 1].x = file->ReadDouble()*scaleFactor;
		(*vertices3)[*nbVertex + 3 * i + 1].y = file->ReadDouble()*scaleFactor;
		(*vertices3)[*nbVertex + 3 * i + 1].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("vertex");
		(*vertices3)[*nbVertex + 3 * i + 2].x = file->ReadDouble()*scaleFactor;
		(*vertices3)[*nbVertex + 3 * i + 2].y = file->ReadDouble()*scaleFactor;
		(*vertices3)[*nbVertex + 3 * i + 2].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("endloop");
		file->ReadKeyword("endfacet");

		*(*facets + i + *nbFacet) = new Facet(3);
		(*facets)[i + *nbFacet]->selected = true;
		(*facets)[i + *nbFacet]->indices[0] = *nbVertex + 3 * i + 0;
		(*facets)[i + *nbFacet]->indices[1] = *nbVertex + 3 * i + 1;
		(*facets)[i + *nbFacet]->indices[2] = *nbVertex + 3 * i + 2;

		if (newStruct) {
			(*facets)[i + *nbFacet]->sh.superIdx = sh.nbSuper;
		}
		else {
			(*facets)[i + *nbFacet]->sh.superIdx = strIdx;
		}
	}

	*nbVertex += nbNewVertex;
	*nbFacet += nbNewFacets;
	if (newStruct) AddStruct("Inserted STL file");
}

void Geometry::SaveSTR(Dataport *dpHit, bool saveSelected) {

	if (!IsLoaded()) throw Error("Nothing to save !");
	if (sh.nbSuper < 1) throw Error("Cannot save single structure in STR format");

	// Block dpHit during the whole disc writting

	for (int i = 0; i < sh.nbSuper; i++)
		SaveSuper(i);

}

void Geometry::SaveSuper(int s) {

	char fName[512];
	sprintf(fName, "%s/%s", strPath, strFileName[s]);
	FileWriter *file = new FileWriter(fName);

	// Unused
	file->Write(0, "\n");

	//Extract data of the specified super structure
	llong totHit = 0;
	llong totAbs = 0;
	llong totDes = 0;
	int *refIdx = (int *)malloc(sh.nbVertex * sizeof(int));
	memset(refIdx, 0xFF, sh.nbVertex * sizeof(int));
	int nbV = 0;
	int nbF = 0;

	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		if (f->sh.superIdx == s) {
			/*totHit += f->counterCache.nbHit;
			totAbs += f->counterCache.nbAbsorbed;
			totDes += f->counterCache.nbDesorbed;*/
			for (int j = 0; j < f->sh.nbIndex; j++)
				refIdx[f->indices[j]] = 1;
			nbF++;
		}
	}

	for (int i = 0; i < sh.nbVertex; i++) {
		if (refIdx[i] >= 0) {
			refIdx[i] = nbV;
			nbV++;
		}
	}

	file->Write(0, "\n");
	file->Write(0, "\n");
	file->Write(0, "\n");
	file->Write(0, "\n");

	file->Write(nbV, "\n");
	file->Write(nbF, "\n");

	// Read geometry vertices
	for (int i = 0; i < sh.nbVertex; i++) {
		if (refIdx[i] >= 0) {
			file->Write(vertices3[i].x, " ");
			file->Write(vertices3[i].y, " ");
			file->Write(vertices3[i].z, "\n");
		}
	}

	// Facets
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		int j;
		if (f->sh.superIdx == s) {
			file->Write(f->sh.nbIndex, " ");
			for (j = 0; j < f->sh.nbIndex - 1; j++)
				file->Write(refIdx[f->indices[j]] + 1, " ");
			file->Write(refIdx[f->indices[j]] + 1, "\n");
		}
	}

	// Params
	for (int i = 0; i < sh.nbFacet; i++) {

		// Update facet hits from shared mem
		Facet *f = facets[i];
		if (f->sh.superIdx == s) {
			f->SaveTXT(file);
		}

	}

	SaveProfileTXT(file);

	SAFE_DELETE(file);
	free(refIdx);

}

void Geometry::SaveProfileTXT(FileWriter *file) {
	// Profiles
	for (int j = 0; j < PROFILE_SIZE; j++)
		file->Write("\n");
}

bool Geometry::IsLoaded() {
	return isLoaded;
}

