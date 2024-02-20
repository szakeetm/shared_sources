/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/

#include "Geometry_shared.h"
#include "GeometryTools.h" //FindEar
#include "Worker.h"
#include "Helper/MathTools.h" //Min max
#include "GLApp/GLToolkit.h"
#include <string.h>
#include <math.h>
#include "GLApp/GLMatrix.h"
#include <tuple>
#include <Helper/GraphicsHelper.h>
#include <set>

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#include "Interface/Interface.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLList.h"
#include "Interface/SmartSelection.h"
#include "Interface/FacetCoordinates.h"
#include "Interface/VertexCoordinates.h"
#include "Facet_shared.h"

#include "ImguiWindow.h"

#if defined(MOLFLOW)
extern MolFlow* mApp;
#endif

#if defined(SYNRAD)
extern SynRad* mApp;
#endif

void InterfaceGeometry::SelectFacet(const size_t facetId) {
	if (!isLoaded) return;
	InterfaceFacet* f = facets[facetId];
	f->selected = (viewStruct == -1) || (viewStruct == f->sh.superIdx) || (f->sh.superIdx == -1);
	if (!f->selected) f->UnselectElem();
	selectHist = { facetId }; //reset with only this facet id
}

void InterfaceGeometry::SelectArea(const int x1, const int y1, const int x2, const int y2, const bool clear, const bool unselect, const bool vertexBound, const bool circularSelection) {

	// Select a set of facet according to a 2D bounding rectangle
	// (x1,y1) and (x2,y2) are in viewport coordinates

	float r2;
	int _x1, _y1, _x2, _y2;

	_x1 = std::min(x1, x2);
	_x2 = std::max(x1, x2);
	_y1 = std::min(y1, y2);
	_y2 = std::max(y1, y2);

	if (circularSelection) {
		r2 = pow((float)(x1 - x2), 2) + pow((float)(y1 - y2), 2);
	}



	if (clear && !unselect) UnselectAll();
	selectHist.clear();
	int lastPaintedProgress = -1;
	int paintStep = (int)((double)sh.nbFacet / 10.0);

	std::vector<std::optional<ScreenCoord>> screenCoords = TransformVerticesToScreenCoords(false);

	//Check facets if all vertices inside
	std::vector<size_t> facetIds_vect;
	{
		std::unordered_set<size_t> facetIds; //vertices of facets in current struct
#pragma omp parallel
		{
			std::unordered_set<size_t> facetIds_local;
#pragma omp for
			for (int i = 0; i < sh.nbFacet; i++) {

				InterfaceFacet* f = facets[i];
				size_t nb = f->sh.nbIndex;
				bool isInside = true;
				size_t j = 0;
				bool hasSelectedVertex = false;
				while (j < nb && isInside) {
					size_t idx = f->indices[j];
					if (!screenCoords[idx].has_value()) {
						isInside = false;
					}
					else {
						if (!circularSelection) {
							isInside = (screenCoords[idx]->x >= _x1) && (screenCoords[idx]->x <= _x2) && (screenCoords[idx]->y >= _y1) && (screenCoords[idx]->y <= _y2);
						}
						else { //circular selection
							isInside = (pow((float)(screenCoords[idx]->x - x1), 2) + pow((float)(screenCoords[idx]->y - y1), 2)) <= r2;
						}
						if (vertices3[idx].selected) hasSelectedVertex = true;
						j++;
					}
				}
				if (isInside && (!vertexBound || hasSelectedVertex)) {
					facetIds_local.insert(i);
				}

			}
#pragma omp critical
			facetIds.insert(facetIds_local.begin(), facetIds_local.end());
		}
		//Convert unordered set to vector for OpenMP parallel for
		facetIds_vect.reserve(facetIds.size());
		for (auto id : facetIds) facetIds_vect.push_back(id);
	}

	//Process found facets
#pragma omp parallel for
	for (int id = 0; id < facetIds_vect.size();id++) {
		size_t i = facetIds_vect[id];
		if (!unselect) {
			facets[i]->selected = !unselect;
		}
		else {
			facets[i]->selected = !unselect;
		}
	}
	mApp->SetFacetSearchPrg(false, "");
	UpdateSelection();
}

