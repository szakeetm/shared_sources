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

#include "SMP.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define NOMINMAX

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <malloc.h>
void PrintLastErrorText(LPTSTR suff);

#else
#include <cstdlib>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory.h>
#include <sys/un.h>
#include <time.h>
#include <algorithm> // std::min
#include <sys/time.h>

void PrintLastErrorText( const char* errorMsg );
int build_key (char *name)
{
    const int hash_base = 23;
    const long long hash_mod = 9827870924701019;
    int key;

/* Test dataport name length */

    if (strlen(name)>25)
    {
        fprintf(stderr,"build_key(): dataport name too long (>25)\n");
        return (-1); /* name too long */
    }

/* Build key */

    for (key = 0;*name;name++)
    {
        key = hash_base * key + *name;
    }

    return (key % hash_mod);
}
#endif

#if not (defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64))
//#if defined(__MACOSX__) || defined(__APPLE__)
// according to https://stackoverflow.com/questions/1405132/unix-osx-version-of-semtimedop
#include <signal.h>
#include <sys/ipc.h>
#include <string>

volatile int alarm_triggered = 0;
void alarm_handler(int sig)
{
    alarm_triggered = 1;
}
#endif

// create named semaphore related to dp->semaname
// ret -1 on fail
int CreateSemaphore(Dataport *dp) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    SetLastError(ERROR_SUCCESS);
    dp->sema = CreateMutex(NULL, false, dp->semaname);

    if (dp->sema == INVALID_HANDLE_VALUE) {
        PrintLastErrorText("CreateDataport(): CreateMutex() failed");
        CloseHandle(dp->mem);
        free(dp);
        return -1;
    }
#else
    key_t 	       semKey;
    int		flag;
    union semun      arg;

 flag = IPC_CREAT;

  /* Get unique key for semaphore. */

  if ((semKey = build_key(dp->semaname)) == (key_t) -1) {
  //if ((semKey = ftok("/tmp", 'M')) == (key_t) -1) {
  //if( ( semKey = (key_t) atol( dp->semaname ) ) == (key_t) -1 ){
      //fprintf(stderr,"Error for sema name %s\n",dp->semaname);
      PrintLastErrorText("CreateDataport(): atol() failed");

      free(dp);
      return -1;
  }

  flag |= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

  dp->sema  = (int) semget( semKey, 1, flag );

  if (dp->sema < 0){
      PrintLastErrorText("CreateDataport(): semget() failed");

      free(dp);
      return -1;
  }

     arg.val = 1;
  if (semctl(dp->sema, 0, SETVAL, arg) == -1){
      PrintLastErrorText("CreateDataport(): semctl() failed");

      free(dp);
      return -1;
  }

#endif

    return 0;
}

// get access to semaphore related to dp->semaname
// ret -1 on fail
int LinkSemaphore(Dataport *dp) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    //SetLastError (ERROR_SUCCESS);
    //dp->sema = CreateMutex (NULL, false, dp->semaname);
    dp->sema = OpenMutex(SYNCHRONIZE, false, dp->semaname);

    if ( /*GetLastError()!=ERROR_ALREADY_EXISTS*/ !dp->sema) {

        printf("OpenDataport(): dataport semaphore %s doesn't exist.\n", dp->semaname);
        if (dp->sema != INVALID_HANDLE_VALUE)
            CloseHandle(dp->sema);
        free(dp);
        return -1;

    }

#else
    key_t           semKey;
    int             flag;
    flag = 0;

    if ((semKey = build_key(dp->semaname)) == (key_t) -1) {
        //if ((semKey = ftok("/tmp", 'M')) == (key_t) -1) {
    //if( ( semKey = (key_t) atol(dp->semaname) ) == (key_t) -1 ){
        free(dp);
        return -1;
    }

    flag |= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH	| S_IWOTH;

    dp->sema = (int) semget( semKey, 1, flag );

    if (dp->sema  == -1){
        printf("OpenDataport(): dataport semaphore %s doesn't exist.\n",dp->semaname);
        /*if( dp->sema != INVALID_HANDLE_VALUE )
            close(dp->sema);*/
        close(dp->shmFd);
        free(dp);
        return -1;
    }
#endif
    return 0;
}

