/*
  File:        SmpStatus.cpp
  Description: SMP status dialog
  Program:     SynRad
  Author:      R. KERSEVAN / M ADY
  Copyright:   E.S.R.F / CERN

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include "SmpStatus.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLInputBox.h"
#include "Utils.h"

static const int   plWidth[] = {40,70,70,50,345};
static const char *plName[] = {"PID","Mem Usage","Mem Peak","CPU","Status"};
static const int   plAligns[] = { ALIGN_CENTER,ALIGN_CENTER,ALIGN_CENTER,ALIGN_CENTER,ALIGN_LEFT };

// ----------------------------------------------------------------

SmpStatus::SmpStatus():GLWindow() {

  int wD = 600;
  int hD = 300;

  SetTitle("Process status");
  SetIconfiable(TRUE);

  processList = new GLList(0);
  processList->SetHScrollVisible(TRUE);
  processList->SetSize(5,MAX_PROCESS);
  processList->SetColumnWidths((int*)plWidth);
  processList->SetColumnLabels((char **)plName);
  processList->SetColumnAligns((int *)plAligns);
  processList->SetColumnLabelVisible(TRUE);
  processList->SetBounds(5,5,wD-10,hD-55);
  Add(processList);

  restartButton = new GLButton(0,"Restart");
  restartButton->SetBounds(5,hD-43,90,19);
  Add(restartButton);

  GLLabel *l1 = new GLLabel("Process number");
  l1->SetBounds(105,hD-41,100,19);
  Add(l1);

  nbProcText = new GLTextField(0,"");
  nbProcText->SetEditable(TRUE);
  nbProcText->SetBounds(210,hD-43,30,19);
  Add(nbProcText);

  maxButton = new GLButton(0,"Change MAX generated");
  maxButton->SetBounds(wD-280,hD-43,180,19);
  Add(maxButton);

  dismissButton = new GLButton(0,"Dismiss");
  dismissButton->SetBounds(wD-95,hD-43,90,19);
  Add(dismissButton);


  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  int xD = (wS-wD)/2;
  int yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  RestoreDeviceObjects();

  /*lastUpdate = 0.0f;
  for(int i=0;i<MAX_PROCESS;i++) lastCPUTime[i]=-1.0f;
  memset(lastCPULoad,0,MAX_PROCESS*sizeof(float));*/

}

// ----------------------------------------------------------------

void SmpStatus::Display(Worker *w) {
  char tmp[64];
  worker = w;
  int nb = worker->GetProcNumber();
  sprintf(tmp,"%d",nb);
  nbProcText->SetText(tmp);
  SetVisible(TRUE);
}

// ----------------------------------------------------------------

void SmpStatus::Update(float appTime) {

  if(!IsVisible() || IsIconic()) return;  
  int nb = worker->GetProcNumber();

  if( appTime-lastUpdate>1.0 && nb>0 ) {

    char tmp[512];
    PROCESS_INFO pInfo;
    int  states[MAX_PROCESS];
    char statusStr[MAX_PROCESS][64];

    memset(states,0,MAX_PROCESS*sizeof(int));
    worker->GetProcStatus(states,(char **)statusStr);

    processList->ResetValues();
    for(int i=0;i<nb;i++) {
      DWORD pid = worker->GetPID(i);
      sprintf(tmp,"%d",pid);
      processList->SetValueAt(0,i,tmp);
      if( !GetProcInfo(pid,&pInfo) ) {
        processList->SetValueAt(1,i,"0 KB");
        processList->SetValueAt(2,i,"0 KB");
        processList->SetValueAt(3,i,"0 %");
        processList->SetValueAt(4,i,"Dead");
      } else {
        processList->SetValueAt(1,i,FormatMemory(pInfo.mem_use));
        processList->SetValueAt(2,i,FormatMemory(pInfo.mem_peak));

        // CPU usage
        if( lastCPUTime[i]!=-1.0f ) {
          float dTime = appTime-lastUpdate;
          float dCPUTime = (float)pInfo.cpu_time-lastCPUTime[i];
          float cpuLoad = dCPUTime/dTime;
          lastCPULoad[i] = 0.85f*cpuLoad + 0.15f*lastCPULoad[i];
          int percent = (int)(100.0f*lastCPULoad[i] + 0.5f);
          if(percent<0) percent=0;
          sprintf(tmp,"%d %%",percent);
          processList->SetValueAt(3,i,tmp);
        } else {
          processList->SetValueAt(3,i,"---");
        }
        lastCPUTime[i] = (float)pInfo.cpu_time;

        // State/Status
        char status[128];
        _snprintf(tmp,127,"%s: %s",prStates[states[i]],statusStr[i]);
        status[127]=0;
        processList->SetValueAt(4,i,tmp);

      }
    }

    lastUpdate = appTime;
  }

}

// ----------------------------------------------------------------

void SmpStatus::RestartProc() {

 int nbProc;
 if( sscanf(nbProcText->GetText(),"%d",&nbProc)==0 ) {
   GLMessageBox::Display("Invalid process number","Error",GLDLG_OK,GLDLG_ICONERROR);
 } else {
   char tmp[128];
   sprintf(tmp,"Kill all running sub-process(es) and start %d new ones ?",nbProc);
   int rep = GLMessageBox::Display(tmp,"Question",GLDLG_OK|GLDLG_CANCEL,GLDLG_ICONWARNING);
   if( rep == GLDLG_OK ) {
     if(nbProc<=0 || nbProc>32) {
       GLMessageBox::Display("Invalid process number [1..32]","Error",GLDLG_OK,GLDLG_ICONERROR);
     } else {
       try {
         worker->SetProcNumber(nbProc);
         worker->Reload();
       } catch(Error &e) {
         GLMessageBox::Display((char *)e.GetMsg(),"Error",GLDLG_OK,GLDLG_ICONERROR);
       }
     }
   }
 }

}

// ----------------------------------------------------------------

void SmpStatus::ProcessMessage(GLComponent *src,int message) {

  switch(message) {

    case MSG_BUTTON:
      if(src==dismissButton) {
        SetVisible(FALSE);
      } else if (src==restartButton) {
        RestartProc();
      } else if (src==maxButton) {
        if( worker->GetGeometry()->IsLoaded() ) {
          char tmp[128];
          sprintf(tmp,"%I64d",worker->maxDesorption);
          char *val = GLInputBox::GetInput(tmp,"Desorption max (0=>endless)","Edit MAX");
          if( val ) {
            llong maxDes;
            if( sscanf(val,"%I64d",&maxDes)==0 ) {
              GLMessageBox::Display("Invalid 'maximum desorption' number","Error",GLDLG_OK,GLDLG_ICONERROR);
            } else {
              worker->SetMaxDesorption(maxDes);
            }
          }
        } else {
          GLMessageBox::Display("No file loaded","Error",GLDLG_OK,GLDLG_ICONERROR);
        }
      }
      break;

  }

  GLWindow::ProcessMessage(src,message);

}