void InterfaceGeometry::Select(const int x, const int y, const int width, const int height, const bool clear, const bool unselect, const bool vertexBound) {
	// Select a facet on a mouse click in 3D perspective view 
	constexpr bool printDebugInfo = false; //error-prone parallel search, set to true to print

	if (!isLoaded) return;

	//Transform all vertex coordinates to screen coordinates
	std::vector<std::optional<ScreenCoord>> screenCoords = TransformVerticesToScreenCoords(printDebugInfo);

	std::set<size_t> foundInSelectionHistory; //facets that are under the mouse pointer but have been selected before
	
	if (printDebugInfo) {
		//debug
		fmt::print("\nSelection history before search: {{");
		for (const auto s : selectHist)
			fmt::print("{},", s + 1);
		fmt::print("}}\n");
	}

	int global_solution_facetId = -1; //facet id that was found and not yet in sel. history
#pragma omp parallel
	{
			int found_local_facetId = -1; //-1: not found, otherwise id of facet
			
#pragma omp for
		for (int i = 0; i < sh.nbFacet; i++) {
			if (global_solution_facetId >= 0) continue; //this or an other thread have found a valid facet, don't look for more

			if (viewStruct == -1 || facets[i]->sh.superIdx == viewStruct || facets[i]->sh.superIdx == -1) {

				bool hasVertexBehindCamera = false; //can't do polygon intersection check
				bool hasVertexOnScreen = false;
				bool hasSelectedVertex = false;
				// Build array of 2D points
				std::vector<Vector2d> facetScreenCoords(facets[i]->indices.size());

				for (int j = 0; j < facets[i]->indices.size() && !hasVertexBehindCamera; j++) {
					size_t idx = facets[i]->indices[j];
					if (screenCoords[idx].has_value()) {
						facetScreenCoords[j] = Vector2d((double)screenCoords[idx]->x, (double)screenCoords[idx]->y);
						if (screenCoords[idx]->x >= 0 && screenCoords[idx]->x < width && screenCoords[idx]->y >= 0 && screenCoords[idx]->y < height) {
							hasVertexOnScreen = true;
						}
					} else {
						hasVertexBehindCamera = true;
					}
				}
				if (vertexBound) { //CAPS LOCK on, select facets only with at least one seleted vertex
					for (size_t j = 0; j < facets[i]->indices.size() && (!hasSelectedVertex); j++) {
						size_t idx = facets[i]->indices[j];
						if (vertices3[idx].selected) hasSelectedVertex = true;
					}
				}

				if (!hasVertexBehindCamera) { //facetScreenCoords all valid
					if (hasVertexOnScreen) { //at least one vertex visible to user
						if (!vertexBound || hasSelectedVertex) { //CAPS LOCK off or has selected vertex
							if (unselect || !facets[i]->selected) { //In "Add" mode, ignore already selected

								if (IsInPoly((double)x, (double)y, facetScreenCoords)) {
									found_local_facetId = i;
									if (unselect) {
										if ((!mApp->smartSelection || !mApp->smartSelection->IsSmartSelection()) && (mApp->imWnd ? (!mApp->imWnd->smartSelect.IsVisible() || !mApp->imWnd->smartSelect.IsEnabled()) : true)) {
											facets[i]->selected = false;
											if (printDebugInfo) {
#pragma omp critical
												fmt::print("Thread {}/{}: deselecting facet {} and looking for more under the pointer.\n", omp_get_thread_num() + 1, omp_get_num_threads(), found_local_facetId + 1);
											}
											found_local_facetId = -1; //Skip processing and look for more
										}
										else { //Smart selection
											double maxAngleDiff;
											if (mApp->imWnd && mApp->imWnd->smartSelect.IsEnabled() && mApp->imWnd->smartSelect.IsVisible()) {
												maxAngleDiff = mApp->imWnd->smartSelect.GetMaxAngle();
											}
											else
												maxAngleDiff = mApp->smartSelection->GetMaxAngle();
											std::vector<size_t> connectedFacets;
											if (maxAngleDiff >= 0.0) connectedFacets = GetConnectedFacets(i, maxAngleDiff);
											for (auto& ind : connectedFacets)
												facets[ind]->selected = false;
										}
									} //end unselect
									if (found_local_facetId >= 0) { //false if unselect mode without smart selection
										if (InSelectionHistory(i)) {
											if (printDebugInfo) {
#pragma omp critical
												fmt::print("Thread {}/{}: found facet {} already in sel.history, looking for better candidate.\n", omp_get_thread_num() + 1, omp_get_num_threads(), i + 1);
											}
										}
										else { //found a facet that wasn't in the selected history
#pragma omp critical
											{
												global_solution_facetId = i; //set global found flag to true, all threads will skip every facet check after this. Race condition allowed, accept any thread's result
												if (printDebugInfo) {
													fmt::print("Thread {}/{}: found facet {} not in history, setting global found flag to true, others will skip\n", omp_get_thread_num() + 1, omp_get_num_threads(), i + 1);
												}
											}
										}
									}
								} //end "inPoly"
							} //if not already selected or unselect mode
						} //if CAPS LOCK not on or has selected vertex
					} //if has at least a vertex on screen
				} //if doesn't have vertex that couldn't be transformed (behind camera)
			} //if in current (or all) structure
		} //parallel for loop on facets

//process results, still in parallel region to keep found_local_facetId
#pragma omp critical
		if (!unselect) {
			if (global_solution_facetId==-1) {

				if (printDebugInfo) fmt::print("Thread {}/{} finished, found_local_facetId={}", omp_get_thread_num() + 1, omp_get_num_threads(), found_local_facetId + 1);

					if (found_local_facetId >= 0) { //This thread has a facet already in the selection history
						foundInSelectionHistory.insert(found_local_facetId); //we'll process later
						if (printDebugInfo) fmt::print(" - no global result, add this result to candidates in selection history\n");
					}
					else if (printDebugInfo) {
						fmt::print(" - thread had no result\n");
					}
				
			} else if (printDebugInfo) {
				fmt::print("Thread {}/{} finished, found_local_facetId={}, not processing as there's a global result.\n", omp_get_thread_num() + 1, omp_get_num_threads(), found_local_facetId + 1);
			}
		} else if (printDebugInfo) {
			fmt::print("Thread {}/{} finished, found_local_facetId={}, not processing as unselect mode.\n", omp_get_thread_num() + 1, omp_get_num_threads(), found_local_facetId + 1);
		}
	} //thread results processed, end parallel region

	if (clear && !unselect) UnselectAll();

	if (!unselect) {
		if (global_solution_facetId >= 0) { //Found a new facet
			if (!unselect) AddToSelectionHist(global_solution_facetId);
			if (printDebugInfo) fmt::print("Added facet {} to selection history\n", global_solution_facetId + 1);

			TreatNewSelection(global_solution_facetId, unselect);
		} else if (global_solution_facetId == -1 && foundInSelectionHistory.size() > 0) { //found at least one that was already selected previously
			
			size_t lastIndex; //selection: oldest in the history. Unselection: newest in the history

			if (printDebugInfo) {
				//debug
				fmt::print("\nNo new facet found, choosing from candidates already in history: {{");
				for (const auto s : foundInSelectionHistory)
					fmt::print("{},", s + 1);
				fmt::print("}}\n");
			}

			if (!unselect) { //select
				for (int s = 0; s < selectHist.size(); s++) { //find the oldest in the history that's under our pointer
					if (foundInSelectionHistory.count(selectHist[s])) {
						lastIndex = selectHist[s];
						if (printDebugInfo) fmt::print("Choosing facet {} as oldest in the history.\n", lastIndex + 1);
						break;
					}
				}
				selectHist = { (size_t)lastIndex }; //reset
				if (printDebugInfo) fmt::print("Resetting selection history to: {{{}}}.\n", lastIndex + 1);
				TreatNewSelection(lastIndex, unselect);
			}
			/*
			else { //unselect last selected facet
				for (int s = selectHist.size()-1; s >=0; s--) { //find the newest in the history that's under our pointer
					if (foundInSelectionHistory.count(selectHist[s]) && facets[selectHist[s]]->selected) {
						lastIndex = selectHist[s];
						break;
					}
				}
			}*/


		} else { //not found at all
			if (printDebugInfo) fmt::print("Nothing found, not even in selection history, cleared selection history.\n");
			selectHist.clear();
		}
	}
	
	//fmt::print("Found:{} Lastfound:{} Selection history:", found, lastFound_facetId);
	//for (auto i : selectHist) fmt::print("{} ", i);
	//fmt::print("\n\n");
	UpdateSelection();

}

void InterfaceGeometry::TreatNewSelection(int lastFound, bool unselect) //helper to avoid duplicate code
{
	if ((!mApp->smartSelection || !mApp->smartSelection->IsSmartSelection()) && (mApp->imWnd ? (!mApp->imWnd->smartSelect.IsVisible() || !mApp->imWnd->smartSelect.IsEnabled()) : true)) {
		facets[lastFound]->selected = !unselect;
	}
	else { //Smart selection
		double maxAngleDiff;
		if (mApp->imWnd && mApp->imWnd->smartSelect.IsEnabled() && mApp->imWnd->smartSelect.IsVisible()) {
			maxAngleDiff = mApp->imWnd->smartSelect.GetMaxAngle();
		}
		else
			maxAngleDiff = mApp->smartSelection->GetMaxAngle();
		std::vector<size_t> connectedFacets;
		mApp->SetFacetSearchPrg(true, "Smart selecting...");
		if (maxAngleDiff >= 0.0) connectedFacets = GetConnectedFacets(lastFound, maxAngleDiff);
		for (auto& ind : connectedFacets)
			facets[ind]->selected = !unselect;
		mApp->SetFacetSearchPrg(false, "");
	}
	if (!unselect) mApp->facetList->ScrollToVisible(lastFound, 0, true); //scroll to selected facet
}

void InterfaceGeometry::SelectVertex(const int vertexId) {
	//isVertexSelected[vertexId] = (viewStruct==-1) || (viewStruct==f->sp.superIdx);
	//here we should look through facets if vertex is member of any
	//if( !f->selected ) f->UnselectElem();
	if (!isLoaded) return;
	vertices3[vertexId].selected = true;
}