// CreateDataport: Create a block of shared memory
Dataport *CreateDataport(char *name, size_t size) {
    Dataport *dp;

    /* ------------------- Allocate new Dataport Handle ------------------- */

    dp = (Dataport *) malloc(sizeof(Dataport));

    if (dp == nullptr) {

        printf("CreateDataport(): Not enough memory...");
        free(dp);
        return nullptr;

    }

    strcpy(dp->name, name);
    sprintf(dp->semaname,"%s_sema",name); // for linux creates semaphore as /dev/sem/%s_sema
    /* ------------------- Create shared memory ------------------- */

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    SetLastError(ERROR_SUCCESS);


    // 2^32 = 4294967296      DWORD is 32-bit unsigned
    DWORD sizeHighOrder = DWORD(size >> 32);
    //DWORD sizeLowOrder = DWORD(size - (size >> 32) * 4294967296);
    DWORD sizeLowOrder = DWORD(size & 0xffffffff);

    //Debug:
    //dp->file = CreateFile(name, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    dp->mem = CreateFileMapping(
            INVALID_HANDLE_VALUE /*dp->file*/,   // to create a memory file
            NULL,                   // no security
            PAGE_READWRITE,         // to allow read & write access
            sizeHighOrder,
            sizeLowOrder,                   // file size
            name);                  // object name

    if (GetLastError() == ERROR_ALREADY_EXISTS) {

        printf("CreateDataport(): Warning connecting to existing dataport %s...\n", dp->name);

    }

    if (dp->mem == nullptr) {
        PrintLastErrorText("CreateDataport(): CreateFileMapping() failed");
        free(dp);
        return nullptr;
    }

#else
    int status = 0;

    dp->size = size;

    dp->shmFd = shm_open(name, O_RDWR | O_CREAT, 0777);
    if (dp->shmFd < 0) {
        PrintLastErrorText("CreateDataport(): shm_open() failed");
        free(dp);
        return nullptr;
    }

#if defined(__MACOSX__) || defined(__APPLE__)
// Increase shared memory size from 0 to size
    struct stat mapstat;
    if (-1 != fstat(dp->shmFd, &mapstat) && mapstat.st_size == 0) {
        status = ftruncate(dp->shmFd, size);
    }
    else
        return dp;
    if (status != 0) {
        PrintLastErrorText("CreateDataport(): ftruncate() failed");
        free(dp);
        return nullptr;
    }
#else
    // Increase shared memory size from 0 to size
    status = ftruncate(dp->shmFd, size);
    if (status != 0) {
        PrintLastErrorText("CreateDataport(): ftruncate() failed");
        free(dp);
        return nullptr;
    }
#endif //__APPLE__

#endif
    /* ------------------- Create the semaphore ------------------- */
    if (CreateSemaphore(dp) == -1) {
        return nullptr;
    }

    /* ------------------- Map the memomy ------------------- */
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

    dp->buff = MapViewOfFile(dp->mem, FILE_MAP_WRITE, 0, 0, 0); //With this function write access equals all_access


    if (dp->buff == nullptr) {

        PrintLastErrorText("CreateDataport(): MapViewOfFile() failed");
        CloseHandle(dp->mem);
        CloseHandle(dp->sema);
        free(dp);
        return nullptr;

    }
#else

    dp->buff = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, dp->shmFd, 0);
    if (dp->buff == nullptr || dp->buff == MAP_FAILED) {
        PrintLastErrorText("CreateDataport(): mmap() failed");
        close(dp->shmFd);
        close(dp->sema);
        free(dp);
        return nullptr;
    }
#endif
    //memset(dp->buff, 0, size);//Debug

    dp->size = size;
    return (dp);
}

// OpenDataport: Connect to an existing block

Dataport *OpenDataport(char *name, size_t size) {
    Dataport *dp;
    /* ------------------- Allocate new Dataport Handle ------------------- */

    dp = (Dataport *) malloc(sizeof(Dataport));

    if (dp == nullptr) {

        printf("OpenDataport(): Not enough memory...");
        free(dp);
        return nullptr;

    }

    strcpy(dp->name, name);
    sprintf(dp->semaname, "%s_sema", name);

    /* ------------------- Link to the share memory ------------------- */

    //
    //// 2^32 = 4294967296      DWORD is 32-bit unsigned
    //DWORD sizeHighOrder = DWORD(size >> 32);
    //DWORD sizeLowOrder = DWORD(size - (size >> 32) * 4294967296);

    ////dp->file = CreateFile(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    //dp->mem = CreateFileMapping(
    //          INVALID_HANDLE_VALUE /*dp->file*/,   // to create a memory file
    //      		NULL,                   // no security
    //      	PAGE_READWRITE,         // to allow read & write access
    //     		sizeHighOrder,
    //     		sizeLowOrder,                   // file size
    //      	name);                  // object name
    //


#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    dp->mem = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, name);

    if ( /*GetLastError()!=ERROR_ALREADY_EXISTS*/ !dp->mem) {

        printf("OpenDataport(): dataport %s doesn't exist.\n", dp->name);
        if (dp->mem != INVALID_HANDLE_VALUE)
            CloseHandle(dp->mem);
        free(dp);
        return NULL;

    }

#else
    dp->shmFd = shm_open(name, O_RDWR, 0777);
    if (dp->shmFd < 0) {
        std::string errorString = "OpenDataport(): shm_open() failed for file ";
        errorString.append(name);
        PrintLastErrorText(errorString.c_str());
        close(dp->shmFd);
        free(dp);
        return nullptr;
    }
#endif
    /* ------------------- Link to the semaphore ------------------- */
    LinkSemaphore(dp);

    /* ------------------- Map the memory ------------------- */

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    dp->buff = MapViewOfFile(dp->mem, FILE_MAP_WRITE, 0, 0, 0);  //With this function write access equals all_access

    if (dp->buff == NULL) {

        PrintLastErrorText("OpenDataport(): MapViewOfFile() failed");
        free(dp);
        return NULL;

    }
#else
    dp->buff = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, dp->shmFd, 0);
    if (dp->buff == nullptr || dp->buff == MAP_FAILED) {
        PrintLastErrorText("OpenDataport(): mmap() failed");
        close(dp->shmFd);
        close(dp->sema);
        free(dp);
        return nullptr;
    }
