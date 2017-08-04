#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif
#include "Facet.h"
#include "Polygon.h"
//#include <malloc.h>

#include <string.h>
#include <math.h>
#include "GLApp/GLToolkit.h"
#include "GLApp\MathTools.h"
#include <sstream>

using namespace pugi;

// Colormap stuff
extern COLORREF rainbowCol[]; //defined in GLGradient.cpp

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

void Facet::DetectOrientation() {

	// Detect polygon orientation (clockwise or counter clockwise)
	// p= 1.0 => The second vertex is convex and vertex are counter clockwise.
	// p=-1.0 => The second vertex is concave and vertex are clockwise.
	// p= 0.0 => The polygon is not a simple one and orientation cannot be detected.

	POLYGON p;
	p.nbPts = sh.nbIndex;
	p.pts = vertices2;
	p.sign = 1.0;

	bool convexFound = false;
	size_t i = 0;
	while (i < p.nbPts && !convexFound) {
		Vector2d c;
		bool empty = EmptyTriangle(&p, (int)i - 1, (int)i, (int)i + 1, &c);
		if (empty || sh.nbIndex == 3) {
			size_t _i1 = IDX(i - 1, p.nbPts);
			size_t _i2 = IDX(i, p.nbPts);
			size_t _i3 = IDX(i + 1, p.nbPts);
			if (IsInPoly(c.u, c.v, p.pts, p.nbPts)) {
				convexFound = true;
				// Orientation
				if (IsConvex(&p, i)) p.sign = 1.0;
				else                 p.sign = -1.0;
			}
		}
		i++;
	}

	if (!convexFound) {
		// Not a simple polygon
		sh.sign = 0.0;
	}
	else {

		sh.sign = p.sign;
	}

}

int Facet::RestoreDeviceObjects() {

	// Initialize scene objects (OpenGL)
	if (sh.isTextured) {
		glGenTextures(1, &glTex);
		glList = glGenLists(1);
	}

	//BuildMeshList();
	BuildSelElemList();

	return GL_OK;

}

int Facet::InvalidateDeviceObjects() {

	// Free all alocated resource (OpenGL)
	DELETE_TEX(glTex);
	DELETE_LIST(glList);
	DELETE_LIST(glElem);
	DELETE_LIST(glSelElem);
	return GL_OK;

}

bool Facet::SetTexture(double width, double height, bool useMesh) {

	bool dimOK = (width*height > 0.0000001);

	if (dimOK) {
		sh.texWidthD = width;
		sh.texHeightD = height;
		sh.texWidth = (int)ceil(width *0.9999999); //0.9999999: cut the last few digits (convert rounding error 1.00000001 to 1, not 2)
		sh.texHeight = (int)ceil(height *0.9999999);
		dimOK = (sh.texWidth > 0 && sh.texHeight > 0);
	}
	else {
		sh.texWidth = 0;
		sh.texHeight = 0;
		sh.texWidthD = 0.0;
		sh.texHeightD = 0.0;
	}

	texDimW = 0;
	texDimH = 0;
	hasMesh = false;
	//SAFE_FREE(mesh);
	for (size_t i = 0; i < meshvectorsize; i++)
		SAFE_FREE(meshvector[i].points);
	SAFE_FREE(meshvector);
	meshvectorsize = 0;
	SAFE_FREE(dirCache);
	DELETE_TEX(glTex);
	DELETE_LIST(glList);
	DELETE_LIST(glElem);
	/*if (meshPts) {
		for (size_t i = 0; i < nbElem; i++)
			SAFE_FREE(meshPts[i].pts);
	}*/

	//SAFE_FREE(meshPts);
	SAFE_FREE(cellPropertiesIds);
	//nbElem = 0;
	UnselectElem();

	if (dimOK) {

		// Add a 1 texel border for bilinear filtering (rendering purpose)
		texDimW = GetPower2(sh.texWidth + 2);
		texDimH = GetPower2(sh.texHeight + 2);
		if (texDimW < 4) texDimW = 4;
		if (texDimH < 4) texDimH = 4;
		glGenTextures(1, &glTex);
		glList = glGenLists(1);
		if (useMesh)
			if (!BuildMesh()) return false;
		if (sh.countDirection) {
			dirCache = (VHIT *)calloc(sh.texWidth*sh.texHeight, sizeof(VHIT));
			if (!dirCache) return false;
			//memset(dirCache,0,dirSize); //already done by calloc
		}

	}

	UpdateFlags(); //set hasMesh to true if everything was OK
	return true;

}

