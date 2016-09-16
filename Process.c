/*
  File:        Process.cpp
  Description: Process management
  Program:     MolFlow
  Author:      R. KERSEVAN / J-L PONS / M ADY
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

#include <windows.h>
//#include <winperf.h>
#include<Psapi.h>
#include <stdio.h>
#include "SMP.h"

static BOOL privilegeEnabled = FALSE;

//-----------------------------------------------------------------------------
// Enable required process privilege for system tasks
//-----------------------------------------------------------------------------

static BOOL EnablePrivilege() {

  HANDLE  hToken;
  LUID    DebugValue;
  TOKEN_PRIVILEGES tkp;
  DWORD err_code;

  if( !privilegeEnabled ) {

  	/* Enable privileges */

    if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &hToken)) {
	     return FALSE;
    }

    if (!LookupPrivilegeValue((LPSTR) NULL,
            SE_DEBUG_NAME,
            &DebugValue)) {
      CloseHandle(hToken);
  	  return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = DebugValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken,
        FALSE,
        &tkp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES) NULL,
        (PDWORD) NULL);

	err_code=GetLastError();

    /*if (err_code != ERROR_SUCCESS) {
      CloseHandle(hToken);
  	  return FALSE;
    }*/ //Caused privilege error codes when didn't run as administrator

    privilegeEnabled = TRUE;

  }

  return privilegeEnabled;

}

//-----------------------------------------------------------------------------
// Kill a process
//-----------------------------------------------------------------------------

BOOL KillProc(DWORD pID) {

  HANDLE p;

  if( !EnablePrivilege() ) return FALSE;
	p = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pID);
  if( p == NULL ) return FALSE;
  if( !TerminateProcess( p, 1 ) ) {
    CloseHandle(p);
    return FALSE;
  }
  CloseHandle(p);
  return TRUE;

}

//-----------------------------------------------------------------------------
// Get process info
//-----------------------------------------------------------------------------

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
static BOOL   counterInited = FALSE;

//-----------------------------------------------------------------------------
// Search performance counter
//-----------------------------------------------------------------------------

/*BOOL InitCounter() {

  if( !counterInited ) {

    DWORD                        rc;
    HKEY                         hKeyNames;
    DWORD                        dwType;
    DWORD                        dwSize;
    LPBYTE                       buf = NULL;
    LANGID                       lid;
    LPSTR                        p;
    LPSTR                        lastP;
    LPSTR                        buffSize;
    int                          counterNum;

    //
    // Look for the list of counters.  Always use the neutral
    // English version, regardless of the local language.  We
    // are looking for some particular keys, and we are always
    // going to do our looking in English.  We are not going
    // to show the user the counter names, so there is no need
    // to go find the corresponding name in the local language.
    //
    lid = MAKELANGID( LANG_ENGLISH, SUBLANG_NEUTRAL );
    sprintf( keyPerfName, "%s\\%03x", REGKEY_PERF, lid );
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       keyPerfName,
                       0,
                       KEY_READ,
                       &hKeyNames
                     );

    if (rc != ERROR_SUCCESS) {
      return FALSE;
    }

    //
    // get the buffer size for the counter names
    //
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          NULL,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
      RegCloseKey( hKeyNames );
      return FALSE;
    }

    //
    // allocate the counter names buffer
    //
    buf = (LPBYTE) malloc( dwSize );
    if (buf == NULL) {
      RegCloseKey( hKeyNames );
      return FALSE;
    }
    memset( buf, 0, dwSize );

    //
    // read the counter names from the registry
    //
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          buf,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
      free(buf);
      RegCloseKey( hKeyNames );
      return FALSE;
    }

    //
    // now loop thru the counter names looking for the following counters:
    // the buffer contains multiple null terminated strings and then
    // finally null terminated at the end.  the strings are in pairs of
    // counter number and counter name.

    p = (LPSTR)buf;
    lastP = p;
    buffSize = p + dwSize;
    counterNum = 0;

    while ( (p<buffSize) && (*p) && (counterNum<5) ) {

        if (_stricmp(p, PROCESS_COUNTER) == 0) {
          strcpy( keyPerfName, lastP );
          counterNum++;
        } else if (_stricmp(p, PROCESSID_COUNTER) == 0) {
          dwProcessIdTitle = atol( lastP );
          counterNum++;
        } else if (_stricmp(p, PROCESSTIME_COUNTER) == 0) {
          dwProcessTimeTitle = atol( lastP );
          counterNum++;
        } else if (_stricmp(p, PROCESSMEM_COUNTER) == 0) {
          dwProcessMemTitle = atol( lastP );
          counterNum++;
        } else if (_stricmp(p, PROCESSMEMP_COUNTER) == 0) {
          dwProcessMempTitle = atol( lastP );
          counterNum++;
        }

        // next counter
        lastP = p;
        p += (strlen(p) + 1);

    }

    //
    // free the counter names buffer
    //
    free( buf );
    RegCloseKey( hKeyNames );
    counterInited = TRUE;

  }
  return counterInited;

}*/

