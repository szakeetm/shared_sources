
#ifndef _SMPH_
#define _SMPH_

#include "ProcessControl.h"

#define STARTPROC_NORMAL 0
#define STARTPROC_BACKGROUND 1
#define STARTPROC_FOREGROUND 2
#define STARTPROC_NOWIN 3

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <windows.h>

 // Win32 shared memory
    struct Dataport{
	 char              name[32]; //Unique identifier
	 char              semaname[32]; //Mutex unique identifier
     HANDLE            sema; //Mutex handle (CreateMutex return value)
	 HANDLE            mem; //File mapping handle (CreateFileMapping return value)
	 HANDLE file;			//Physical file handle (if persistent)
	 size_t size;		//keep track of mapped size
	 void              *buff; //View handle (MapViewOfFile return value, pointer to data)
 };

#else
#include <time.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

 using DWORD = unsigned int;
 using WORD = unsigned short;
#if !defined(__MACOSX__) && !defined(__APPLE__)
 union semun {
    int                 val;   /* value for SETVAL             */
    struct semid_ds    *buf;   /* buffer for IPC_STAT, IPC_SET */
    unsigned short     *array; /* array for GETALL, SETALL     */
};
#endif
 // Linux shared memory
struct Dataport {
    char              name[32]; //Unique identifier
    char              semaname[32]; //Mutex unique identifier
    int            sema; //Mutex handle (CreateMutex return value)
    int            shmFd; //File mapping handle (CreateFileMapping return value)
    int file;			//Physical file handle (if persistent)
    size_t size;		//keep track of mapped size
    void              *buff; //View handle (MapViewOfFile return value, pointer to data)
 };

#endif

#define MAX_PROCESS (size_t)64    // Maximum number of process

struct SHCONTROL {
    // Process control
    ProcComm procInformation[MAX_PROCESS];
};



// Shared memory
Dataport *CreateDataport(char *name, size_t size);
Dataport *OpenDataport(char *name, size_t size);
bool AccessDataport(Dataport *dp);
bool AccessDataportTimed(Dataport *dp, DWORD timeout);
bool ReleaseDataport(Dataport *dp);
#ifdef _WIN32
bool CloseDataport(Dataport *dp);
#else
bool CloseDataport(Dataport *dp, bool unlinkShm);
#endif
// Process management
bool          KillProc(DWORD pID);
bool          GetProcInfo(DWORD pID,PROCESS_INFO *pInfo);
DWORD StartProc(char **procv, int mode);
bool IsProcessRunning(DWORD pID);

void InitTick();
double GetTick();
DWORD GetSeed();

// Helper functions
inline void ProcessSleep(const unsigned int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    struct timespec tv;
    tv.tv_sec = milliseconds / 1000;
    tv.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&tv, nullptr);
#endif
}

/*extern DWORD         StartProc_background(char *pname);
extern DWORD         StartProc_foreground(char *pname); //TODO: unite these three*/

// seperate
#ifdef _WIN32
#define CLOSEDP(dp) if(dp) { CloseDataport(dp);(dp)=NULL; }
#define CLOSEDPSUB(dp) CLOSEDP(dp)
#else
#define CLOSEDP(dp) if(dp) { CloseDataport(dp,true);(dp)=NULL; }
#define CLOSEDPSUB(dp) if(dp) { CloseDataport(dp,false);(dp)=NULL; }
#endif
#ifdef __cplusplus
}
#endif

#endif /* _SMPH_ */
