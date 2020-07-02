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


#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define NOMINMAX
#include <windows.h>
//#include <winperf.h>
#include<Psapi.h>
#elif defined(__MACOSX__) || defined(__APPLE__)
#include <mach/mach.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/types.h>
#else
#include <sys/types.h>
#include <signal.h>
#include <fstream>
#endif
#include <stdio.h>
#include <cerrno>
#include <cstring>

#include "SMP.h"



#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
// Get process info
#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         "software\\microsoft\\windows nt\\currentversion\\perflib"
#define REGSUBKEY_COUNTERS  "Counters"
#define PROCESS_COUNTER     "process"
#define PROCESSID_COUNTER   "id process"
#define PROCESSTIME_COUNTER "% Processor Time"
#define PROCESSMEM_COUNTER  "Working Set"
#define PROCESSMEMP_COUNTER "Working Set Peak"

static DWORD  dwProcessIdTitle;
static DWORD  dwProcessMempTitle;
static DWORD  dwProcessMemTitle;
static DWORD  dwProcessTimeTitle;
static CHAR   keyPerfName[1024];
static bool   counterInited = false;
#else

#endif



#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
static bool privilegeEnabled = false;

// Enable required process privilege for system tasks

static bool EnablePrivilege() {

  HANDLE  hToken;
  LUID    DebugValue;
  TOKEN_PRIVILEGES tkp;
  DWORD err_code;

  if( !privilegeEnabled ) {

  	/* Enable privileges */

    if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &hToken)) {
	     return false;
    }

    if (!LookupPrivilegeValue((LPSTR) NULL,
            SE_DEBUG_NAME,
            &DebugValue)) {
      CloseHandle(hToken);
  	  return false;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = DebugValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken,
        false,
        &tkp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES) NULL,
        (PDWORD) NULL);

	err_code=GetLastError();

    /*if (err_code != ERROR_SUCCESS) {
      CloseHandle(hToken);
  	  return false;
    }*/ //Caused privilege error codes when didn't run as administrator

    privilegeEnabled = true;

  }

  return privilegeEnabled;

}
#endif

// Kill a process
bool KillProc(DWORD pID) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    HANDLE p;

    if( !EnablePrivilege() )
        return false;
    p = OpenProcess(PROCESS_ALL_ACCESS,false,pID);
    if( p == NULL )
        return false;
    if( !TerminateProcess( p, 1 ) ) {
        CloseHandle(p);
        return false;
    }
    CloseHandle(p);
    return true;
#else
    return !kill(pID, SIGKILL); // true if (kill==0)
#endif
}

// Launch the process procv[0] and return its PID.
DWORD StartProc(char **procv, int mode) { //minimized in Debug mode, hidden in Release mode
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    PROCESS_INFORMATION pi;
	STARTUPINFO si;

	/* Launch */

	memset( &si, 0, sizeof(si) );
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	DWORD launchMode;

#if not defined(_DEBUG) || not defined(DEBUG)
	if (mode == STARTPROC_NORMAL) {
		si.wShowWindow = SW_SHOW;
		launchMode = DETACHED_PROCESS;
	}
	else if (mode == STARTPROC_BACKGROUND) {
		si.wShowWindow = SW_MINIMIZE;
		launchMode = CREATE_NEW_CONSOLE;
	}
    else if (mode == STARTPROC_NOWIN) {
        si.wShowWindow = SW_MINIMIZE;
        launchMode = CREATE_NO_WINDOW;
    }
	else {
		si.wShowWindow = SW_SHOW;
		launchMode = CREATE_NEW_CONSOLE;
	}
#else
	launchMode = CREATE_NEW_CONSOLE;
	if (mode == STARTPROC_NORMAL) {
		si.wShowWindow = SW_MINIMIZE;
	}
	else if (mode == STARTPROC_BACKGROUND) {
		si.wShowWindow = SW_MINIMIZE;
	}
    else if (mode == STARTPROC_NOWIN) {
        si.wShowWindow = SW_MINIMIZE;
        //launchMode = CREATE_NO_WINDOW;
    }
	else {
		si.wShowWindow = SW_SHOW;
	}

#endif
    //Temporary patch
    launchMode = CREATE_NO_WINDOW;
    //End patch
	if (!CreateProcess(
		NULL,             // pointer to name of executable module
		procv[0],            // pointer to command line string
		NULL,             // process security attributes
		NULL,             // thread security attributes
		false,            // handle inheritance flag
		launchMode | IDLE_PRIORITY_CLASS, // creation flags
		NULL,             // pointer to new environment block
		NULL,             // pointer to current directory name
		&si,              // pointer to STARTUPINFO
		&pi               // pointer to PROCESS_INFORMATION
	)) {

	  return 0;

	}

	return pi.dwProcessId;
#else
    pid_t process = fork();

    if (process == 0) { // child process
        if (0 > execvp(procv[0], procv)) {
            fprintf(stderr," StartProc error: %s\n",std::strerror(errno));
            return 0;
        }
    } else if (process < 0) {
        return 0;
    }
    return process;
#endif
}

bool GetProcInfo(DWORD processID, PROCESS_INFO *pInfo) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;

	HANDLE process = OpenProcess(SYNCHRONIZE, false, processID);
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	if (ret != WAIT_TIMEOUT) return false;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		false, processID);

	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		pInfo->mem_peak=pmc.PeakWorkingSetSize;
		pInfo->mem_use=pmc.WorkingSetSize;
        pInfo->cpu_time = 0;
	}
	else {
		CloseHandle(hProcess);
		return false;
	}
	CloseHandle(hProcess);
	return true;
#else
    /* BSD, Linux, and OSX -------------------------------------- */
    //void mem_usage(double& vm_usage, double& resident_set) {
   unsigned long vm_size = 0u;
   unsigned long resident_set = 0u;

#if defined(__MACOSX__) || defined(__APPLE__)

    if(processID!=getpid()){
        vm_size = (size_t)0L;      /* Can't access? */
        resident_set =  (size_t)0L;      /* Can't access? */
    }
    else{
        struct rusage rusage;
        getrusage( RUSAGE_SELF, &rusage );
        vm_size = (size_t)rusage.ru_maxrss / 1024.0;

        struct mach_task_basic_info info;
        mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
        if ( task_info( mach_task_self( ), MACH_TASK_BASIC_INFO,(task_info_t)&info, &infoCount ) != KERN_SUCCESS )
            resident_set =  (size_t)0L;      /* Can't access? */
        else
            resident_set =  (size_t)info.resident_size / 1024.0;
    }






#else
   std::ifstream stat_stream("/proc/"+std::to_string(processID)+"/statm",std::ios_base::in); //get info from proc directory
   stat_stream >> vm_size >> resident_set; // don't care about the rest
   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) /*/ 1024*/; // most commonly 4kB pages used
   vm_size = vm_size * page_size_kb / 1024.0;
   resident_set = resident_set * page_size_kb / 1024.0;

#endif

pInfo->mem_peak=vm_size;
pInfo->mem_use=resident_set;
pInfo->cpu_time = 0;
return true;

#endif
}

bool IsProcessRunning(DWORD pid)
{

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    HANDLE process = OpenProcess(SYNCHRONIZE, false, pid);
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
#else
    if (0 == kill(pid, 0))
        return true; // process is running
    else
        return false;

#endif
}