void InterfaceGeometry::SelectVertex(int x1, int y1, int x2, int y2, bool shiftDown, bool ctrlDown, bool circularSelection, bool facetBound) {

	// Select a set of vertices according to a 2D bounding rectangle
	// (x1,y1) and (x2,y2) are in viewport coordinates

	float r2;
	int _x1, _y1, _x2, _y2;

	_x1 = std::min(x1, x2);
	_x2 = std::max(x1, x2);
	_y1 = std::min(y1, y2);
	_y2 = std::max(y1, y2);

	if (circularSelection) {
		r2 = pow((float)(x1 - x2), 2) + pow((float)(y1 - y2), 2);
	}

	GLMatrix view, proj, mvp;
	GLVIEWPORT viewPort;
	std::tie(view, proj, mvp, viewPort) = GLToolkit::GetCurrentMatrices();

	if (!ctrlDown && !shiftDown) {
		UnselectAllVertex(); EmptySelectedVertexList();
		//nbSelectedHistVertex = 0;
	}

	std::unordered_set<int> selectedFacetsVertices;
	if (facetBound) selectedFacetsVertices = GetVertexBelongsToSelectedFacet();

	std::unordered_set<int> vertices_local, vertices_global;
#pragma omp parallel private(vertices_local)
	{
#pragma omp for
		for (int i = 0; i < sh.nbVertex; i++) {
			if (facetBound && selectedFacetsVertices.count(i) == 0) continue; //doesn't belong to selected facet
			Vector3d* v = GetVertex(i);

			bool isInside = false;
			int idx = i;
			std::optional<ScreenCoord> screenCoords = GLToolkit::Get2DScreenCoord_fast(vertices3[i], mvp, viewPort);
			if (screenCoords.has_value()) { //otherwise ignore
				if (!circularSelection) {
					isInside = (screenCoords->x >= _x1) && (screenCoords->x <= _x2) && (screenCoords->y >= _y1) && (screenCoords->y <= _y2);
				}
				else {//circular selection
					isInside = (pow((float)(screenCoords->x - x1), 2) + pow((float)(screenCoords->y - y1), 2)) <= r2;
				}
			}

			if (isInside) {
				vertices_local.insert(i);
			}
		}
#pragma omp critical
		vertices_global.insert(vertices_local.begin(), vertices_local.end());
	} //end parallel region

	for (auto index : vertices_global) {
		vertices3[index].selected = !ctrlDown;
		if (ctrlDown) RemoveFromSelectedVertexList(index);
		else {
			AddToSelectedVertexList(index);
			if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateId(index);
		}
	}

	if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
}

void InterfaceGeometry::SelectVertex(int x, int y, int width, int height, bool shiftDown, bool ctrlDown, bool facetBound) {

	if (!isLoaded) return;

	// Select a vertex on a mouse click in 3D perspectivce view 
	// (x,y) are in screen coordinates

	std::unordered_set<int> selectedFacetsVertices;
	if (facetBound) selectedFacetsVertices = GetVertexBelongsToSelectedFacet();

	std::vector<std::optional<ScreenCoord>> screenCoords = TransformVerticesToScreenCoords(false);

	//Get Closest Point to click
	double minDist_global = std::numeric_limits<double>::max();
	int minId_global = -1;

#pragma omp parallel
	{
		//These will be private
		double minDist_local = std::numeric_limits<double>::max();
		int minId_local = -1;
#pragma omp for
		for (int i = 0; i < sh.nbVertex; i++) {
			if (facetBound && selectedFacetsVertices.count(i) == 0) continue; //doesn't belong to selected facet
			if (screenCoords[i].has_value()) {
				if (screenCoords[i]->x >= 0 && screenCoords[i]->x < width && screenCoords[i]->y >= 0 && screenCoords[i]->y < height) { //calculate only for points on screen
					double distanceSqr = pow((double)(screenCoords[i]->x - x), 2) + pow((double)(screenCoords[i]->y - y), 2);
					if (distanceSqr < minDist_local) {
						minDist_local = distanceSqr;
						minId_local = i;
					}
				}
			}
		}

		// After the parallel loop, combine the local minDist and minId to find the overall minimum
#pragma omp critical
		{
			if (minDist_local < minDist_global) {
				minDist_global = minDist_local;
				minId_global = minId_local;
			}
		}
	}

	if (!ctrlDown && !shiftDown) {
		UnselectAllVertex(); EmptySelectedVertexList();
		//nbSelectedHistVertex = 0;
	}

	if (minDist_global < 250.0) {
		vertices3[minId_global].selected = !ctrlDown;
		if (ctrlDown) RemoveFromSelectedVertexList(minId_global);
		else {
			AddToSelectedVertexList(minId_global);
			if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateId(minId_global);
			//nbSelectedHistVertex++;
		}
	}

	//UpdateSelection();
	if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
}

void InterfaceGeometry::AddToSelectionHist(size_t f) {
	selectHist.push_back(f);
}

bool InterfaceGeometry::InSelectionHistory(size_t f) {
	return std::find(selectHist.begin(), selectHist.end(), f) != selectHist.end();
}

std::optional<size_t> InterfaceGeometry::GetLastSelected()
{
	if (selectHist.empty()) return false;
	else return selectHist.back();
}

std::optional<size_t> InterfaceGeometry::GetFirstSelected()
{
	if (selectHist.empty()) return false;
	else return selectHist.front();
}

void InterfaceGeometry::SelectAll() {
	for (int i = 0; i < sh.nbFacet; i++)
		SelectFacet(i);
	UpdateSelection();
}

void InterfaceGeometry::EmptySelectedVertexList() {
	selectedVertexList_ordered.clear();
}

void InterfaceGeometry::RemoveFromSelectedVertexList(size_t vertexId) {
	selectedVertexList_ordered.erase(std::remove(selectedVertexList_ordered.begin(), selectedVertexList_ordered.end(), vertexId), selectedVertexList_ordered.end());
}

void InterfaceGeometry::AddToSelectedVertexList(size_t vertexId) {
	selectedVertexList_ordered.push_back(vertexId);
}

void InterfaceGeometry::SelectAllVertex() {
#pragma omp parallel for
	for (int i = 0; i < sh.nbVertex; i++)
		SelectVertex(i);
	//UpdateSelectionVertex();
}

size_t InterfaceGeometry::GetNbSelectedVertex() {
	size_t nbSelectedVertex = 0;
	for (int i = 0; i < sh.nbVertex; i++) {
		if (vertices3[i].selected) nbSelectedVertex++;
	}
	return nbSelectedVertex;
}

void InterfaceGeometry::UnselectAll() {
#pragma omp parallel for
	for (int i = 0; i < sh.nbFacet; i++) {
		facets[i]->selected = false;
		facets[i]->UnselectElem();
	}
	UpdateSelection();
}

void InterfaceGeometry::UnselectAllVertex() {
#pragma omp parallel for
	for (int i = 0; i < sh.nbVertex; i++) {
		vertices3[i].selected = false;
		//facets[i]->UnselectElem(); //what is this?
	}
	//UpdateSelectionVertex();
}

std::vector<size_t> InterfaceGeometry::GetSelectedVertices()
{
	std::vector<size_t> sel;
	for (size_t i = 0; i < sh.nbVertex; i++)
		if (vertices3[i].selected) sel.push_back(i);
	return sel;
}

