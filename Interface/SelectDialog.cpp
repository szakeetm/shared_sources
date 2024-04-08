#include "SelectDialog.h"
#include "Facet_shared.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLList.h"
#include "GLApp/GLMessageBox.h"

#include "GLApp/GLTextField.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"

#include "Geometry_shared.h"
#include "Helper/MathTools.h" //Splitstring
#include <numeric> //iota
#include <Helper/StringHelper.h>

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

// Construct a message dialog box
SelectDialog::SelectDialog(InterfaceGeometry *g):GLWindow() {

  int /*xD,yD,*/wD,hD;
  wD=400;hD=105;
  SetTitle("Select facet(s) by number");

  // Label
  GLLabel  *label = new GLLabel("Facet number:");
  label->SetBounds(10,4,100,18);
  Add(label);

  numText = new GLTextField(0,NULL);
  numText->SetBounds(90,3,280,18);
  Add(numText);

  GLLabel* label2 = new GLLabel("You can enter a list and/or range(s), examples: 1,2,3 or 1-10 or 1-10,20-30");
  label2->SetBounds(10, 28, 100, 18);
  Add(label2);

  // Buttons
  int startX = 5;
  int startY = 55;
     
    selButton = new GLButton(0,"Select");
	selButton->SetBounds(startX,startY,125,20);
    Add(selButton);
    startX+=130;
  
    addButton = new GLButton(0,"Add to selection");
    addButton->SetBounds(startX,startY,125,20);
    Add(addButton);
    startX+=130;
  
    remButton = new GLButton(0,"Remove from selection");
    remButton->SetBounds(startX,startY,125,20);
    Add(remButton);
    startX+=130;  

  // Center dialog
  /*int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  if( wD > wS ) wD = wS;
  xD = (wS-wD)/2;
  yD = (hS-hD)/2;*/
  SetBounds(10,30,wD,hD);

  // Create objects
  RestoreDeviceObjects();

  rCode = GLDLG_CANCEL;

  interfGeom = g;
}

void SelectDialog::ProcessMessage(GLComponent *src,int message) {
  if(message==MSG_BUTTON) {
      std::vector<size_t> facetIds;
      try{
          splitFacetList(facetIds, numText->GetText(), interfGeom->GetNbFacet());
      }
      catch (const std::exception &e) {
          GLMessageBox::Display(e.what(), "Error", GLDLG_OK, GLDLG_ICONERROR);
          return;
      }

      if (src == selButton)
          interfGeom->UnselectAll();

      for (const auto facetId : facetIds) {
          interfGeom->GetFacet(facetId)->selected = Contains({ selButton,addButton },src);
      }
      interfGeom->UpdateSelection();
      mApp->UpdateFacetParams(true);
      mApp->UpdateFacetlistSelected();
      mApp->facetList->ScrollToVisible(facetIds.back(),1,true);
  }
  GLWindow::ProcessMessage(src,message);
}