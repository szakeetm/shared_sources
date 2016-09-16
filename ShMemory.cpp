/*
  File:        ShMemory.cpp
  Description: Shared memory management
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

#ifdef WIN

#include <windows.h>
#include <stdio.h>
#include "SMP.h"

void PrintLastErrorText( LPTSTR suff );

// --------------------------------------------------------------------------
// CreateDataport: Create a block of shared memory
// --------------------------------------------------------------------------

Dataport *CreateDataport(char *name,size_t size)
{
   Dataport  *dp;
   
   /* ------------------- Allocate new Dataport Handle ------------------- */

   dp = (Dataport *)malloc(sizeof(Dataport));

   if( dp == NULL ) {
   
 	   printf("CreateDataport(): Not enough memory...");
     free(dp);
	   return NULL;
  
   }

   strcpy(dp->name,name);
   sprintf(dp->semaname,"%s_sema",name);

   /* ------------------- Create shared memory ------------------- */

   SetLastError (ERROR_SUCCESS);
   dp->mem = CreateFileMapping(
	            INVALID_HANDLE_VALUE,   // to create a memory file
			      	NULL,                   // no security 
				      PAGE_READWRITE,         // to allow read & write access
				      0,     
				      (DWORD)size,                   // file size     
				      name);                  // object name      

   if( GetLastError()==ERROR_ALREADY_EXISTS ) {
	 
	   printf("CreateDataport(): Warning connecting to existing dataport %s...\n",dp->name);

   }
   
   if( dp->mem==NULL ) {

	   PrintLastErrorText("CreateDataport(): CreateFileMapping() failed");
     free(dp);
	   return NULL;

   }

   /* ------------------- Create the semaphore ------------------- */

   SetLastError (ERROR_SUCCESS);
   dp->sema = CreateMutex (NULL, FALSE, dp->semaname);

   if( dp->sema == INVALID_HANDLE_VALUE ) {
   
	   PrintLastErrorText("CreateDataport(): CreateMutex() failed");
	   CloseHandle(dp->mem);
	   free(dp);
	   return NULL;
   
   }

   /* ------------------- Map the memomy ------------------- */

   dp->buff = MapViewOfFile(dp->mem,FILE_MAP_WRITE,0,0,0);

   if( dp->buff==NULL ) {

	   PrintLastErrorText("CreateDataport(): MapViewOfFile() failed");
	   CloseHandle(dp->mem);
	   CloseHandle(dp->sema);
	   free(dp);
	   return NULL;

   }

   return (dp);
}

// --------------------------------------------------------------------------
// OpenDataport: Connect to an existing block
// --------------------------------------------------------------------------

Dataport *OpenDataport(char *name,size_t size)
{
   Dataport *dp;

   /* ------------------- Allocate new Dataport Handle ------------------- */

   dp = (Dataport *)malloc(sizeof(Dataport));

   if( dp == NULL ) {
   
 	   printf("OpenDataport(): Not enough memory...");
     free(dp);
	   return NULL;
  
   }

   strcpy(dp->name,name);
   sprintf(dp->semaname,"%s_sema",name);

   /* ------------------- Link to the share memory ------------------- */

   dp->mem = CreateFileMapping(
	            INVALID_HANDLE_VALUE,   // to create a memory file
		       		NULL,                   // no security 
			      	PAGE_READWRITE,         // to allow read & write access
		      		0,     
		      		(DWORD)size,                   // file size     
			      	name);                  // object name      
   
   if( GetLastError()!=ERROR_ALREADY_EXISTS ) {

	   printf("OpenDataport(): dataport %s doesn't exist.\n",dp->name);
	   if( dp->mem != INVALID_HANDLE_VALUE )
	     CloseHandle(dp->mem);
     free(dp);
	   return NULL;

   }

   /* ------------------- Link to the semaphore ------------------- */

   SetLastError (ERROR_SUCCESS);
   dp->sema = CreateMutex (NULL, FALSE, dp->semaname);

   if( GetLastError()!=ERROR_ALREADY_EXISTS ) {

	   printf("OpenDataport(): dataport semaphore %s doesn't exist.\n",dp->semaname);
	   if( dp->sema != INVALID_HANDLE_VALUE )
	     CloseHandle(dp->sema);
     free(dp);
	   return NULL;

   }

   /* ------------------- Map the memomy ------------------- */

   dp->buff = MapViewOfFile(dp->mem,FILE_MAP_WRITE,0,0,0);

   if( dp->buff==NULL ) {

	   PrintLastErrorText("OpenDataport(): MapViewOfFile() failed");
	   free(dp);
	   return NULL;

   }

   return (dp);
}

