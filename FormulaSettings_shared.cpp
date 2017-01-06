#include "FormulaSettings.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"

BOOL FormulaSettings::EditFormula(GLParser *f) {

  createButton->SetText("Apply");
  deleteButton->SetEnabled(TRUE);
  nameT->SetText(f->GetName());
  exprT->SetText(f->GetExpression());
  rCode = 0;
  DoModal();

  if( rCode==2 ) {
    // Delete
    return FALSE;
  } else if (rCode==1) {
    // Apply
    f->SetExpression(exprT->GetText());
    f->SetName(nameT->GetText());
    if( !f->Parse() ) 
      DisplayError(f);
  }

  return TRUE;
}

GLParser *FormulaSettings::NewFormula() {

  createButton->SetText("Create");
  deleteButton->SetEnabled(FALSE);
  nameT->SetText("");
  exprT->SetText("");
  rCode = 0;
  DoModal();

  if (rCode==1) {
    // Create
    GLParser *f = new GLParser();
    f->SetExpression(exprT->GetText());
    f->SetName(nameT->GetText());
    if( !f->Parse() )
      DisplayError(f);
    return f;
  }

  return NULL;

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
    if(src==createButton) {
      rCode = 1;
      GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    } else if(src==deleteButton) {
      rCode = 2;
      GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    } else if(src==cancelButton) {
      rCode = 0;
      GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    }
    break;
  }

  GLWindow::ProcessMessage(src,message);
}