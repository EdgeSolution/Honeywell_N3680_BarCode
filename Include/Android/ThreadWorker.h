// ============================================================================
// $Id: ThreadWorker.h 158 2018-12-06 17:58:56Z e411776 $
// Copyright Honeywell International Inc. 2015
// ============================================================================
/** Worker threads.
	Abstracts usage of the thread API.
	\file */
//=============================================================================
#pragma once

#include <semaphore.h>
#include <pthread.h>
#include <queue>
#include "Functor.h"

typedef TFunctor0<void*>	ThreadCode_t;

// modeled after a thread class in dfUsbLib

#ifdef ANDROID
	#define __sched_priority sched_priority
#endif


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
	: m_tid(0)
	, m_Tread(pObject, fpt)
	, m_running(true)
	{
		sem_init(&m_semAction,0,0);
		sem_init(&m_semStop,0,0);
		pthread_mutex_init(&m_mutRun, NULL);
		pthread_mutex_init(&m_mutPause, NULL);
		pthread_mutex_init(&m_mutCmd, NULL);
		Pause();
	}

	///////////////////////////////////////////////////////////////////////////////
	//! dtor.
	/*!
	*/
	~TThreadWorker(void)
	{
		WaitForStop();
		sem_destroy(&m_semAction);
		sem_destroy(&m_semStop);
		pthread_mutex_destroy(&m_mutRun);
		pthread_mutex_destroy(&m_mutPause);
		pthread_mutex_destroy(&m_mutCmd);
	}

//#define USE_HIGH_PRIO 1
	///////////////////////////////////////////////////////////////////////////////
	//! Start worker thread.
	/*!
	 \return 0 on success
	*/
	int Start()
	{
		int ret = pthread_create(&m_tid, NULL, &TThreadWorker::KickStart, (void*)&m_Tread);

#ifdef USE_HIGH_PRIO
		// change the priority...
		int policy;
		struct sched_param param;

		// get the current priority
		pthread_getschedparam(m_tid, &policy, &param);

		// show the current priority
		DBG_OUT1("current: m_tid=0x%x,policy=%s,param.__shed_priority=%d\n", m_tid, (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                   (policy == SCHED_RR)    ? "SCHED_RR" :
                   (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                   "???", param.__sched_priority);

		// boost it a tad
		policy = SCHED_RR;
		param.__sched_priority = 20;
		//pthread_setschedparam(m_tid, policy, &param);

		// show the current priority (again to see if it has changed)
		DBG_OUT1("modified: m_tid=0x%x,policy=%s,param.__shed_priority=%d\n", m_tid, (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                   (policy == SCHED_RR)    ? "SCHED_RR" :
                   (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                   "???", param.__sched_priority);
#endif

		return ret;
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Stop worker thread.
	/*! Can be called from any place.
	*/
	void Stop()
	{
		sem_post(&m_semStop);

		sem_post(&m_semAction);
		pthread_mutex_unlock(&m_mutRun);
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Stop worker thread and wait until it has finished.
	/*! Never call from worker thread.
	*/
	void WaitForStop()
	{
		Stop();
		pthread_join(m_tid, NULL);
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Start the action in worker thread.
	/*! Can be called from any place.
	*/
	void Action()
	{
		sem_post(&m_semAction);
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Wait for semaphore that starts the single action.
	/*! Call this from inside the worker thread.
		It also checks for the termination call and exits the thread.
	*/
	int WaitForAction()
	{
		sem_wait(&m_semAction);
		if(sem_trywait(&m_semStop)==0)
		{
			pthread_exit(NULL);
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Issue a new command.
	/*!
	*/
	void QueueCommand(TCmd cmd)
	{
		pthread_mutex_lock(&m_mutCmd);
		m_CmdQueue.push(cmd);
//		LOGE("IssueCommand %i - total %i", cmd, m_CmdQueue.size());
		pthread_mutex_unlock(&m_mutCmd);

		Action();
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Wait for a new command.
	/*!
	*/
	TCmd WaitForCommand()
	{
		TCmd cmd = NoCmd;
		WaitForAction();
		pthread_mutex_lock(&m_mutCmd);
		if(!m_CmdQueue.empty())
		{
			cmd = m_CmdQueue.front();
			m_CmdQueue.pop();
		}
		pthread_mutex_unlock(&m_mutCmd);
		return cmd;
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Wait for a new command.
	/*!
	*/
	TCmd PeekCommand()
	{
		TCmd cmd = NoCmd;
		pthread_mutex_lock(&m_mutCmd);
		if(!m_CmdQueue.empty())
		{
			cmd = m_CmdQueue.front();
		}
		pthread_mutex_unlock(&m_mutCmd);
		return cmd;
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Run the worker thread.
	/*!
	 * m_running prevents a lookup if Pause is called twice
	 * Run, Pause and PauseWait must always be called form the same thread!
	 * They control the behavior of the of our thread.
	*/
	void Run()
	{
		m_running=true;
		pthread_mutex_unlock(&m_mutRun);
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Pause the worker thread.
	/*! Wait until the thread is paused
	 * m_running prevents a lookup if Pause is called twice
	 * Run, Pause and PauseWait must always be called form the same thread!
	 * They control the behavior of the of our thread.
	*/
	void PauseWait()
	{
		if(m_running)
		{
			m_running=false;
			pthread_mutex_lock(&m_mutRun);
			pthread_mutex_lock(&m_mutPause);
			pthread_mutex_unlock(&m_mutPause);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Pause the worker thread.
	/*! Do not wait until the thread is paused.
	 * m_running prevents a lookup if Pause is called twice
	 * Run, Pause and PauseWait must always be called form the same thread!
	 * They control the behavior of the of our thread.
	*/
	void Pause()
	{
		if(m_running)
		{
			m_running=false;
			pthread_mutex_lock(&m_mutRun);
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	//! Wait for condition variable that tells to keep running.
	/*! Call this from inside the worker thread.
		It also checks for the termination call and exits the thread.
	*/
	bool ShallWeRun()
	{
		pthread_mutex_unlock(&m_mutPause);
		pthread_mutex_lock(&m_mutRun);
		pthread_mutex_lock(&m_mutPause);
		pthread_mutex_unlock(&m_mutRun);
		if(sem_trywait(&m_semStop)==0)
		{
			pthread_exit(NULL);
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Check for condition variable that tells to keep running.
	/*! Call this from inside the worker thread.
 	 \return true for continue, false for not continue

	*/
	bool TryContinue()
	{
		bool ret=false;
		if(pthread_mutex_trylock(&m_mutRun)==0)
		{
			pthread_mutex_unlock(&m_mutRun);
			ret = true;
		}
		return ret;
	}

protected:
	///////////////////////////////////////////////////////////////////////////////
	//! Static helper to start a thread.
	/*!
	 \param pVoid
	*/
	static void* KickStart(void *pVoid)
	{
		ASSERT(pVoid!=NULL);
		ThreadCode_t *pFunctor = (ThreadCode_t*) pVoid;
		return pFunctor->Call();
	}

protected:
	pthread_t m_tid;							//<! thread id
	TSpecificFunctor0<TClass, void*> m_Tread;	//!< functor for thread start
	sem_t m_semAction;						 	//!< semaphor for starting the action
	sem_t m_semStop;						 	//!< semaphor to flag the termination
	pthread_mutex_t m_mutRun;
	pthread_mutex_t m_mutPause;
//	pthread_cond_t m_condRun;
	volatile bool m_running;
	pthread_mutex_t m_mutCmd;		//!< protect thread commands
	std::queue<TCmd> m_CmdQueue;	//!< commands for the thread
};