#endif

    dp->size = size;
    return (dp);
}

// Get access by getting ownership of the semaphore
bool AccessDataport(Dataport *dp) {

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    DWORD retVal = WaitForSingleObject(dp->sema, 8000);
    if (retVal == WAIT_OBJECT_0)
        return true;
    else
        return false;
#elif defined(__MACOSX__) || defined(__APPLE__)
    struct sembuf   semBuf;
    semBuf.sem_num = 0;
    semBuf.sem_op = -1;
    semBuf.sem_flg = SEM_UNDO | IPC_NOWAIT;

    if (semop(dp->sema, &semBuf, 1) != 0) {
        semBuf.sem_flg = SEM_UNDO;

        /* set up signal handler */
        signal(SIGALRM, alarm_handler);
        int rc;
        alarm(8); /* 8 second timeout */

        if ((rc = semop(dp->sema, &semBuf, 1)) != 0) { // if (rc == -1 && errno == EINTR)
            char errMsg[128];
            sprintf(errMsg, "[%s / %d] Locking Mutex failed",dp->semaname,getpid());
            PrintLastErrorText(errMsg);
            return false;
        }
        alarm(0); /* disable alarm */
    }
    return true;
#else
    struct sembuf   semBuf;
    semBuf.sem_num = 0;
    semBuf.sem_op = -1;
    semBuf.sem_flg = SEM_UNDO | IPC_NOWAIT;


    //printf("[%s / %d] Trying to lock! -> %d\n",dp->semaname,getpid(), semctl(dp->sema, 0, GETVAL));
    if (semop(dp->sema, &semBuf, 1) != 0) {
        semBuf.sem_flg = SEM_UNDO;
        struct timespec sem_timeout;
        sem_timeout.tv_sec = 8;
        sem_timeout.tv_nsec = 0;
        if (semtimedop(dp->sema, &semBuf, 1, &sem_timeout) != 0) {
            char errMsg[128];
            sprintf(errMsg, "[%s / %d] Locking Mutex failed",dp->semaname,getpid());
            PrintLastErrorText(errMsg);
            return false;
        }
    }
    //printf("[%s / %d] Semaphore locked!\n",dp->semaname,getpid());
    return true;
#endif
}

// timeout in milliseconds
bool AccessDataportTimed(Dataport *dp, DWORD timeout) {

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    DWORD retVal = WaitForSingleObject(dp->sema, timeout);
    if (retVal == WAIT_OBJECT_0)
        return true;
    else
        return false;
#elif defined(__MACOSX__) || defined(__APPLE__)
    struct sembuf   semBuf;
    semBuf.sem_num = 0;
    semBuf.sem_op = -1;
    semBuf.sem_flg = SEM_UNDO | IPC_NOWAIT;

    if (semop(dp->sema, &semBuf, 1) != 0) {
        semBuf.sem_flg = SEM_UNDO;

        /* set up signal handler */
        signal(SIGALRM, alarm_handler);
        int rc;
        alarm(std::min(1u,timeout / 1000)); /* timeout in ms to seconds */

        if ((rc = semop(dp->sema, &semBuf, 1)) != 0) { // if (rc == -1 && errno == EINTR)
            char errMsg[128];
            sprintf(errMsg, "[%s / %d] Locking Mutex failed",dp->semaname,getpid());
            PrintLastErrorText(errMsg);
            return false;
        }
        alarm(0); /* disable alarm */
    }
    return true;
#else
    struct sembuf   semBuf;
    semBuf.sem_num = 0;
    semBuf.sem_op = -1;
    semBuf.sem_flg = SEM_UNDO | IPC_NOWAIT;
    if (semop(dp->sema, &semBuf, 1) != 0) {
        semBuf.sem_flg = SEM_UNDO;
        struct timespec sem_timeout;
        sem_timeout.tv_sec = timeout / 1000;
        sem_timeout.tv_nsec = (timeout % 1000) * 1000000;
        if (semtimedop(dp->sema, &semBuf, 1, &sem_timeout) != 0) {
            //char errMsg[128];
            //sprintf(errMsg, "[%s / %d] Locking Mutex failed",dp->semaname,getpid());
            //PrintLastErrorText(errMsg);
            return false;
        }
    }
    return true;
#endif
}