// --------------------------------------------------------------------------

BOOL AccessDataport(Dataport *dp)
{
  // 1sec timeout 
  if (WaitForSingleObject(dp->sema,8000)==WAIT_OBJECT_0)
	return TRUE;
  else
	return FALSE;
}

// --------------------------------------------------------------------------

BOOL AccessDataportTimed(Dataport *dp,DWORD timeout)
{
  // 1ms timeout
  if (WaitForSingleObject(dp->sema,timeout)==WAIT_OBJECT_0)
	return TRUE;
  else
	return FALSE;
}

// --------------------------------------------------------------------------

BOOL ReleaseDataport(Dataport *dp)
{
  if( ReleaseMutex(dp->sema)==0 )
	  return TRUE;
  else
	  return FALSE;
}

// --------------------------------------------------------------------------

BOOL CloseDataport(Dataport *dp)
{

	UnmapViewOfFile(dp->buff);
	CloseHandle(dp->mem);
	CloseHandle(dp->sema);
	free(dp);
	return TRUE;

}

// --------------------------------------------------------------------------

void PrintLastErrorText( LPTSTR suff )
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           NULL,
                           GetLastError(),
                           LANG_NEUTRAL,
                           (LPTSTR)&lpszTemp,
                           0,
                           NULL );

    printf( TEXT("%s:%s (0x%x)"), suff,lpszTemp,GetLastError() );

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

}

#else

// -------------------------------------------------------------------------------------------------
// Linux code
// -------------------------------------------------------------------------------------------------

#include <sys/types.h>

typedef struct {
     int              sema;
     int              shar;
     int              key;
     pid_t            creator_pid;
     char             body;
} Dataport;

#include <stdio.h>
#include <dataport.h>
#include <boolean.h>
#include <shared.h>
#include <sema.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>
#include <strings.h>
#include <unistd.h>


/****************************************************************************
*
*		Code for build_key function 
*               --------------------------- 
* 
*    Function rule : To build a unique key from the dataport name
*
*    Argin : - The dataport name
* 
*    Argout : No argout	
* 
*    This function returns the key or -1 if it fails
*
****************************************************************************/

int build_key (name)
char *name;
{
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
		key = HASH_BASE * key + *name;
	}

  	return (key % HASH_MOD);
}

/****************************************************************************
*
*		Code for CreateDataport function
*               --------------------------------
*
*    Function rule : To create a complete dataport (shared memory and its 
*							associated semaphore)
*
*    Argin : - The dataport name
*            - The shared memory size wanted by the user
*
*    Argout : No argout
*
*    This function returns the datport pointer or NULL if it fails
*
****************************************************************************/

Dataport *CreateDataport(name,size)
char *name;
long size;
{
   int key;
   Dataport *dp;
   int shmid;
	int semid;
	int full_size;

/* Build a key from the dataport name */

   key = build_key(name);
   if (key == -1)
		return(NULL);

/* Get shared memory and semaphore */

	full_size = size + sizeof(Dataport);
   dp = (Dataport *)get_shared(full_size,key,TRUE,&shmid);
   if (dp==(Dataport *)(-1))
   	return (NULL);
   dp->shar = shmid;

/* Get semaphore to protect the shared memory */

   semid = define_sema(1,key,TRUE);
	if (semid == -1)
		return(NULL);
	dp->sema = semid;

/* Init. dataport own parameters and leave function */

   dp->creator_pid = getpid();
   dp->key = key;
   return (dp);
}

