/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
#ifndef _SMPH_
#define _SMPH_

#define STARTPROC_NORMAL 0
#define STARTPROC_BACKGROUND 1
#define STARTPROC_FOREGROUND 2

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <windows.h>

 // Win32 shared memory
 typedef struct {
	 char              name[32]; //Unique identifier
	 char              semaname[32]; //Mutex unique identifier
     HANDLE            sema; //Mutex handle (CreateMutex return value)
	 HANDLE            mem; //File mapping handle (CreateFileMapping return value)
	 HANDLE file;			//Physical file handle (if persistent)
	 size_t size;		//keep track of mapped size
	 void              *buff; //View handle (MapViewOfFile return value, pointer to data)
 } Dataport;

#else
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

 using DWORD = unsigned int;
 using WORD = unsigned short;

 union semun {
    int                 val;   /* value for SETVAL             */
    struct semid_ds    *buf;   /* buffer for IPC_STAT, IPC_SET */
    unsigned short     *array; /* array for GETALL, SETALL     */
};

 // Linux shared memory
 typedef struct {
/*   int              sema;
   int              shar;
   int              key;
   pid_t            creator_pid;
   char             body;*/
    char              name[32]; //Unique identifier
    char              semaname[32]; //Mutex unique identifier
    int            sema; //Mutex handle (CreateMutex return value)
    int            shmFd; //File mapping handle (CreateFileMapping return value)
    int file;			//Physical file handle (if persistent)
    size_t size;		//keep track of mapped size
    void              *buff; //View handle (MapViewOfFile return value, pointer to data)
 } Dataport;

#endif

typedef struct {

  double cpu_time; // CPU time         (in second)
  size_t  mem_use;  // Memory usage     (in byte)
  size_t  mem_peak; // MAx Memory usage (in byte)

} PROCESS_INFO;

// Shared memory
Dataport *CreateDataport(char *name, size_t size);
Dataport *OpenDataport(char *name, size_t size);
bool AccessDataport(Dataport *dp);
bool AccessDataportTimed(Dataport *dp, DWORD timeout);
bool ReleaseDataport(Dataport *dp);
bool CloseDataport(Dataport *dp);

// Process management
bool          KillProc(DWORD pID);
bool          GetProcInfo(DWORD pID,PROCESS_INFO *pInfo);
DWORD         StartProc(const char *pname,int mode, char **argv);
bool IsProcessRunning(DWORD pID);

/*extern DWORD         StartProc_background(char *pname);
extern DWORD         StartProc_foreground(char *pname); //TODO: unite these three*/

#define CLOSEDP(dp) if(dp) { CloseDataport(dp);(dp)=NULL; }

#ifdef __cplusplus
}
#endif

#endif /* _SMPH_ */