bool ReleaseDataport(Dataport *dp) {

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    if (dp)
        if (ReleaseMutex(dp->sema) == 0)
            return true;
        else
            return false;
#else
    if (dp){
        struct sembuf   semBuf;
        semBuf.sem_num = 0;
        semBuf.sem_op = 1;
        semBuf.sem_flg = SEM_UNDO;
        if (semop(dp->sema, &semBuf, 1) == 0){
            //printf("[%s / %d] Semaphore unlocked!\n",dp->semaname,getpid());
            return true;
        }
        else{
            //char errMsg[128];
            //sprintf(errMsg, "[%s / %d] Unlocking Mutex failed",dp->semaname,getpid());
            //PrintLastErrorText(errMsg);
            return false;
        }
    }
#endif
    return false;
}

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

bool CloseDataport(Dataport *dp) {
#else
    bool CloseDataport(Dataport *dp, bool unlinkShm) {
#endif
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    UnmapViewOfFile(dp->buff);
    CloseHandle(dp->mem);
    CloseHandle(dp->sema);
    //CloseHandle(dp->file); //Debug
    //DeleteFile(dp->name); //Debug: will only succeed if all handles released
    free(dp);
    return true;
#else
    if (dp->buff) {
        munmap(dp->buff, dp->size);
    }
    if (dp->shmFd) {
        if(unlinkShm)
            shm_unlink(dp->name);
        close(dp->shmFd);
    }
    if (dp->sema) {
        if(unlinkShm)
            shm_unlink(dp->semaname);
        close(dp->sema);
    }
    free(dp);
    return true;
#endif
}

// Timing stuff

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
bool usePerfCounter;         // Performance counter usage
LARGE_INTEGER perfTickStart; // First tick
double perfTicksPerSec;      // Performance counter (number of tick per second)
DWORD tickStart;
#else
struct timespec tickStart;
#endif

void InitTick(){
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
		LARGE_INTEGER qwTicksPerSec;
		usePerfCounter = QueryPerformanceFrequency(&qwTicksPerSec);
		if (usePerfCounter) {
			QueryPerformanceCounter(&perfTickStart);
			perfTicksPerSec = (double)qwTicksPerSec.QuadPart;
		}
		tickStart = GetTickCount();
#else
    clock_gettime(CLOCK_MONOTONIC, &tickStart);
#endif
};
double GetTick() {

    // Number of sec since the application startup
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    if (usePerfCounter) {
		LARGE_INTEGER t, dt;
		QueryPerformanceCounter(&t);
		dt.QuadPart = t.QuadPart - perfTickStart.QuadPart;
		return (double)(dt.QuadPart) / perfTicksPerSec;
	}
    else {
		return (double)((GetTickCount() - tickStart) / 1000.0);
	}
#else

    struct timespec ts_end{};
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    uint64_t time_diff = (ts_end.tv_sec - tickStart.tv_sec) * 1000000000ULL + ts_end.tv_nsec -
            tickStart.tv_nsec; // diff rounded to nsec to prevent errors
    
    return ((double)time_diff / 1e9);
#endif
}

DWORD GetSeed() {
    int processId;
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    processId = _getpid();
#else
    processId = ::getpid();
#endif //  WIN

    return (DWORD)((int)(GetTick()*1000.0)*processId);
}


#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

void PrintLastErrorText(LPTSTR suff) {
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          NULL,
                          GetLastError(),
                          LANG_NEUTRAL,
                          (LPTSTR) &lpszTemp,
                          0,
                          NULL);

    printf(TEXT("%s:%s (%d [0x%x])"), suff, lpszTemp, GetLastError(), GetLastError());

    if (lpszTemp)
        LocalFree((HLOCAL) lpszTemp);

}

#else
void PrintLastErrorText( const char* errorMsg )
{
    fprintf(stderr,"%s:%s (%d [0x%x])\n", errorMsg, strerror(errno), errno, errno);

}
#endif