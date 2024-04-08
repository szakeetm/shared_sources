
#include "AlignFacet.h"
#include "Facet_shared.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLButton.h"
#include "Helper/MathTools.h" //Contains

#include "Geometry_shared.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

#if defined(MOLFLOW)
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
extern SynRad*mApp;
#endif

/**
* \brief Constructor with initialisation for the AlignFacet window (Facet/Align to)
* \param g pointer to the InterfaceGeometry
* \param w Worker handle
*/
AlignFacet::AlignFacet(InterfaceGeometry *g,Worker *w):GLWindow() {

	int wD = 290;
	int hD = 355;

	SetTitle("Align selected facets to an other");

	step1 = new GLTitledPanel("Step 1: select facets of the object");
	step1->SetBounds(5,5,wD-10,75);
	Add(step1);
	
	numFacetSel = new GLLabel("0 facets will be aligned");
	numFacetSel->SetBounds(75,25,120,21);
	step1->Add(numFacetSel);
	
	memoSel = new GLButton(0,"Update from selection");
	memoSel->SetBounds(70,50,130,21);
	step1->Add(memoSel);

	step2 = new GLTitledPanel("Step 2: select snapping facets & points");
	step2->SetBounds(5,85,wD-10,115);
	Add(step2);

	l1 = new GLLabel("1. Choose two facets that will be snapped together.\n\n2. Choose 2-2 vertices on the source and destination \nfacets: One will serve as an anchor point, one as a\ndirection aligner. Once you have 2 facets and 4\nvertices selected, proceed to step 3.");
	l1->SetBounds(10,100,120,21);
	step2->Add(l1);

	step3 = new GLTitledPanel("Step 3: align");
	step3->SetBounds(5,205,wD-10,120);
	Add(step3);

	invertNormal = new GLToggle(0,"Invert normal");
	invertNormal->SetBounds(10,220,150,21);
	invertNormal->SetState(true);
	step3->Add(invertNormal);

	invertDir1 = new GLToggle(0,"Swap anchor/direction vertices on source");
	invertDir1->SetBounds(10,245,150,21);
	step3->Add(invertDir1);

	invertDir2 = new GLToggle(0,"Swap ancor/direction vertices on destination");
	invertDir2->SetBounds(10,270,150,21);
	step3->Add(invertDir2);

	alignButton = new GLButton(0,"Align");
	alignButton->SetBounds(10,295,63,21);
	step3->Add(alignButton);

	copyButton = new GLButton(0,"Copy");
	copyButton->SetBounds(78,295,63,21);
	//step3->Add(copyButton);

	undoButton = new GLButton(0,"Undo");
	undoButton->SetBounds(146,295,63,21);
	step3->Add(undoButton);

	cancelButton = new GLButton(0,"Dismiss");
	cancelButton->SetBounds(214,295,63,21);
	step3->Add(cancelButton);

	// Center dialog
	int wS,hS;
	GLToolkit::GetScreenSize(&wS,&hS);
	int xD = (wS-wD)/2;
	int yD = (hS-hD)/2;
	SetBounds(xD,yD,wD,hD);

	RestoreDeviceObjects();

	interfGeom = g;
	work = w;

}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void AlignFacet::ProcessMessage(GLComponent *src,int message) {
	switch(message) {
	case MSG_BUTTON:

		if(src==cancelButton) {
			GLWindow::ProcessMessage(NULL,MSG_CLOSE);

		} else if (src==memoSel) {
			MemorizeSelection();
		} else if (src==alignButton || src==copyButton) {
			if (memorizedSelection.size()==0) {
				GLMessageBox::Display("No facets memorized","Nothing to align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			auto appSelectedFacets = interfGeom->GetSelectedFacets();
			if (appSelectedFacets.size()!=2) {
				GLMessageBox::Display("Two facets (source and destination) must be selected","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			bool foundSource = false;
			size_t sourceFacetId;
			for (auto& mem:memorizedSelection) { //find source facet
				if ( Contains(appSelectedFacets,mem)) {
					if (!foundSource) {
						foundSource = true;
						sourceFacetId=mem;
					} else {
						GLMessageBox::Display("Both selected facets are on the source object. One must be on the destination.","Can't align",GLDLG_OK,GLDLG_ICONERROR);
						return;
					}
				}
			}
			
			if (!foundSource) {
				GLMessageBox::Display("No facet selected on source object.","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			bool foundDest = false;
			size_t destFacetId;
			for (int i=0;i<appSelectedFacets.size()&&(!foundDest);i++) { //find destination facet
				if (appSelectedFacets[i] != sourceFacetId) {
					destFacetId = appSelectedFacets[i];
					foundDest = true;
				}
			}
			if (!foundDest) {
				GLMessageBox::Display("Can't find destination facet","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			if (interfGeom->GetNbSelectedVertex()!=4) {
				GLMessageBox::Display("4 vertices must be selected: two on source and two on destination facets","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			int anchorSourceVertexId,anchorDestVertexId,dirSourceVertexId,dirDestVertexId;
			anchorSourceVertexId=anchorDestVertexId=dirSourceVertexId=dirDestVertexId=-1;

			//find source anchor and dir vertex
			for (int j=0;j<interfGeom->GetFacet(sourceFacetId)->sh.nbIndex;j++) {
				if (interfGeom->GetVertex(interfGeom->GetFacet(sourceFacetId)->indices[j])->selected) {
					if (anchorSourceVertexId==-1 && dirSourceVertexId==-1) {
						anchorSourceVertexId=(int)interfGeom->GetFacet(sourceFacetId)->indices[j];
					} else if (dirSourceVertexId==-1) {
						dirSourceVertexId=(int)interfGeom->GetFacet(sourceFacetId)->indices[j];
					} else {
						GLMessageBox::Display("More than two selected vertices are on the source facet. Two must be on the destination.","Can't align",GLDLG_OK,GLDLG_ICONERROR);
						return;
					}
				}
			}

			if (anchorSourceVertexId==-1 || dirSourceVertexId==-1) {
				GLMessageBox::Display("Less than two selected vertices found on source facet. Select two (anchor, direction).","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			//find destination anchor and dir vertex
			for (int j=0;j<interfGeom->GetFacet(destFacetId)->sh.nbIndex;j++) {
				if (interfGeom->GetVertex(interfGeom->GetFacet(destFacetId)->indices[j])->selected) {
					if (anchorDestVertexId==-1 && dirDestVertexId==-1) {
						anchorDestVertexId=(int)interfGeom->GetFacet(destFacetId)->indices[j];
					} else if (dirDestVertexId==-1) {
						dirDestVertexId=(int)interfGeom->GetFacet(destFacetId)->indices[j];
					} else {
						GLMessageBox::Display("More than two selected vertices are on the destination facet. Two must be on the source.","Can't align",GLDLG_OK,GLDLG_ICONERROR);
						return;
					}
				}

			}
			if (anchorDestVertexId==-1 || dirDestVertexId==-1) {
				GLMessageBox::Display("Less than two selected vertices found on destination facet. Select two (anchor, direction).","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			if (mApp->AskToReset()){
				interfGeom->AlignFacets(memorizedSelection,sourceFacetId, destFacetId,anchorSourceVertexId,anchorDestVertexId,dirSourceVertexId,
dirDestVertexId,
					invertNormal->GetState(),invertDir1->GetState(),invertDir2->GetState(),src==copyButton,work);
				#if defined(MOLFLOW)
				if (src == copyButton) mApp->worker.GetGeometry();
				//mApp->worker.CalcTotalOutgassing();
				#endif
				//mApp->UpdateModelParams();
				work->MarkToReload();

				mApp->changedSinceSave = true;
				mApp->UpdateFacetlistSelected();	
				mApp->UpdateViewers();
				//GLWindowManager::FullRepaint();
			}
		} else if (src==undoButton) {
			if (!mApp->AskToReset(work)) return;
			for (size_t i=0;i<memorizedSelection.size();i++) {
				InterfaceFacet *f=interfGeom->GetFacet(memorizedSelection[i]);
				for (size_t j=0;j<f->sh.nbIndex;j++) {
					interfGeom->GetVertex(f->indices[j])->SetLocation(this->oriPositions[i][j]);
				}
			}
			interfGeom->InitializeGeometry();
			work->MarkToReload();			 
			mApp->UpdateFacetlistSelected();	
			mApp->UpdateViewers();
		}
		break;
	}

	GLWindow::ProcessMessage(src,message);
}

/**
* \brief Memorises current facet selection for the align process
*/
void AlignFacet::MemorizeSelection() {
	memorizedSelection = interfGeom->GetSelectedFacets();
	oriPositions.clear();
	for (auto& sel : memorizedSelection) {
		std::vector<Vector3d> op;
		for (size_t ind = 0; ind < interfGeom->GetFacet(sel)->sh.nbIndex; ind++)
			op.push_back(*interfGeom->GetVertex(interfGeom->GetFacet(sel)->indices[ind]));
		oriPositions.push_back(op);
	}
	std::stringstream msg;
	msg << memorizedSelection.size() << " facets will be aligned.";
	numFacetSel->SetText(msg.str());
}