/*
BOOL GetProcInfo(DWORD pID,PROCESS_INFO *pInfo) {

    DWORD                        rc;
    DWORD                        dwType;
    DWORD                        dwSize;
    LPBYTE                       buf = NULL;
    PPERF_DATA_BLOCK             pPerf;
    PPERF_OBJECT_TYPE            pObj;
    PPERF_INSTANCE_DEFINITION    pInst;
    PPERF_COUNTER_BLOCK          pCounter;
    PPERF_COUNTER_DEFINITION     pCounterDef;
    DWORD                        i;
  	DWORD                        dwProcessIdCounter;
    DWORD                        dwProcessMempCounter;
    DWORD                        dwProcessMemCounter;
    DWORD                        dwProcessTimeCounter;

    BOOL                         pFound = FALSE;
    DWORD                        dwProcessId;

    if( !EnablePrivilege() ) return FALSE;
    if( !InitCounter() )     return FALSE;

    //
    // allocate the initial buffer for the performance data
    //
    dwSize = INITIAL_SIZE;
    buf = (LPBYTE)malloc( dwSize );
    if (buf == NULL) 
      return FALSE;
    memset( buf, 0, dwSize );


    while (TRUE) {

        rc = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                              keyPerfName,
                              NULL,
                              &dwType,
                              buf,
                              &dwSize
                            );

        pPerf = (PPERF_DATA_BLOCK) buf;

        //
        // check for success and valid perf data block signature
        //
        if ((rc == ERROR_SUCCESS) &&
            (dwSize > 0) &&
            (pPerf)->Signature[0] == (WCHAR)'P' &&
            (pPerf)->Signature[1] == (WCHAR)'E' &&
            (pPerf)->Signature[2] == (WCHAR)'R' &&
            (pPerf)->Signature[3] == (WCHAR)'F' ) {
            break;
        }

        //
        // if buffer is not big enough, reallocate and try again
        //
        if (rc == ERROR_MORE_DATA) {
          dwSize += EXTEND_SIZE;
          buf = (LPBYTE)realloc( buf, dwSize );
          memset( buf, 0, dwSize );
        } else {
          free( buf );
          return FALSE;
        }

    }

    //
    // set the perf_object_type pointer
    //
    pObj = (PPERF_OBJECT_TYPE) ((BYTE *)pPerf + pPerf->HeaderLength);

    //
    // loop thru the performance counter definition records looking
    // for the process id counter and then save its offset
    //
    pCounterDef = (PPERF_COUNTER_DEFINITION) ((BYTE *)pObj + pObj->HeaderLength);
    for (i=0; i<(DWORD)pObj->NumCounters; i++) {
        if (pCounterDef->CounterNameTitleIndex == dwProcessIdTitle) {
            dwProcessIdCounter = pCounterDef->CounterOffset;
        }
        if (pCounterDef->CounterNameTitleIndex == dwProcessTimeTitle) {
            dwProcessTimeCounter = pCounterDef->CounterOffset;
        }
        if (pCounterDef->CounterNameTitleIndex == dwProcessMemTitle) {
            dwProcessMemCounter = pCounterDef->CounterOffset;
        }
        if (pCounterDef->CounterNameTitleIndex == dwProcessMempTitle) {
            dwProcessMempCounter = pCounterDef->CounterOffset;
        }

        pCounterDef++;
    }

    pInst = (PPERF_INSTANCE_DEFINITION) ((BYTE *)pObj + pObj->DefinitionLength);

    //
    // loop thru the performance instance data extracting each process name
    // and process id
    //
    i = 0;
    while ( !pFound && (i<(DWORD)pObj->NumInstances) ) {

  	  __int64 data;

      //
      // get the process id
      //
      pCounter = (PPERF_COUNTER_BLOCK) ((BYTE *)pInst + pInst->ByteLength);
      dwProcessId = *((LPDWORD) ((BYTE *)pCounter + dwProcessIdCounter));
      pFound = (dwProcessId == pID);
      if( pFound ) {
        // Get the cpu time
        data = *(__int64 *)((PBYTE)pCounter + dwProcessTimeCounter);
		    pInfo->cpu_time = (double)data / 10000000.0;
        // get the actual memory
	      pInfo->mem_use = *((LPDWORD) ((BYTE *)pCounter + dwProcessMemCounter));
        // get the peak memory
		    pInfo->mem_peak = *((LPDWORD) ((BYTE *)pCounter + dwProcessMempCounter));
      } else {
        pInst = (PPERF_INSTANCE_DEFINITION) ((BYTE *)pCounter + pCounter->ByteLength);
        i++;
      }

		}

    free( buf );
    return pFound;

}*/

