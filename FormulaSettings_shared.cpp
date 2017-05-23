#include "FormulaSettings.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp\GLTextField.h"
#include "GLApp\GLButton.h"

#ifdef MOLFLOW
#include "MolFlow.h"
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
#include "SynRad.h"
extern SynRad*mApp;
#endif

void FormulaSettings::Update(GLParser *f,int id) {

	if (f == NULL) { //New formula
	  applyButton->SetText("Create");
	  deleteButton->SetEnabled(false);
	  nameT->SetText("");
	  exprT->SetText("");
	}
	else { //Edit formula
		applyButton->SetText("Apply");
		deleteButton->SetEnabled(true);
		nameT->SetText(f->GetName());
		exprT->SetText(f->GetExpression());
	}
	formulaId = id;
}

void FormulaSettings::DisplayError(GLParser *f) {

  char tmp[512];
  char tmp2[512];
  sprintf(tmp2,f->GetExpression());
  if(strlen(tmp2)) {
    int pos = f->GetCurrentPos();
    tmp2[pos] = 0;
    int ew = GLToolkit::GetDialogFont()->GetTextWidth(tmp2);
    tmp2[0] = ' ';
    tmp2[1] = 0;
    int sw = GLToolkit::GetDialogFont()->GetTextWidth(tmp2);
    int nbSpace = ew / sw;
    memset(tmp2,' ',512);
    tmp2[nbSpace]=0;
    sprintf(tmp,"%s\n%s^\n%s",f->GetExpression(),tmp2,f->GetErrorMsg());
	GLMessageBox::Display(tmp, "Formula error", GLDLG_OK, GLDLG_ICONINFO);
  } else {
    GLMessageBox::Display(f->GetErrorMsg(),"Formula error",GLDLG_OK,GLDLG_ICONINFO);
  }

}

void FormulaSettings::ProcessMessage(GLComponent *src,int message) {

  switch(message) {
    case MSG_BUTTON:
    if(src==applyButton) {
		// Apply
		GLParser* f=new GLParser();
		f->SetExpression(exprT->GetText());
		f->SetName(nameT->GetText());
		if (!f->Parse()) {
			DisplayError(f);
			SAFE_DELETE(f);
		}
		else {
			//add to interface
			if (formulaId == -1) { //New formula
				mApp->AddFormula(f);
			}
			else { //Update formula
				SAFE_DELETE(mApp->formulas[formulaId].parser);
				mApp->formulas[formulaId].parser = f;
				mApp->UpdateFormulaName(formulaId);
			}
			GLWindow::ProcessMessage(NULL,MSG_CLOSE);
		}
		
	} else if(src==deleteButton) {
		//delete from interface
		mApp->DeleteFormula(formulaId);
      GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    } else if(src==cancelButton) {
      GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    }
    break;
  }

  GLWindow::ProcessMessage(src,message);
}