/****************************************************************************
*
*		Code for OpenDataport function
*               ------------------------------
*
*    Function rule : To link a process to an existing dataport
*
*    Argin : - The dataport name
*            - The dataport shared memory size
*
*    Argout : No argout
*
*    This function returns a pointer to the dataport shm or NULL if it
*	  fails
*
****************************************************************************/

Dataport *OpenDataport(name,size)
char *name;
long size;
{
   int key;
   Dataport *dp;
   int shmid;
	int semid;

/* Build the key from dataport name */

   key = build_key(name);
   if (key == -1)
   	return (NULL); /* Name not in lookup table */

/* Link the process to the dataport shared memory */

   dp = (Dataport *)get_shared (size, key, FALSE, &shmid);
   if (dp==(Dataport *)(-1))
    	return (NULL); 
   dp->shar = shmid;

/* Link the process to the dataport semaphore */

   semid = define_sema (1, key, FALSE);
	if (semid == -1)
   {
		release_shared(dp);
		return(NULL);
   }
   dp->sema = semid;
   return (dp);
}

/****************************************************************************
*
*		Code for AccessDataport function
*               --------------------------------
*
*    Function rule : To take a dataport control
*
*    Argin : - Address of dataport shared memory
*
*    Argout : No argout
*
*    This function returns 0 when the process gets the dataport control or
*	  -1 when it fails
*
****************************************************************************/

long AccessDataport(dp)
Dataport *dp;
{
   return(get_sema (dp->sema));
}

/****************************************************************************
*
*		Code for AccessDataportNoWait function
*               --------------------------------------
*
*    Function rule : To take a dataport control
*
*    Argin : - Address of dataport shared memory
*
*    Argout : No argout
*
*    This function returns 0 when the process gets the dataport control or
*	  immediately -1 when it fails
*
****************************************************************************/

long AccessDataportNoWait(dp)
Dataport *dp;
{
   return(get_sema_nowait (dp->sema));
}

/****************************************************************************
*
*		Code for ReleaseDataport function
*               ---------------------------------
*
*    Function rule : To release a dataport control
*
*    Argin : - Address of dataport shared memory
*
*    Argout : No argout
*
*    This function returns 0 if the dataport is released. Otherwise, the
*	  function returns -1.
*
****************************************************************************/

long ReleaseDataport(dp)
Dataport *dp;
{
   return(release_sema (dp->sema));
}

/****************************************************************************
*
*		Code for CloseDataport function
*               -------------------------------
*
*    Function rule : To return the free memory in a block and to return
*		     				largest free area.
*
*    Argin : - Address of the allocation table
*            - The buffer size
*
*    Argout : - The largest free area size
*	      	  - The amount of free memory
*
*    This function returns -1 if one error occurs and the error code will
*    be set
*
****************************************************************************/

long CloseDataport(dp,name)
Dataport *dp;
char *name;
{
   int key,shar;

/* The caller is the dataport creator ? */

/* JMC suppress that test on september 8 94 *******
*   if (dp->creator_pid != getpid())
*   {
*    	fprintf(stderr,"This process did not create the dataport\n");
*    	return (-1); 
*   }
*/
/* Build the key from dataport name */

   key = build_key(name);
   if (key==-1)
   {
    	fprintf(stderr,"The key for this name could not be found\n");
    	return (-1); /* Name not in lookup table */
   }

/* Is it the correct key ? */

   if (dp->key != key)
   {
    	fprintf(stderr,"This dataport does not correspond to the name\n");
    	return (-1); /* Name and dp to not correspond */
   }

/* Delete dataport semaphore */

   delete_sema (dp->sema);

/* Release and delte the shared memory */

   shar = dp->shar;
   release_shared((char *)dp);
   delete_shared (shar);

   return (0);
}

#endif

