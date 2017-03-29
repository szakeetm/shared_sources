/*
  File:        SMP.h
  Description: Multi-processing utility routines (Symmetric MultiProcessing)
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
#define STARTPROC_NORMAL 0
#define STARTPROC_BACKGROUND 1
#define STARTPROC_FOREGROUND 2


#ifndef _SMPH_
#define _SMPH_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN

 #include <windows.h>

 // Win32 shared memory
 typedef struct {
	 char              name[32];
	 char              semaname[32];
   HANDLE            sema;
	 HANDLE            mem;
	 HANDLE file; //Debug
	 void              *buff;
 } Dataport;

#else

#include <sys/types.h>

 // Linux shared memory
 typedef struct {
   int              sema;
   int              shar;
   int              key;
   pid_t            creator_pid;
   char             body;
 } Dataport;

#endif

typedef struct {

  double cpu_time; // CPU time         (in second)
  DWORD  mem_use;  // Memory usage     (in byte)
  DWORD  mem_peak; // MAx Memory usage (in byte)

} PROCESS_INFO;

// Shared memory
Dataport *CreateDataport(char *name, size_t size);
Dataport *OpenDataport(char *name, size_t size);
BOOL AccessDataport(Dataport *dp);
BOOL AccessDataportTimed(Dataport *dp, DWORD timeout);
BOOL ReleaseDataport(Dataport *dp);
BOOL CloseDataport(Dataport *dp);

// Process management
BOOL          KillProc(DWORD pID);
BOOL          GetProcInfo(DWORD pID,PROCESS_INFO *pInfo);
DWORD         StartProc(char *pname,int mode);
BOOL IsProcessRunning(DWORD pID);

/*extern DWORD         StartProc_background(char *pname);
extern DWORD         StartProc_foreground(char *pname); //TODO: unite these three*/

#define CLOSEDP(dp) if(dp) { CloseDataport(dp);(dp)=NULL; }

#ifdef __cplusplus
}
#endif

#endif /* _SMPH_ */
