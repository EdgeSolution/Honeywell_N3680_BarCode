
#ifndef SYNCOBJECTS_H
#define SYNCOBJECTS_H

#include "winerror.h"
#include <assert.h>
#include<unistd.h>

// #ifndef __USE_UNIX98
// #define __USE_UNIX98 	                  // make pthread_mutexattr_settype visible
// #endif
#include <pthread.h>

//#define INFINITE portMAX_DELAY
#define WAIT_OBJECT_0 0

#define CRITICAL_SECTION pthread_mutex_t

#define HANDLE void*
#define BOOL bool
#define TRUE true
#define FALSE false
#define DWORD unsigned int

inline void InitializeCriticalSection(CRITICAL_SECTION *pMutex)
{
	assert(pMutex!=NULL);
    pthread_mutex_init(pMutex, 0);
}

inline void DeleteCriticalSection(CRITICAL_SECTION *pMutex)
{
    assert(pMutex!=NULL);
    pthread_mutex_destroy(pMutex);
}

inline void EnterCriticalSection(CRITICAL_SECTION *pMutex)
{
	assert(pMutex!=NULL);
	pthread_mutex_lock(pMutex);
}

inline bool TryEnterCriticalSection(CRITICAL_SECTION *pMutex)
{
	assert(pMutex!=NULL);
	return pthread_mutex_trylock(pMutex) ? false : true;
}

inline void LeaveCriticalSection(CRITICAL_SECTION *pMutex)
{
	assert(pMutex!=NULL);
	pthread_mutex_unlock(pMutex);
}

/////////////////////////////////////////////////////////////////////////////////////////
HANDLE CreateMutex(void* lpMutexAttributes, BOOL bInitialOwner, const char *lpName);
HANDLE CreateEvent(void* pDummy, BOOL bManualReset, BOOL bInitialState, void* lpName);

int WaitForSingleObject(HANDLE handle, int timeout);

/*void Sleep(int timems){
	usleep(timems*1000);
	return;
}*/

void CloseHandle(HANDLE dummy);
BOOL ReleaseMutex(HANDLE hMutex);

BOOL ResetEvent(HANDLE hEvent);
#define SetEvent ReleaseMutex

#endif