//-----------------------------------------------------------------------------
// Launch the process pname and return its PID.
//-----------------------------------------------------------------------------

DWORD StartProc(char *pname,int mode) { //minimized in Debug mode, hidden in Release mode

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	/* Launch */

	memset( &si, 0, sizeof(si) );      
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	DWORD launchMode;

#ifndef _DEBUG
	
	if (mode == STARTPROC_NORMAL) {
		si.wShowWindow = SW_SHOW;
		launchMode = DETACHED_PROCESS;
	}
	else if (mode == STARTPROC_BACKGROUND) {
		si.wShowWindow = SW_MINIMIZE;
		launchMode = CREATE_NEW_CONSOLE;
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
	else {
		si.wShowWindow = SW_SHOW;
	}


		  
#endif
	if (!CreateProcess(
		NULL,             // pointer to name of executable module
		pname,            // pointer to command line string
		NULL,             // process security attributes
		NULL,             // thread security attributes
		FALSE,            // handle inheritance flag
		launchMode | BELOW_NORMAL_PRIORITY_CLASS, // creation flags
		NULL,             // pointer to new environment block
		NULL,             // pointer to current directory name
		&si,              // pointer to STARTUPINFO
		&pi               // pointer to PROCESS_INFORMATION
	)) {

	  return 0;

	}

	return pi.dwProcessId;

}

/*
DWORD StartProc_background(char *pname) { //always starts minimized

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	

	memset( &si, 0, sizeof(si) );      
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_MINIMIZE;


	
  if( !CreateProcess(
          NULL,             // pointer to name of executable module
		      pname,            // pointer to command line string
          NULL,             // process security attributes
          NULL,             // thread security attributes
	        FALSE,            // handle inheritance flag
          CREATE_NEW_CONSOLE|BELOW_NORMAL_PRIORITY_CLASS, // creation flags
          NULL,             // pointer to new environment block
		      NULL,             // pointer to current directory name
		      &si,              // pointer to STARTUPINFO
          &pi               // pointer to PROCESS_INFORMATION
		  ) ) {
		  


	  return 0;

	}

	return pi.dwProcessId;

}

DWORD StartProc_foreground(char *pname) { //always starts minimized

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	

	memset( &si, 0, sizeof(si) );      
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;


	
  if( !CreateProcess(
          NULL,             // pointer to name of executable module
		      pname,            // pointer to command line string
          NULL,             // process security attributes
          NULL,             // thread security attributes
	        FALSE,            // handle inheritance flag
          CREATE_NEW_CONSOLE|BELOW_NORMAL_PRIORITY_CLASS, // creation flags
          NULL,             // pointer to new environment block
		      NULL,             // pointer to current directory name
		      &si,              // pointer to STARTUPINFO
          &pi               // pointer to PROCESS_INFORMATION
		  ) ) {
		  


	  return 0;

	}

	return pi.dwProcessId;

}
*/

BOOL GetProcInfo(DWORD processID, PROCESS_INFO *pInfo) {
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;

	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, processID);
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	if (ret != WAIT_TIMEOUT) return FALSE;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);

	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{

		pInfo->mem_peak=pmc.PeakWorkingSetSize;
		pInfo->mem_use=pmc.WorkingSetSize;
		/*
		//Init
		SYSTEM_INFO sysInfo;
		FILETIME ftime, fsys, fuser;
		ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
		
		GetSystemInfo(&sysInfo);

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&lastCPU, &ftime, sizeof(FILETIME));

		GetProcessTimes(hProcess, &ftime, &ftime, &fsys, &fuser);
		memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
		memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));

		
		ULARGE_INTEGER now, sys, user;
		double percent;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));

		GetProcessTimes(hProcess, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));
		percent = (sys.QuadPart - lastSysCPU.QuadPart) +
			(user.QuadPart - lastUserCPU.QuadPart);
		percent /= (now.QuadPart - lastCPU.QuadPart);
		percent /= sysInfo.dwNumberOfProcessors;
		lastCPU = now;
		lastUserCPU = user;
		lastSysCPU = sys;

		pInfo->cpu_time=percent * 100;
		*/
			pInfo->cpu_time = 0;
	}
	else {
		CloseHandle(hProcess);
		return FALSE;
	}
	CloseHandle(hProcess);
	return TRUE;
}

BOOL IsProcessRunning(DWORD pid)
{
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
}