void InterfaceGeometry::DrawFacetWireframe(const InterfaceFacet* f, bool offset, bool selOffset) {

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
			glEdgeFlag(true);
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
			for (j = 0; j < nb; j++) {
					i1 = f->indices[j];
					i2 = f->indices[Next(j,nb)];
					glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
					glVertex3d(vertices3[i2].x, vertices3[i2].y, vertices3[i2].z);
			}			
			glEnd();
		}
	}

}

void InterfaceGeometry::DrawFacetWireframe_Vertexarray(const InterfaceFacet* f, std::vector<GLuint>& lines) {

	// Render a facet (wireframe)
	int nb = f->sh.nbIndex;
	for (int j = 0; j < nb; j++) {
		lines.emplace_back((GLuint)f->indices[j]);
		lines.emplace_back((GLuint)f->indices[(j + 1) % nb]);
	}
}


void InterfaceGeometry::DrawPolys() {

	std::vector<int> f3; f3.reserve(sh.nbFacet);
	std::vector<int> fp; fp.reserve(sh.nbFacet);

	// Group TRI,QUAD and POLY
	for (int i = 0; i < sh.nbFacet; i++) {
		int nb = facets[i]->sh.nbIndex;
		if (facets[i]->viewSettings.volumeVisible) {
			if (nb == 3) {
				f3.emplace_back(i);
			}
			else {
				fp.emplace_back(i);
			}
		}
	}

	// Draw
	std::vector<double> vertexCoords, normalCoords;
	std::vector<float> textureCoords, colorValues;
	GLCOLOR currentColor; //unused
	currentColor.r = 0.0f;
	currentColor.g = 0.0f;
	currentColor.b = 0.0f;
	currentColor.a = 0.0f;

#pragma omp parallel
	{
		std::vector<double> vertexCoords_local, normalCoords_local;
		std::vector<float> textureCoords_local, colorValues_local;
		// Triangle
#pragma omp for
		for (int i = 0; i < f3.size(); i++) {
			int j = f3[i];
			FillFacet(facets[j], vertexCoords_local, normalCoords_local, textureCoords_local, colorValues_local, currentColor, false);
		}
#pragma omp critical
		{
			vertexCoords.insert(vertexCoords.end(), vertexCoords_local.begin(), vertexCoords_local.end());
			normalCoords.insert(normalCoords.end(), normalCoords_local.begin(), normalCoords_local.end());
			textureCoords.insert(textureCoords.end(), textureCoords_local.begin(), textureCoords_local.end());
			colorValues.insert(colorValues.end(), colorValues_local.begin(), colorValues_local.end());
		}
	}

#pragma omp parallel
	{
		std::vector<double> vertexCoords_local, normalCoords_local;
		std::vector<float> textureCoords_local, colorValues_local;
		// Triangulate polygon
#pragma omp for
		for (int i = 0; i < fp.size(); i++) {
			int j = fp[i];
			TriangulateForRender(facets[j], vertexCoords_local, normalCoords_local, textureCoords_local, colorValues_local, currentColor, false);
		}
#pragma omp critical
		{
			vertexCoords.insert(vertexCoords.end(), vertexCoords_local.begin(), vertexCoords_local.end());
			normalCoords.insert(normalCoords.end(), normalCoords_local.begin(), normalCoords_local.end());
			textureCoords.insert(textureCoords.end(), textureCoords_local.begin(), textureCoords_local.end());
			colorValues.insert(colorValues.end(), colorValues_local.begin(), colorValues_local.end());
		}
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_DOUBLE, 0, vertexCoords.data());
	glNormalPointer(GL_DOUBLE, 0, normalCoords.data());

	glDrawArrays(GL_TRIANGLES, 0, vertexCoords.size() / 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

// returns 1 if lhs is greater, -1 otherwise. unused
float InterfaceGeometry::getMaxDistToCamera(InterfaceFacet* f) {

	float rx, ry, rz, rw;

	GLfloat mProj[16];
	GLfloat mView[16];
	GLVIEWPORT g;

	glGetFloatv(GL_PROJECTION_MATRIX, mProj);
	glGetFloatv(GL_MODELVIEW_MATRIX, mView);
	glGetIntegerv(GL_VIEWPORT, (GLint*)&g);

	GLMatrix proj; proj.LoadGL(mProj);
	GLMatrix view; view.LoadGL(mView);
	GLMatrix m; m.Multiply(&proj, &view);
	float distToCamera = -99999.99f;

	for (int i = 0; i < f->sh.nbIndex; ++i) {
		size_t idx = f->indices[i];
		m.TransformVec((float)vertices3[idx].x, (float)vertices3[idx].y, (float)vertices3[idx].z, 1.0f,
			&rx, &ry, &rz, &rw);
		distToCamera = std::max(rz, distToCamera);
	}
	return distToCamera;
}

// returns 1 if lhs is greater, -1 otherwise. unused
int InterfaceGeometry::compareFacetDepth(InterfaceFacet* lhs, InterfaceFacet* rhs) {

	if (getMaxDistToCamera(lhs) > getMaxDistToCamera(rhs)) {
		return 1;
	}
	else return 0;

}
void InterfaceGeometry::DrawSemiTransparentPolys(const std::vector<size_t>& selectedFacets) {

	const auto colorHighlighting = mApp->worker.GetGeometry()->GetPlottedFacets(); // For colors
	// Draw
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	struct ArrowToDraw {
		Vector3d startPoint, endPoint, parallel;
		std::array<float, 4> color; //to pass components to glColor4f
	};
	std::vector<ArrowToDraw> arrowsToDraw;
	std::vector<double> vertexCoords, normalCoords;
	std::vector<float> textureCoords, colorValues;
	GLCOLOR currentColor;

	for (const auto sel : selectedFacets) {
		if (!colorHighlighting.empty() && (mApp->profilePlotter != nullptr && ((GLWindow*)(mApp->profilePlotter))->IsVisible())) {
			auto it = colorHighlighting.find(sel);
			// Check if element exists in map or not
			auto profileMode = facets[sel]->sh.profileType;
			ArrowToDraw arrow;
			if (it != colorHighlighting.end()) {
				//Plotted
				float r = static_cast<float>(it->second.r) / 255.0f;
				float g = static_cast<float>(it->second.g) / 255.0f;
				float b = static_cast<float>(it->second.b) / 255.0f;
				GLCOLOR newColor;
				newColor.r = r;
				newColor.g = g;
				newColor.b = b;
				newColor.a = .5f;
				currentColor = newColor;
				arrow.color = { r, g, b, 1.0f };
				if (profileMode == PROFILE_U || profileMode == PROFILE_V) {
					//Profiled, add arrow
					Vector3d& center = facets[sel]->sh.center;
					Vector3d& dir = profileMode == PROFILE_U ? facets[sel]->sh.U : facets[sel]->sh.V;
					arrow.startPoint = center - .5 * dir;
					arrow.endPoint = center + .5 * dir;
					arrow.parallel = profileMode == PROFILE_U ? facets[sel]->sh.nV : facets[sel]->sh.nU;
					arrowsToDraw.push_back(arrow);
				}
			}
			else {
				//Not plotted
				GLCOLOR metroRedLight;
				metroRedLight.r = 0.933f;
				metroRedLight.g = 0.067f;
				metroRedLight.b = 0.067f;
				metroRedLight.a = 0.05f;
				currentColor = metroRedLight;
				//arrow.color = { 1.0f,1.0f,1.0f, 0.3f };
			}

		}
		else {
			//No highlighting, regular selected facet
			GLCOLOR metroRed;
			metroRed.r = 0.933f;
			metroRed.g = 0.067f;
			metroRed.b = 0.067f;
			metroRed.a = 0.15f;
			currentColor = metroRed; 
		}
		size_t nb = facets[sel]->sh.nbIndex;
		if (nb == 3) {
			FillFacet(facets[sel], vertexCoords, normalCoords, textureCoords, colorValues, currentColor, false);
		}
		else {
			TriangulateForRender(facets[sel], vertexCoords, normalCoords, textureCoords, colorValues, currentColor, false);
		}
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_DOUBLE, 0, vertexCoords.data());
	glNormalPointer(GL_DOUBLE, 0, normalCoords.data());
	glColorPointer(4, GL_FLOAT, 0, colorValues.data());

	glDrawArrays(GL_TRIANGLES, 0, vertexCoords.size() / 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	AxisAlignedBoundingBox bb = GetBB();
	double arrowLength = 30.0 / std::max((bb.max.x - bb.min.x), (bb.max.y - bb.min.y));

	//glPushAttrib(GL_ENABLE_BIT);
	//glLineStipple(2, 0xAAAA);
	//glEnable(GL_LINE_STIPPLE);
	//glLineWidth(2.0f);
	for (const auto& arr : arrowsToDraw) {
		glColor4f(arr.color[0], arr.color[1], arr.color[2], arr.color[3]);
		GLToolkit::DrawVector(arr.startPoint, arr.endPoint, arr.parallel);
	}
	//glPopAttrib();
	//---end transparent
}

void InterfaceGeometry::SetCullMode(VolumeRenderMode mode) {

	switch (mode) {
	case VolumeRenderMode::FrontOnly:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	case VolumeRenderMode::BackOnly:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	case VolumeRenderMode::FrontAndBack:
		glDisable(GL_CULL_FACE);
	}

}

void InterfaceGeometry::ClearFacetTextures()
{
	GLProgress_GUI prg = GLProgress_GUI("Clearing texture", "Frame update");
	prg.SetBounds(5, 28, 300, 90);
	int startTime = SDL_GetTicks();
	for (int i = 0; i < sh.nbFacet; i++) {
		if (!prg.IsVisible() && ((SDL_GetTicks() - startTime) > 500)) {
			prg.SetVisible(true);
		}
		prg.SetProgress((double)i / (double)sh.nbFacet);
		facets[i]->glTex = std::make_unique<GLTextureWrapper>();
	}
}

void InterfaceGeometry::RenderArrow(GLfloat* matView, float dx, float dy, float dz, float px, float py, float pz, float d) {

	if (!arrowList) BuildDirectionList();

	// Compute transformation matrix for the arrow
	GLMatrix mView;
	GLMatrix aView;
	GLMatrix mScale;
	GLMatrix mRot;
	GLMatrix mT;
	float v1x, v1y, v1z;

	mView.LoadGL(matView);

	// Direction
	float n = sqrtf(dx * dx + dy * dy + dz * dz);
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
		glCallList(sphereList->listId);
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

	float n1 = sqrtf(v1x * v1x + v1y * v1y + v1z * v1z);
	v1x /= n1;
	v1y /= n1;
	v1z /= n1;
	mRot._12 = v1x;
	mRot._22 = v1y;
	mRot._32 = v1z;

	// Cross product
	mRot._13 = (dy) * (v1z)-(dz) * (v1y);
	mRot._23 = (dz) * (v1x)-(dx) * (v1z);
	mRot._33 = (dx) * (v1y)-(dy) * (v1x);

	// Scale
	if (!autoNorme) {
		mScale._11 = (n * d * normeRatio);
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
	glCallList(arrowList->listId);

}

// Triangulation stuff

void InterfaceGeometry::AddTextureCoord(const InterfaceFacet* f, const Vector2d& p, std::vector<float>& textureCoords) {

	// Add texture coord with a 1 texel border (for bilinear filtering)
	double uStep = 1.0 / (double)f->texDimW;
	double vStep = 1.0 / (double)f->texDimH;

#if 1
	double fu = f->sh.texWidth_precise * uStep;
	double fv = f->sh.texHeight_precise * vStep;
	textureCoords.push_back((float)(uStep + p.u * fu));
	textureCoords.push_back((float)(vStep + p.v * fv));
#else
	// Show border (debugging purpose)
	double fu = (f->sh.texWidth_precise + 2.0) * uStep;
	double fv = (f->sh.texHeight_precise + 2.0) * vStep;
	textureCoords.push_back((float)(uStep + p.u * fu));
	textureCoords.push_back((float)(vStep + p.v * fv));
#endif

}

void InterfaceGeometry::FillFacet(const InterfaceFacet* f, std::vector<double>& vertexCoords, std::vector<double>& normalCoords, std::vector<float>& textureCoords, std::vector<float>& colorValues, const GLCOLOR& currentColor, bool addTextureCoord) {

	for (size_t i = 0; i < f->sh.nbIndex; i++) {
		size_t idx = f->indices[i];
		if (addTextureCoord) AddTextureCoord(f, f->vertices2[i], textureCoords);
		vertexCoords.push_back(vertices3[idx].x);
		vertexCoords.push_back(vertices3[idx].y);
		vertexCoords.push_back(vertices3[idx].z);
		normalCoords.push_back(-f->sh.N.x);
		normalCoords.push_back(-f->sh.N.y);
		normalCoords.push_back(-f->sh.N.z);
		colorValues.push_back(currentColor.r);
		colorValues.push_back(currentColor.g);
		colorValues.push_back(currentColor.b);
		colorValues.push_back(currentColor.a);
	}
}

void InterfaceGeometry::DrawEar(const InterfaceFacet* f, const GLAppPolygon& p, int ear, std::vector<double>& vertexCoords, std::vector<double>& normalCoords, std::vector<float>& textureCoords, std::vector<float>& colorValues, const GLCOLOR& currentColor, bool addTextureCoord) {

	//Commented out sections: theoretically in a right-handed system the vertex order is inverse
	//However we'll solve it simpler by inverting the geometry viewer Front/back culling mode setting

	Vector3d  p3D;
	const Vector2d* points[] = {
		&p.pts[Previous(ear, p.pts.size())],
		&p.pts[IDX(ear, p.pts.size())],
		&p.pts[Next(ear, p.pts.size())]
	};

	for (auto p : points) {
		normalCoords.push_back(-f->sh.N.x);
		normalCoords.push_back(-f->sh.N.y);
		normalCoords.push_back(-f->sh.N.z);
		if (addTextureCoord) AddTextureCoord(f, *p, textureCoords);
		f->glVertex2uVertexArray(p->u, p->v, vertexCoords);
		colorValues.push_back(currentColor.r);
		colorValues.push_back(currentColor.g);
		colorValues.push_back(currentColor.b);
		colorValues.push_back(currentColor.a);
	}
}

void InterfaceGeometry::TriangulateForRender(const InterfaceFacet* f, std::vector<double>& vertexCoords, std::vector<double>& normalCoords, std::vector<float>& textureCoords, std::vector<float>& colorValues, const GLCOLOR& currentColor, bool addTextureCoord) {

	// Triangulate a facet (rendering purpose)
	// The facet must have at least 3 points
	// Use the very simple "Two-Ears" theorem. It computes in O(n^2).

	if (f->nonSimple) {
		// Not a simple polygon
		// Abort triangulation
		return;
	}

	// Build a Polygon
	GLAppPolygon p;
	p.pts = f->vertices2;
	//p.sign = f->sign;

	// Perform triangulation
	while (p.pts.size() > 3) {
		int e = GeometryTools::FindEar(p);
		DrawEar(f, p, e, vertexCoords, normalCoords, textureCoords, colorValues, currentColor, addTextureCoord);
		// Remove the ear
		p.pts.erase(p.pts.begin() + e);
	}

	// Draw the last ear
	DrawEar(f, p, 0, vertexCoords, normalCoords, textureCoords, colorValues, currentColor, addTextureCoord);

}

void InterfaceGeometry::Render(GLfloat* matView, bool renderVolume, bool renderTexture, VolumeRenderMode volumeRenderMode, bool filter, bool showHiddenFacet, bool showMesh, bool showDir, bool clippingEnabled) {

	if (!isLoaded) return;

	// Render the geometry
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	// Render Volume
	if (renderVolume) {
		glPolygonOffset(1.0f, 4.0f);
		SetCullMode(volumeRenderMode);
		GLfloat global_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHTING);
		GLToolkit::SetMaterial(&fillMaterial);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glCallList(polyList->listId);
		glDisable(GL_POLYGON_OFFSET_FILL);
		GLToolkit::SetMaterial(&whiteMaterial);
		glDisable(GL_LIGHTING);
	}
	else {
		// Default material
		GLToolkit::SetMaterial(&whiteMaterial);

		// Draw lines
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);

		float color = (mApp->whiteBg) ? 0.0f : 1.0f; //whitebg here
		if (viewStruct == -1) {
			glColor4f(color, color, color, .8f);
			if (mApp->antiAliasing) {
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			}
			glPolygonOffset(1.0f, 5.0f);
			glEnable(GL_POLYGON_OFFSET_LINE);

			for (auto& lineList : lineLists) {
				glCallList(lineList.listId);
			}

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
			for (int i = 0; i < sh.nbSuper; i++)
				if (viewStruct != i)
					glCallList(lineLists[i].listId);
			// Selectable in white
			glColor3f(color, color, color);
			glPolygonOffset(1.0f, 5.0f);
			glEnable(GL_POLYGON_OFFSET_LINE);
			glCallList(lineLists[viewStruct].listId);
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
		for (size_t i = 0; i < sh.nbFacet && renderTexture; i++) {
			InterfaceFacet* f = facets[i];
			bool paintRegularTexture = f->sh.isTextured && f->viewSettings.textureVisible && (f->sh.countAbs || f->sh.countRefl || f->sh.countTrans);
#if defined(MOLFLOW)
			paintRegularTexture = paintRegularTexture || (f->sh.isTextured && f->viewSettings.textureVisible && (f->sh.countACD || f->sh.countDes));
#endif
			if (paintRegularTexture) {
				if (f->sh.is2sided)   glDisable(GL_CULL_FACE);
				else                   SetCullMode(volumeRenderMode);
				glBindTexture(GL_TEXTURE_2D, f->glTex->textureId);
				if (filter) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				}
				else {

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				}

				glCallList(f->glList->listId);
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

		for (int i = 0; i < sh.nbFacet; i++) {

			InterfaceFacet* f = facets[i];
			if (!f->cellPropertiesIds.empty() && f->viewSettings.textureVisible) {
				if (!f->glElem) f->BuildMeshGLList();

				glCallList(f->glElem->listId);
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
		for (int i = 0; i < sh.nbFacet; i++) {
			InterfaceFacet* f = facets[i];
			if (f->sh.countDirection && !f->dirCache.empty()) {
				double iw = 1.0 / (double)f->sh.texWidth_precise;
				double ih = 1.0 / (double)f->sh.texHeight_precise;
				double rw = f->sh.U.Norme() * iw;
				for (int x = 0; x < f->sh.texWidth; x++) {
					for (int y = 0; y < f->sh.texHeight; y++) {
						size_t add = x + y * f->sh.texWidth;
						if (f->GetMeshArea(add) > 0.0) {
							double uC = ((double)x + 0.5) * iw;
							double vC = ((double)y + 0.5) * ih;
							float xc = (float)(f->sh.O.x + f->sh.U.x * uC + f->sh.V.x * vC);
							float yc = (float)(f->sh.O.y + f->sh.U.y * uC + f->sh.V.y * vC);
							float zc = (float)(f->sh.O.z + f->sh.U.z * uC + f->sh.V.z * vC);

							RenderArrow(matView,
								(float)f->dirCache[add].dir.x,
								(float)f->dirCache[add].dir.y,
								(float)f->dirCache[add].dir.z,
								xc, yc, zc, (float)rw); // dircache already normalized
						}
					}
				}
			}
		}

		// Restore default matrix
		glLoadMatrixf(matView);
	}

	// Paint non-planar and selected facets
	//if (GetNbSelectedFacets()>0) {
	if (mApp->antiAliasing) {
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
	}
	glBlendFunc(GL_ONE, GL_ZERO);
	if (mApp->highlightNonplanarFacets) {
		glColor3f(1.0f, 0.0f, 1.0f);    //purple
		glCallList(nonPlanarList->listId);
	}

	//Selected facets
	//Don't clip them in cross-section
	if (clippingEnabled) {
		glDisable(GL_CLIP_PLANE0);
	}
	glColor3f(1.0f, 0.0f, 0.0f);    //red
	if (showHiddenFacet) {
		glDisable(GL_DEPTH_TEST);
		glCallList(selectList3->listId);
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glCallList(selectList3->listId);
	}
	if (mApp->antiAliasing) {
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
	}
	if (clippingEnabled) {
		glEnable(GL_CLIP_PLANE0);
	}

	// Paint selected cell on mesh
	for (int i = 0; i < sh.nbFacet; i++) {
		InterfaceFacet* f = facets[i];
		f->RenderSelectedElem();
	}
}

void InterfaceGeometry::RenderSemiTransparent() {
	if (mApp->antiAliasing) {
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
	}

	glCallList(selectHighlightList->listId);

	if (mApp->antiAliasing) {
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
	}
}

void InterfaceGeometry::DeleteGLLists(bool deletePoly, bool deleteLine) {
	if (deleteLine) {
		lineLists.clear();
	}
	if (deletePoly) {
		polyList.reset();
	}
	//selectList.reset();
	//selectList2.reset();
	selectList3.reset();
	selectHighlightList.reset();
}

std::unordered_set<int> InterfaceGeometry::GetVertexBelongsToSelectedFacet() {
	std::unordered_set<int> result;
	auto selFacetIds = GetSelectedFacets();
	for (auto& facetId : selFacetIds) {
		InterfaceFacet* f = facets[facetId];
		for (size_t i = 0; i < f->sh.nbIndex; i++)
			result.insert(f->indices[i]);
	}
	return result;
}

void InterfaceGeometry::BuildDirectionList() {

	// Shapes used for direction field rendering

	// 3D arrow (direction field)

	// Draw a cone
	const float radius = .5f;
	const int cone_slices = 10;
	const int cone_numVerts = (cone_slices + 1) * 2;

	GLfloat vertices[cone_numVerts * 3];
	GLfloat normals[cone_numVerts * 3];

	GLfloat* vertPtr = vertices;
	GLfloat* normPtr = normals;

	GLfloat cap_vertices[(cone_slices + 2) * 3];
	GLfloat* cap_vertPtr = cap_vertices;

	//cap centerpoint
	*cap_vertPtr++ = -.5f;
	*cap_vertPtr++ = 0.0f;
	*cap_vertPtr++ = 0.0f;

	// Calculate the vertices, normals and texture coordinates of the cone
	for (int i = 0; i <= cone_slices; i++) {
		float theta = i * 2 * M_PI / cone_slices;
		float x = radius * cos(theta);
		float z = radius * sin(theta);

		*normPtr++ = radius / sqrt(x * x + z * z);
		*normPtr++ = x;
		*normPtr++ = z;
		*vertPtr++ = *cap_vertPtr++ = -.5f;
		*vertPtr++ = *cap_vertPtr++ = x;
		*vertPtr++ = *cap_vertPtr++ = z;

		*normPtr++ = radius / sqrt(x * x + z * z);
		*normPtr++ = x;
		*normPtr++ = z;
		*vertPtr++ = .5f;
		*vertPtr++ = 0.0f;
		*vertPtr++ = 0.0f;
	}

	arrowList = std::make_unique<GLListWrapper>();
	glNewList(arrowList->listId, GL_COMPILE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	//Draw sides
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glNormalPointer(GL_FLOAT, 0, normals);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, cone_numVerts);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	//Draw cap
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, cap_vertices);

	glDrawArrays(GL_TRIANGLE_FAN, 0, cone_slices + 2);

	glDisableClientState(GL_VERTEX_ARRAY);

	glEndList();

	// Shpere list (isotropic case)
	//const float radius = 1.0f; //radius is 1.0f
	const int sphere_slices = 16;
	const int stacks = 7;
	const int sphere_numVerts = (stacks + 1) * 2 * sphere_slices;

	GLfloat vertices_normals[sphere_numVerts * 3];

	GLfloat* vert_normPtr = vertices_normals;

	// Calculate the vertices, normals and texture coordinates of the sphere
	for (int i = 0; i < sphere_slices; i++) {
		float theta1 = i * 2 * M_PI / sphere_slices;
		float theta2 = (i + 1) * 2 * M_PI / sphere_slices;
		for (int j = 0; j <= stacks; j++) {
			float phi = j * M_PI / stacks;
			float x1 = /*radius * */cos(theta1) * sin(phi);
			float y1 = /*radius * */sin(theta1) * sin(phi);
			float z1 = /*radius * */cos(phi);
			float x2 = /*radius * */cos(theta2) * sin(phi);
			float y2 = /*radius * */sin(theta2) * sin(phi);
			float z2 = /*radius * */cos(phi);

			*vert_normPtr++ = x1;
			*vert_normPtr++ = y1;
			*vert_normPtr++ = z1;
			*vert_normPtr++ = x2;
			*vert_normPtr++ = y2;
			*vert_normPtr++ = z2;
		}
	}

	sphereList = std::make_unique<GLListWrapper>();
	glNewList(sphereList->listId, GL_COMPILE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertices_normals);
	glNormalPointer(GL_FLOAT, 0, vertices_normals);

	glDrawArrays(GL_QUAD_STRIP, 0, sphere_numVerts);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glEndList();

}

void InterfaceGeometry::BuildSelectList() {
	/*
	selectList = std::make_unique<GLListWrapper>();
	glNewList(selectList->listId, GL_COMPILE);
	glEndList();

	// Second list for usage with POLYGON_OFFSET
	selectList2 = std::make_unique<GLListWrapper>();
	glNewList(selectList2->listId, GL_COMPILE);
	glEndList();
	*/

	// Third list with hidden (hole join) edge visible
	selectList3 = std::make_unique<GLListWrapper>();
	glNewList(selectList3->listId, GL_COMPILE);

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

	const auto selectedFacets = GetSelectedFacets();
	const auto colorHighlighting = mApp->worker.GetGeometry()->GetPlottedFacets(); // For colors

	typedef std::pair<size_t, size_t> Edge;
	typedef std::set<Edge> EdgeSet;

	EdgeSet edgeSet;

	//construct edge map
#pragma omp parallel
	{
		EdgeSet edgeSet_local;
#pragma omp for
		for (int i = 0; i < selectedFacets.size(); ++i) {
			const auto& f = facets[selectedFacets[i]];

			//If color highlighting is on, skip highlighted facets as they will be drawn last
			if (!colorHighlighting.empty() && (mApp->profilePlotter != nullptr && ((GLWindow*)(mApp->profilePlotter))->IsVisible())) {
				auto it = colorHighlighting.find(i);
				// Check if element exists in map or not
				if (it != colorHighlighting.end()) {
					continue; //Selected and plotted, will be drawn later (so drawn over everything else), process next selected facet
				}
			}
			//Color highlighting off or highlighting on but this facet isn't plotted (will change its color later to light blue)
			for (size_t j = 0; j < f->indices.size(); ++j) {
				size_t v1 = f->indices[j];
				size_t v2 = f->indices[(j + 1) % f->indices.size()];

				// Ensure canonical order for edge vertices
				if (v1 > v2) {
					std::swap(v1, v2);
				}
				Edge newEdge = std::make_pair(v1, v2);
				edgeSet_local.insert(newEdge);
			}
		}
#pragma omp critical
			edgeSet.insert(edgeSet_local.begin(), edgeSet_local.end());
	}

	std::vector<GLuint> lines;
	lines.reserve(edgeSet.size() * 2);
	for (const auto& it : edgeSet) {
		lines.push_back((GLuint)it.first);
		lines.push_back((GLuint)it.second);
	}

	if (!colorHighlighting.empty() && (mApp->profilePlotter != nullptr && ((GLWindow*)(mApp->profilePlotter))->IsVisible())) {
		
		//Thin light blue for selected but not plotted facets
		//glLineWidth(1.5f);
		GLCOLOR metroLightBlue;
		metroLightBlue.r = 0.937f;
		metroLightBlue.g = 0.957f;
		metroLightBlue.b = 1.0f;
		metroLightBlue.a = 0.08f; //unused in this context
		glColor4f(1.0f, 0.0f, 0.0f,0.5f);
	}
	else {
		glColor3f(1.0f, 0.0f, 0.0f); //regular red selection
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, vertices_raw_opengl.data());
	glDrawElements(GL_LINES, lines.size(), GL_UNSIGNED_INT, lines.data());
	glDisableClientState(GL_VERTEX_ARRAY);

	// give profiled selection priority for being rendered last
	if (!colorHighlighting.empty() && (mApp->profilePlotter != nullptr && ((GLWindow*)(mApp->profilePlotter))->IsVisible())) {
		for (auto& sel : selectedFacets) {
			auto it = colorHighlighting.find(sel);
			// Check if element exists in map or not
			if (it != colorHighlighting.end()) {
				glLineWidth(3.0f);
				float r = static_cast<float>(it->second.r) / 255.0f;
				float g = static_cast<float>(it->second.g) / 255.0f;
				float b = static_cast<float>(it->second.b) / 255.0f;

				// incrase brightness and saturity
				modifyRGBColor(r, g, b, 1.2f, 1.2f);

				glColor3f(r, g, b);
				InterfaceFacet* f = facets[sel];
				DrawFacetWireframe(f, false, false); //Faster than true true true, without noticeable glitches
				glLineWidth(2.0f);
			}
		}
	}
	glLineWidth(1.0f);
	if (mApp->antiAliasing) {
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}
	glEndList();

	if (mApp->highlightSelection) { //Above 500 selected facets rendering can be slow
		// Fourth list with transparent highlighting for selected facets
		selectHighlightList = std::make_unique<GLListWrapper>();
		glNewList(selectHighlightList->listId, GL_COMPILE);
		if (GetNbSelectedFacets() < 500) {  //Above 500 selected facets rendering can be slow
			glDepthMask(GL_FALSE);
			/*glDisable(GL_CULL_FACE);
			glDepthFunc(GL_LEQUAL);*/
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			//glDisable(GL_BLEND);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			glEnable(GL_BLEND);
			//glBlendEquation(GL_MAX);
			//glEnable(GL_MULTISAMPLE);
			//glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

			DrawSemiTransparentPolys(selectedFacets);

			//glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			//glDisable(GL_MULTISAMPLE);
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
		glEndList();
	}
}

void InterfaceGeometry::BuildVolumeFacetList() {
	polyList = std::make_unique<GLListWrapper>();
	glNewList(polyList->listId, GL_COMPILE);
	DrawPolys();
	glEndList();
}

void InterfaceGeometry::BuildNonPlanarList() {


	nonPlanarList = std::make_unique<GLListWrapper>();
	glNewList(nonPlanarList->listId, GL_COMPILE);

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

	auto nonPlanarFacetIds = GetNonPlanarFacetIds();
	hasNonPlanar = !nonPlanarFacetIds.empty();
	for (const auto& np : nonPlanarFacetIds) {
		InterfaceFacet* f = facets[np];
		//DrawFacetWireframe(f,false,true,true);
		DrawFacetWireframe(f, false, false); //Faster than true true true, without noticeable glitches
	}
	glLineWidth(1.0f);
	if (mApp->antiAliasing) {
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}
	glEndList();

}


void InterfaceGeometry::UpdateSelection() {
	DeleteGLLists();
	BuildSelectList();
}

void InterfaceGeometry::BuildGLList() {
	lineLists.clear();
	lineLists.resize(sh.nbSuper);

	// Compile geometry for OpenGL
	for (int s = 0; s < sh.nbSuper; s++) {

		typedef std::pair<size_t, size_t> Edge;
		typedef std::set<Edge> EdgeSet;

		EdgeSet edgeSet;

		//construct edge map
#pragma omp parallel
		{
			EdgeSet edgeSet_local;
#pragma omp for
			for (int i = 0; i < facets.size(); ++i) {
				const auto& f = facets[i];
				if (f->sh.superIdx == s || f->sh.superIdx == -1) {
					for (size_t j = 0; j < f->indices.size(); ++j) {
						size_t v1 = f->indices[j];
						size_t v2 = f->indices[(j + 1) % f->indices.size()];

						// Ensure canonical order for edge vertices
						if (v1 > v2) {
							std::swap(v1, v2);
						}
						Edge newEdge = std::make_pair(v1, v2);
						edgeSet_local.insert(newEdge);
					}
				}
			}
#pragma omp critical
			edgeSet.insert(edgeSet_local.begin(), edgeSet_local.end());
		}

		std::vector<GLuint> lines;
		lines.reserve(edgeSet.size() * 2);
		for (const auto& it : edgeSet) {
			lines.push_back((GLuint)it.first);
			lines.push_back((GLuint)it.second);
		}

		glNewList(lineLists[s].listId, GL_COMPILE);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_DOUBLE, 0, vertices_raw_opengl.data());
		glDrawElements(GL_LINES, lines.size(), GL_UNSIGNED_INT, lines.data());
		glDisableClientState(GL_VERTEX_ARRAY);

		glEndList();
	}

	BuildVolumeFacetList();
	BuildNonPlanarList();
	BuildSelectList();

}

int InterfaceGeometry::InvalidateDeviceObjects() {

	DeleteGLLists(true, true);
	arrowList.reset();
	sphereList.reset();
	for (int i = 0; i < sh.nbFacet; i++)
		facets[i]->InvalidateDeviceObjects();

	return GL_OK;

}

int InterfaceGeometry::RestoreDeviceObjects() {

	if (!IsLoaded()) return GL_OK;

	for (int i = 0; i < sh.nbFacet; i++) {
		InterfaceFacet* f = facets[i];
		f->RestoreDeviceObjects();
		BuildFacetList(f);
	}

	BuildGLList();

	return GL_OK;

}

void InterfaceGeometry::BuildFacetList(InterfaceFacet* f) {

	// Rebuild OpenGL geometry with texture

	if (f->sh.isTextured) {

		std::vector<double> vertexCoords, normalCoords;
		std::vector<float> textureCoords, colorValues;
		GLCOLOR currentColor; //unused
		currentColor.r = 0.0f;
		currentColor.g = 0.0f;
		currentColor.b = 0.0f;
		currentColor.a = 0.0f;

		// Facet geometry
		glNewList(f->glList->listId, GL_COMPILE);
		if (f->sh.nbIndex == 3) {
			FillFacet(f, vertexCoords, normalCoords, textureCoords, colorValues, currentColor, true);
		}
		else {
			TriangulateForRender(f, vertexCoords, normalCoords, textureCoords, colorValues, currentColor, true);
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_DOUBLE, 0, vertexCoords.data());
		glNormalPointer(GL_DOUBLE, 0, normalCoords.data());
		glTexCoordPointer(2, GL_FLOAT, 0, textureCoords.data());

		glDrawArrays(GL_TRIANGLES, 0, vertexCoords.size() / 3);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		glEndList();
	}
}

std::vector<std::optional<ScreenCoord>> InterfaceGeometry::TransformVerticesToScreenCoords(const bool printDebugInfo) {
	// Check intersection of the facet and a "perspective ray"
	// Returns nullopt for vertices behind camera
	// Does not do viewport clipping (off-screen) test

	std::vector<std::optional<ScreenCoord>> screenCoords(sh.nbVertex);

	GLMatrix view, proj, mvp;
	GLVIEWPORT viewPort;
	std::tie(view, proj, mvp, viewPort) = GLToolkit::GetCurrentMatrices();

	Chronometer timer;
	timer.Start();

#pragma omp parallel for
	for (int i = 0; i < sh.nbVertex; i++) {
		screenCoords[i] = GLToolkit::Get2DScreenCoord_fast(vertices3[i], mvp, viewPort);
	}

	timer.Stop();

	if (printDebugInfo) {
		fmt::print("\nTransformed {} vertices in {:.1f} ms.\n", sh.nbVertex, timer.ElapsedMs());
		fmt::print("Not on screens: {{");
		for (int i = 0; i < screenCoords.size(); i++)
			if (!screenCoords[i].has_value()) fmt::print("{},", i + 1);
		fmt::print("}}\n");
	}

	return screenCoords;
}