void Facet::glVertex2u(double u, double v) {

	glVertex3d(sh.O.x + sh.U.x*u + sh.V.x*v,
		sh.O.y + sh.U.y*u + sh.V.y*v,
		sh.O.z + sh.U.z*u + sh.V.z*v);

}

bool Facet::BuildMesh() {

	/*mesh = (SHELEM *)malloc(sh.texWidth * sh.texHeight * sizeof(SHELEM));
	if (!mesh) {
		//Couldn't allocate memory
		return false;
		//throw Error("malloc failed on Facet::BuildMesh()");
	}
	meshPts = (MESH *)malloc(sh.texWidth * sh.texHeight * sizeof(MESH));
	if (!meshPts) {
		return false;
	}*/
	cellPropertiesIds = (int *)malloc(sh.texWidth * sh.texHeight * sizeof(int));
	if (!cellPropertiesIds) {
		//Couldn't allocate memory
		return false;
		//throw Error("malloc failed on Facet::BuildMesh()");
	}
	meshvector = (CellProperties *)malloc(sh.texWidth * sh.texHeight * sizeof(CellProperties)); //will shrink at the end
	if (!meshvector) {
		//Couldn't allocate memory
		return false;
		//throw Error("malloc failed on Facet::BuildMesh()");

	}
	meshvectorsize = 0;
	hasMesh = true;
	//memset(mesh, 0, sh.texWidth * sh.texHeight * sizeof(SHELEM));
	//memset(meshPts, 0, sh.texWidth * sh.texHeight * sizeof(MESH));
	memset(cellPropertiesIds, 0, sh.texWidth * sh.texHeight * sizeof(int));
	memset(meshvector, 0, sh.texWidth * sh.texHeight * sizeof(CellProperties));

	POLYGON P1, P2;
	double sx, sy, A/*,tA*/;
	double iw = 1.0 / (double)sh.texWidthD;
	double ih = 1.0 / (double)sh.texHeightD;
	double rw = sh.U.Norme() * iw;
	double rh = sh.V.Norme() * ih;
	double *vList;
	double fA = iw*ih;
	size_t    nbv;

	P1.pts = (Vector2d *)malloc(4 * sizeof(Vector2d));

	if (!P1.pts) {
		throw Error("Couldn't allocate memory for texture mesh points.");
	}
	P1.nbPts = 4;
	P1.sign = 1.0;
	P2.nbPts = sh.nbIndex;
	P2.pts = vertices2;
	P2.sign = -sh.sign;
	//tA = 0.0;
	//nbElem = 0;

	for (size_t j = 0;j < sh.texHeight;j++) {
		sy = (double)j;
		for (size_t i = 0;i < sh.texWidth;i++) {
			sx = (double)i;

			bool allInside = false;
			double u0 = sx * iw;
			double v0 = sy * ih;
			double u1 = (sx + 1.0) * iw;
			double v1 = (sy + 1.0) * ih;
			float  uC, vC;
			//mesh[i + j*sh.texWidth].elemId = -1;

			if (sh.nbIndex <= 4) {

				// Optimization for quad and triangle
				allInside = IsInPoly(u0, v0, vertices2, sh.nbIndex);
				allInside = allInside && IsInPoly(u0, v1, vertices2, sh.nbIndex);
				allInside = allInside && IsInPoly(u1, v0, vertices2, sh.nbIndex);
				allInside = allInside && IsInPoly(u1, v1, vertices2, sh.nbIndex);

			}

			if (!allInside) {
				CellProperties cellprop;

				// Intersect element with the facet (facet boundaries)
				P1.pts[0].u = u0;
				P1.pts[0].v = v0;
				P1.pts[1].u = u1;
				P1.pts[1].v = v0;
				P1.pts[2].u = u1;
				P1.pts[2].v = v1;
				P1.pts[3].u = u0;
				P1.pts[3].v = v1;
				A = GetInterArea(&P1, &P2, visible, &uC, &vC, &nbv, &vList);
				if (!IsZero(A)) {

					if (A > (fA + 1e-10)) {

						// Polyon intersection error !
						// Switch back to brute force
						A = GetInterAreaBF(&P2, u0, v0, u1, v1, &uC, &vC);
						bool fullElem = IsZero(fA - A);
						if (!fullElem) {
							cellprop.area = (float)(A*(rw*rh) / (iw*ih));
							cellprop.uCenter = uC;
							cellprop.vCenter = vC;
							cellprop.nbPoints = 0;
							cellprop.points = NULL;
							cellPropertiesIds[i + j*sh.texWidth] = (int)meshvectorsize;
							meshvector[meshvectorsize++] = cellprop;
						}
						else {
							cellPropertiesIds[i + j*sh.texWidth] = -1;
						}

						//cellprop.full = IsZero(fA - A);

					}
					else {

						bool fullElem = IsZero(fA - A);
						if (!fullElem) {
							// !! P1 and P2 are in u,v coordinates !!
							cellprop.area = (float)(A*(rw*rh) / (iw*ih));
							cellprop.uCenter = uC;
							cellprop.vCenter = vC;
							//cellprop.full = IsZero(fA - A);
							//cellprop.elemId = nbElem;

							// Mesh coordinates
							cellprop.points = (Vector2d*)malloc(nbv * sizeof(Vector2d));
							cellprop.nbPoints = nbv;
							for (size_t n = 0; n < nbv; n++) {
								Vector2d newPoint;
								newPoint.u = vList[2 * n];
								newPoint.v = vList[2 * n + 1];
								cellprop.points[n] = (newPoint);
							}
							cellPropertiesIds[i + j*sh.texWidth] = (int)meshvectorsize;
							meshvector[meshvectorsize++] = cellprop;
							//nbElem++;

						}
						else {
							cellPropertiesIds[i + j*sh.texWidth] = -1;
						}

					}

				}
				else cellPropertiesIds[i + j*sh.texWidth] = -2; //zero element
				SAFE_FREE(vList);

			}
			else {  //All indide and triangle or quad
				cellPropertiesIds[i + j*sh.texWidth] = -1;

				/*mesh[i + j*sh.texWidth].area = (float)(rw*rh);
				mesh[i + j*sh.texWidth].uCenter = (float)(u0 + u1) / 2.0f;
				mesh[i + j*sh.texWidth].vCenter = (float)(v0 + v1) / 2.0f;
				mesh[i + j*sh.texWidth].full = true;
				mesh[i + j*sh.texWidth].elemId = nbElem;

				// Mesh coordinates
				meshPts[nbElem].nbPts = 4;
				meshPts[nbElem].pts = (Vector2d *)malloc(4 * sizeof(Vector2d));

				if (!meshPts[nbElem].pts) {
					throw Error("Couldn't allocate memory for texture mesh points.");
				}
				meshPts[nbElem].pts[0].u = u0;
				meshPts[nbElem].pts[0].v = v0;
				meshPts[nbElem].pts[1].u = u1;
				meshPts[nbElem].pts[1].v = v0;
				meshPts[nbElem].pts[2].u = u1;
				meshPts[nbElem].pts[2].v = v1;
				meshPts[nbElem].pts[3].u = u0;
				meshPts[nbElem].pts[3].v = v1;
				nbElem++;*/

			}

			//tA += mesh[i + j*sh.texWidth].area;

		}
	}
	//Shrink mesh vector
	meshvector = (CellProperties*)realloc(meshvector, sizeof(CellProperties)*meshvectorsize);

	// Check meshing accuracy (TODO)
	/*
	int p = (int)(ceil(log10(sh.area)));
	double delta = pow(10.0,(double)(p-5));
	if( fabs(sh.area - tA)>delta ) {
	}
	*/

	free(P1.pts);
	if (mApp->needsMesh) BuildMeshList();
	return true;

}

