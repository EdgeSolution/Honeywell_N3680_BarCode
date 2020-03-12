// ============================================================================
// $Id: ThreadWorker.h 158 2018-12-06 17:58:56Z e411776 $
// Copyright Honeywell International Inc. 2015
// ============================================================================
/** Worker threads.
	Abstracts usage of the thread API.
	\file */
//=============================================================================
#pragma once

#include "Functor.h"

typedef TFunctor0<void*>	ThreadCode_t;

#define MANUAL_RESET_EVENT TRUE
#define AUTO_RESET_EVENT   FALSE
#define EVENT_INITIAL_SET	TRUE
#define EVENT_INITIAL_RESET FALSE
#define WAIT_FOR_ANY_SIGNAL FALSE

///////////////////////////////////////////////////////////////////////////////
//! Helper class to maintain worker threads.
/*!
*/
template <class TClass, typename TCmd=int, TCmd NoCmd=0> class TThreadWorker
{
public:
	///////////////////////////////////////////////////////////////////////////////
	//!  ctor,
	/*!
	 \param  pObject this pointer of calling class object
	 \param  fpt thread function in calling class objet
	*/
	TThreadWorker(TClass* pObject, void* (TClass::*fpt)())
	: m_Thread(NULL)
	, m_hThread(NULL)
	, m_TreadFunction(pObject, fpt)
	{
		m_hWatchShutdownEvent = CreateEvent(NULL, MANUAL_RESET_EVENT, EVENT_INITIAL_RESET, NULL);
		ASSERT(m_hWatchShutdownEvent != NULL);
	}

	///////////////////////////////////////////////////////////////////////////////
	//! dtor.
	/*!
	*/
	~TThreadWorker(void)
	{
		WaitForStop(),
		CloseHandle(m_hWatchShutdownEvent);
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Start worker thread.
	/*!
	 \return 0 on success
	*/
	int Start()
	{
		if (!IsWatchThreadCreated())
		{	// generate a new thread
			TRACE(_T("Create port watch thread"));
			ResetEvent(m_hWatchShutdownEvent);
			// Create a secondary thread to watch for an event.
			m_Thread = AfxBeginThread((AFX_THREADPROC)&TThreadWorker::KickStart,
				(void*)this,
				THREAD_PRIORITY_NORMAL,
				0,     // default stack size
				CREATE_SUSPENDED);
			if (!IsWatchThreadCreated())
			{
				TRACE(_T("FAILED\n"));
				return -1;
			}
			TRACE1(": 0x%X\n", m_Thread->m_nThreadID);
			//SET_THREAD_NAME(m_Thread->m_nThreadID, "ThreadWorker");
			m_Thread->m_bAutoDelete = FALSE;
			m_hThread = m_Thread->m_hThread;
		}
		return 0;	// success
	}

	//---------------------------------------------------------------------------
	bool IsWatchThreadCreated(void)
	{
		return (m_Thread != NULL);
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Stop worker thread.
	/*! Can be called from any place.
	*/
	void Stop()
	{
		if (IsWatchThreadCreated())
		{
			DWORD dwThreadID = m_Thread->m_nThreadID;	// save ID for debugging
			SetEvent(m_hWatchShutdownEvent);	// fire shutdown event
			TRACE1("Terminating watch thread: 0x%X\n", dwThreadID);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Stop worker thread and wait until it has finished.
	/*! Never call from worker thread.
	*/
	void WaitForStop()
	{
		if (IsWatchThreadCreated())
		{
			Stop();
			WaitForSingleObject(m_hThread, INFINITE);
			delete m_Thread;
			m_Thread = NULL;	// remove pointer to ensure we do not even try to talk to it
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Run the worker thread.
	/*!
	*/
	void Run()
	{
		ResetEvent(m_hWatchShutdownEvent);	// reset shutdown event
		m_Thread->ResumeThread();
	}

protected:
	///////////////////////////////////////////////////////////////////////////////
	//! Static helper to start a thread.
	/*!
	 \param pVoid
	*/
	static UINT KickStart(void *pVoid)
	{
		ASSERT(pVoid!=NULL);
		if (pVoid != NULL)
		{
			// Call the thread function
			TThreadWorker *pThis = (TThreadWorker*)pVoid;
			pThis->m_TreadFunction.Call();
		}
		return 0; // dummy
	}

protected:
	CWinThread *m_Thread;					//!< A thread for watching the port/device
	HANDLE m_hThread;						//!< we save the handle to avoid problems with the MFC mapping of handles
	HANDLE m_hWatchShutdownEvent;			//!< Used to shutdown the thread
	TSpecificFunctor0<TClass, void*> m_TreadFunction;	//!< functor for thread start
};
