//

#include "stdafx.h"
#include <errno.h>
#include <unistd.h>
#include "syncobjects.h"
#include "timeout.h"

//#define UINT unsigned int
#if 0
///////////////////////////////////////////////////////////////////////////////////////////
extern "C" unsigned long GetTickCount()
{
 	struct timespec ts;
 	clock_gettime(CLOCK_REALTIME, &ts); // or CLOCK_PROCESS_CPUTIME_ID
 	unsigned int ret(ts.tv_sec);
 	return ret * 1000 + ts.tv_nsec / 1000000;
}

void sleep(long mSec)
{
	usleep(mSec*1000);
}
#endif

//#define SYNTRACE_STATES 1
#ifdef SYNTRACE_STATES
#define SYNTRACE TRACE
#else
#define SYNTRACE(...)
#endif

///////////////////////////////////////////////////////////////////////////////////////////

struct SyncObject
{
	pthread_mutex_t m_Mutex;
	enum eType
	{
		EventManual,
		EventAuto,
		Mutex,
	};
	eType		Type;
};

///////////////////////////////////////////////////////////////////////////////////////////
HANDLE CreateMutex(void* /* lpMutexAttributes */, BOOL bInitialOwner, const char * /* lpName */)
{
	SyncObject *pSync = new SyncObject;
    SYNTRACE("CreateMutex+ %p\n", pSync);
	assert(pSync!=NULL);
	pSync->Type = SyncObject::Mutex;

   pthread_mutexattr_t attr;
   pthread_mutexattr_init(&attr);
   pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
   pthread_mutex_init(&pSync->m_Mutex, &attr);

	if(bInitialOwner)
	{
		 pthread_mutex_lock(&pSync->m_Mutex);
	}
//	printf("m= pSync: %X, %X, %X\r\n", (UINT)pSync, (UINT) pSync->xSemaphore, (UINT) pSync->Type);
	return (HANDLE)pSync;
}

///////////////////////////////////////////////////////////////////////////////////////////
HANDLE CreateEvent(void* /* pDummy */, BOOL bManualReset, BOOL bInitialState, void* /* lpName */)
{
	SyncObject *pSync = new SyncObject;
    SYNTRACE("CreateEvent+ %p\n", pSync);
	assert(pSync!=NULL);
	if(bManualReset)
		pSync->Type = SyncObject::EventManual;
	else
		pSync->Type = SyncObject::EventAuto;

#if 0
   pthread_mutexattr_t attr;
   pthread_mutexattr_init(&attr);
   pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
   pthread_mutex_init(&pSync->m_Mutex, &attr);
#else
    pthread_mutex_init(&pSync->m_Mutex, NULL);
#endif
	if(!bInitialState)
	{
         SYNTRACE("CreateEvent lock initially %p\n", pSync);
		 pthread_mutex_lock(&pSync->m_Mutex);
	}
	return (HANDLE)pSync;
}

///////////////////////////////////////////////////////////////////////////////////////////
void CloseHandle(HANDLE handle)
{
    SYNTRACE("CloseHandle+ %p\n", handle);
	if(handle!=0)
	{
		SyncObject *pSync = (SyncObject*) handle;
		assert(pSync!=NULL);
		pthread_mutex_trylock(&pSync->m_Mutex);
		pthread_mutex_unlock (&pSync->m_Mutex);
		pthread_mutex_destroy(&pSync->m_Mutex);

		delete pSync;
	}
}

inline int WaitForMutex(SyncObject *pSync, int timeout)
{
	SYNTRACE("WaitForMutex+ %p, %i\n", pSync, timeout);
	assert(pSync!=NULL);

	int RetVal = WAIT_TIMEOUT;
	int MutexResult = EBUSY;
	if(timeout==0)  // do not wait, just check
	{
		MutexResult = pthread_mutex_trylock(&pSync->m_Mutex);
		{
			if(MutexResult==0)
			{
				RetVal = WAIT_OBJECT_0;
				if(pSync->Type == pSync->SyncObject::EventManual)
				{
					pthread_mutex_unlock(&pSync->m_Mutex);
				}
			}
		};
	}
	else
	{
		CTimeoutmS to(timeout);
		while((MutexResult = pthread_mutex_trylock(&pSync->m_Mutex)) != 0)
		{
			if (to.HasExpired())
			{
                SYNTRACE("WaitForMutex %p TO expired\n",pSync);
				break;
			}
			else
			{
				usleep(1000);
			}
		};
		if(MutexResult==0)
		{
            SYNTRACE("WaitForMutex %p locked after %lu\n", pSync, to.GetElapsed());
			RetVal = WAIT_OBJECT_0;
			if(pSync->Type == pSync->SyncObject::EventManual)
			{
				pthread_mutex_unlock(&pSync->m_Mutex);
			}
		}
	}
	return RetVal;
}

int WaitForSingleObject(HANDLE handle, int timeout)
{
	SyncObject *pSync = (SyncObject*) handle;
	assert(pSync!=NULL);

	unsigned int RetVal = WAIT_TIMEOUT;
	RetVal = WaitForMutex(pSync, timeout);
	return RetVal;
}


BOOL ReleaseMutex(HANDLE hMutex)
{
    SYNTRACE("ReleaseMutex\n");
	SyncObject *pSync = (SyncObject*) hMutex;
	pthread_mutex_unlock(&pSync->m_Mutex);
	return TRUE;
}

BOOL ResetEvent(HANDLE hEvent)
{
    SYNTRACE("ResetEvent %p\n",hEvent);
	SyncObject *pSync = (SyncObject*) hEvent;
	pthread_mutex_trylock(&pSync->m_Mutex);
	return TRUE;
}