void Facet::BuildMeshList() {

	if (!cellPropertiesIds)

		return;

	DELETE_LIST(glElem);

	// Build OpenGL geometry for meshing
	glElem = glGenLists(1);
	glNewList(glElem, GL_COMPILE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	size_t nb = sh.texWidth*sh.texHeight;
	for (size_t i = 0; i < nb; i++) {
		if (cellPropertiesIds[i] != -2) {
			glBegin(GL_POLYGON);
			size_t nbPts = GetMeshNbPoint(i);
			for (size_t n = 0; n < nbPts; n++) {
				glEdgeFlag(true);
				Vector2d pt = GetMeshPoint(i, n);
				glVertex2u(pt.u, pt.v);
			}
			glEnd();
		}

	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEndList();

}

void Facet::BuildSelElemList() {

	DELETE_LIST(glSelElem);
	int nbSel = 0;

	if (cellPropertiesIds && selectedElem.width != 0 && selectedElem.height != 0) {

		glSelElem = glGenLists(1);
		glNewList(glSelElem, GL_COMPILE);
		glColor3f(1.0f, 1.0f, 1.0f);
		glLineWidth(1.0f);
		glEnable(GL_LINE_SMOOTH);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-1.0f, -1.0f);
		for (size_t i = 0; i < selectedElem.width; i++) {
			for (size_t j = 0; j < selectedElem.height; j++) {
				size_t add = (selectedElem.u + i) + (selectedElem.v + j)*sh.texWidth;
				//int elId = mesh[add].elemId;

				//if (cellPropertiesIds[add]!=-1 && meshvector[cellPropertiesIds[add]].elemId>=0) {
				if (cellPropertiesIds[add] != -2) {

					glBegin(GL_POLYGON);
					/*for (int n = 0; n < meshPts[elId].nbPts; n++) {
						glEdgeFlag(true);
						glVertex2u(meshPts[elId].pts[n].u, meshPts[elId].pts[n].v);
					}*/
					for (size_t p = 0;p < GetMeshNbPoint(add);p++) {
						Vector2d point = GetMeshPoint(add, p);
						glEdgeFlag(true);
						glVertex2u(point.u, point.v);
					}
					glEnd();
					nbSel++;
				}

			}
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glDisable(GL_LINE_SMOOTH);
		glEndList();

		// Empty selection
		if (nbSel == 0) UnselectElem();
	}
}

void Facet::UnselectElem() {

	DELETE_LIST(glSelElem);
	selectedElem.width = 0;
	selectedElem.height = 0;

}

void Facet::SelectElem(size_t u, size_t v, size_t width, size_t height) {

	UnselectElem();

	if (cellPropertiesIds && u >= 0 && u < sh.texWidth && v >= 0 && v < sh.texHeight) {

		size_t maxW = sh.texWidth - u;
		size_t maxH = sh.texHeight - v;
		selectedElem.u = u;
		selectedElem.v = v;
		selectedElem.width = Min(maxW, width);
		selectedElem.height = Min(maxH, height);
		BuildSelElemList();

	}

}

void Facet::RenderSelectedElem() {
	if (glSelElem) glCallList(glSelElem);
}

void Facet::Explode(FACETGROUP *group) {
	size_t nonZeroElems = 0, nb = 0;
	for (size_t i = 0;i < sh.texHeight*sh.texWidth;i++) {
		if (cellPropertiesIds[i] != -2) {
			nonZeroElems++;
		}
	}

	if (!(group->facets = (Facet **)malloc(nonZeroElems * sizeof(Facet *))))
		throw Error("Not enough memory to create new facets");
	for (size_t i = 0;i < sh.texHeight*sh.texWidth;i++) {

		if (cellPropertiesIds[i] != -2) {
			try {
				Facet *f = new Facet(GetMeshNbPoint(i));
				f->CopyFacetProperties(this);
				group->facets[i] = f;

			}
			catch (...) {
				for (size_t d = 0; d < i; d++)
					SAFE_DELETE(group->facets[d]);
				throw Error("Cannot reserve memory for new facet(s)");
			}
			nb += GetMeshNbPoint(i);
		}

	}

	group->nbF = nonZeroElems;
	group->nbV = nb;

}

void Facet::FillVertexArray(InterfaceVertex *v) {

	int nb = 0;
	for (size_t i = 0;i < sh.texHeight*sh.texWidth;i++) {
		if (cellPropertiesIds[i] != -2) {
			for (size_t j = 0; j < GetMeshNbPoint(i); j++) {
				Vector2d p = GetMeshPoint(i, j);
				v[nb].x = sh.O.x + sh.U.x*p.u + sh.V.x*p.v;
				v[nb].y = sh.O.y + sh.U.y*p.u + sh.V.y*p.v;
				v[nb].z = sh.O.z + sh.U.z*p.u + sh.V.z*p.v;
				nb++;
			}
		}
	}

}

size_t Facet::GetTexSwapSize(bool useColormap) {

	size_t tSize = texDimW*texDimH;
	if (useColormap) tSize = tSize * 4;
	return tSize;

}

size_t Facet::GetTexSwapSizeForRatio(double ratio, bool useColor) {

	double nU = sh.U.Norme();
	double nV = sh.V.Norme();
	double width = nU*ratio;
	double height = nV*ratio;

	bool dimOK = (width*height > 0.0000001);

	if (dimOK) {

		int iwidth = (int)ceil(width);
		int iheight = (int)ceil(height);
		int m = Max(iwidth, iheight);
		size_t tDim = GetPower2(m);
		if (tDim < 16) tDim = 16;
		size_t tSize = tDim*tDim;
		if (useColor) tSize *= 4;
		return tSize;

	}
	else {
		return 0;
	}
}

size_t Facet::GetNbCell() {
	return sh.texHeight * sh.texWidth;
}

size_t Facet::GetNbCellForRatio(double ratio) {

	double nU = sh.U.Norme();
	double nV = sh.V.Norme();
	double width = nU*ratio;
	double height = nV*ratio;

	bool dimOK = (width*height > 0.0000001);

	if (dimOK) {
		int iwidth = (int)ceil(width);
		int iheight = (int)ceil(height);
		return iwidth*iheight;
	}
	else {

		return 0;
	}

}
		
void Facet::SwapNormal() {

	// Revert vertex order (around the second point)

	size_t* tmp = (size_t *)malloc(sh.nbIndex * sizeof(size_t));
	for (size_t i = sh.nbIndex, j = 0; i > 0; i--, j++)
		tmp[(i + 1) % sh.nbIndex] = GetIndex((int)j + 1);
	free(indices);
	indices = tmp;

	/* normal recalculated at reinitialize
	// Invert normal
	sh.N.x = -sh.N.x;
	sh.N.y = -sh.N.y;
	sh.N.z = -sh.N.z;*/

}

void Facet::ShiftVertex() {

	// Shift vertex

	size_t *tmp = (size_t *)malloc(sh.nbIndex * sizeof(size_t));
	for (size_t i = 0; i < sh.nbIndex; i++)
		tmp[i] = GetIndex((int)i + 1);
	free(indices);
	indices = tmp;

}

void Facet::InitVisibleEdge() {

	// Detect non visible edge (for polygon which contains holes)
	memset(visible, 0xFF, sh.nbIndex * sizeof(bool));

	for (int i = 0;i < sh.nbIndex;i++) {

		size_t p11 = GetIndex(i);
		size_t p12 = GetIndex(i + 1);

		for (size_t j = i + 1;j < sh.nbIndex;j++) {

			size_t p21 = GetIndex((int)j);
			size_t p22 = GetIndex((int)j + 1);

			if ((p11 == p22 && p12 == p21) || (p11 == p21 && p12 == p22)) {
				// Invisible edge found
				visible[i] = false;
				visible[j] = false;
			}

		}

	}

}

size_t Facet::GetIndex(int idx) {
	if (idx < 0) {
		return indices[(sh.nbIndex + idx) % sh.nbIndex];
	}
	else {
		return indices[idx % sh.nbIndex];
	}
}

size_t Facet::GetIndex(size_t idx) {
		return indices[idx % sh.nbIndex];
}

float Facet::GetMeshArea(size_t index,bool correct2sides) {
	if (!cellPropertiesIds) return -1.0f;
	if (cellPropertiesIds[index] == -1) {
		return (float)(((correct2sides && sh.is2sided) ? 2.0f : 1.0f) / (tRatio*tRatio));
	}
	else if (cellPropertiesIds[index] == -2) {
		return 0.0f;
	}
	else {
		return ((correct2sides && sh.is2sided) ? 2.0f : 1.0f) * meshvector[cellPropertiesIds[index]].area;
	}
}

size_t Facet::GetMeshNbPoint(size_t index)
{
	size_t nbPts;
	if (cellPropertiesIds[index] == -1) nbPts = 4;
	else if (cellPropertiesIds[index] == -2) nbPts = 0;
	else nbPts = meshvector[cellPropertiesIds[index]].nbPoints;
	return nbPts;
}

Vector2d Facet::GetMeshPoint(size_t index, size_t pointId)
{
	Vector2d result;
	if (!cellPropertiesIds) {
		result.u = 0.0;
		result.v = 0.0;
		return result;
	}
	else {
		int id = cellPropertiesIds[index];
		if (id == -2) {
			result.u = 0.0;
			result.v = 0.0;
			return result;
		}
		else if (id != -1) {
			if (pointId < meshvector[id].nbPoints)
				return meshvector[id].points[pointId];
			else {
				result.u = 0.0;
				result.v = 0.0;
				return result;
			}

		}

		else { //full elem
			double iw = 1.0 / (double)sh.texWidthD;
			double ih = 1.0 / (double)sh.texHeightD;
			double sx = (double)(index%sh.texWidth);
			double sy = (double)(index / sh.texWidth);
			if (pointId == 0) {
				double u0 = sx * iw;
				double v0 = sy * ih;
				result.u = u0;
				result.v = v0;
				return result;
			}
			else if (pointId == 1) {
				double u1 = (sx + 1.0) * iw;
				double v0 = sy * ih;
				result.u = u1;
				result.v = v0;
				return result;
			}
			else if (pointId == 2) {
				double u1 = (sx + 1.0) * iw;
				double v1 = (sy + 1.0) * ih;
				result.u = u1;
				result.v = v1;
				return result;
			}
			else if (pointId == 3) {
				double u0 = sx * iw;
				double v1 = (sy + 1.0) * ih;
				result.u = u0;
				result.v = v1;
				return result;
			}
			else {
				result.u = 0.0;
				result.v = 0.0;
				return result;
			}
		}
	}
}

Vector2d Facet::GetMeshCenter(size_t index)
{
	Vector2d result;
	if (!cellPropertiesIds) {
		result.u = 0.0;
		result.v = 0.0;
		return result;
	}
	if (cellPropertiesIds[index] != -1) {
		if (cellPropertiesIds[index] == -2) {
			result.u = 0.0;
			result.v = 0.0;
			return result;
		}
		else {
			result.u = meshvector[cellPropertiesIds[index]].uCenter;
			result.v = meshvector[cellPropertiesIds[index]].vCenter;
			return result;
		}
	}
	else {
		double iw = 1.0 / (double)sh.texWidthD;
		double ih = 1.0 / (double)sh.texHeightD;
		double sx = (double)(index%sh.texWidth);
		double sy = (double)(index / sh.texWidth);
		double u0 = sx * iw;
		double v0 = sy * ih;
		double u1 = (sx + 1.0) * iw;
		double v1 = (sy + 1.0) * ih;
		result.u = (float)(u0 + u1) / 2.0f;
		result.v = (float)(v0 + v1) / 2.0f;
		return result;
	}
}

double Facet::GetArea() {
	return sh.area*(sh.is2sided ? 2.0 : 1.0);
}

bool Facet::IsTXTLinkFacet() {
	return ((sh.opacity == 0.0) && (sh.sticking >= 1.0));
}

Vector3d Facet::GetRealCenter() {
	return Project(sh.center, sh.O, sh.N);
}