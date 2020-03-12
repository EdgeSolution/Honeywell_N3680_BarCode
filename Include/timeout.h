// ============================================================================
// $Id: timeout.h 158 2018-12-06 17:58:56Z e411776 $
// Copyright Honeywell International Inc. 2015
// ============================================================================
/** Timeout class.
	Makes it much easier to use timeouts.
	\file */
//=============================================================================

#pragma once

typedef unsigned long timer_mSec_t;

///////////////////////////////////////////////////////////////////////////////
//! A simple to use timeout class.
/*!  A simple adaption of the class from matrix.
*/
class	CTimeoutmS
{
public:
	///////////////////////////////////////////////////////////////////////////////
	//! Default Constructor that sets all times to 0.
	/*! 
	*/
	CTimeoutmS()
	{
		m_Startms = 0;
		m_Durationms = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Constructor that also sets the expiration time.
	/*! 
	 \param mSec Timeout in milliseconds (1ms .. 6000 seconds is tested)
	*/
	CTimeoutmS( timer_mSec_t mSec )
	{
		SetExpiration( mSec );
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Restart the timer.
	/*! 
	*/
	void Reset(void)
	{
		m_Startms = GetTickCount();
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Set the expiration time.
	/*! 
	 \param mSec Timeout in milliseconds (1ms .. 6000 seconds is tested)
	*/
	void SetExpiration( timer_mSec_t mSec )
	{
		m_Startms = GetTickCount();
		m_Durationms = mSec;
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Check whether the timer has expired.
	/*!
	 \return true if expired
	*/
	bool HasExpired()
	{
		int nDiff = GetTickCount() - m_Startms;
		return (((unsigned long)nDiff) >= m_Durationms);  // overflow is automatically handled by binary complement. See below.
	}

	timer_mSec_t GetExpiration(void)
	{
		return m_Durationms;
	}

	// Can be used to see how long we still have to wait. Returns 0 if time expired.
	unsigned long GetRestTime(void)
	{
		long temp = (long)m_Startms+(long)m_Durationms-(long)GetTickCount();

		return ( temp < 0 ? 0 : (unsigned long)temp );
	}

	timer_mSec_t GetElapsed(void)
	{
		return GetTickCount() - m_Startms;

	}
protected:
	static unsigned long GetTickCount(void)
	{
 		struct timespec ts;
 		clock_gettime(CLOCK_MONOTONIC, &ts);
 		return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	}

	// helps with testing
	static int clock_gettime(clockid_t clk_id, struct timespec *tp)
	{
#ifndef GTEST
		return ::clock_gettime(clk_id, tp);
#else
		return ::clock_gettime_mock(clk_id, tp);
#endif
	}

	unsigned long	m_Startms;
	unsigned long	m_Durationms;
};


#if 0
// this is a construction zone, but I want to keep the work done so far (at least for the enxt weeks).
// It looks like a full timeout class without restrictions is possible but 
// is more than a simple adaption of the class I wrote for the matric.
typedef unsigned long timer_uSec_t;

/*-------------------------------------------------------------------------------
   CTimeout provides a mechanism for setting a time duration and periodically
   checking the timeout object to see if the duration has expired.  </p>
*/
//-------------------------------------------------------------------------------
class	CTimeout
{
public:
	CTimeout()
		: m_StartTime(0)
		, m_DurationTime(0)
	{
	}
	CTimeout( timer_uSec_t uSec )
	{
		SetExpiration( uSec );
	}

	void	Reset(void)
	{
		m_StartTick = GetTickCount();
	}

	void	SetExpiration( timer_uSec_t uSec )
	{
 		clock_gettime(CLOCK_MONOTONIC, &m_StartTime);
		m_DurationTime
		m_TickDuration = uSecsToTicks(uSec);
	}

	timer_uSec_t GetExpiration(void)
	{
		return TicksTouSecs(m_TickDuration);
	}

	bool	HasExpired()
	{
		int nDiff = GetTickCount() - m_StartTick;
		return (((unsigned long)nDiff) >= m_TickDuration);  // overflow is automatically handled by binary complement. See below.
	}
#if 0
	// Can be used to see how long we still have to wait. Returns 0 if time expired.
	unsigned long GetRestTicks(void)
	{
		LONG temp = (LONG)m_StartTick+(LONG)m_TickDuration-(LONG)GetTickCount();

		return ( temp < 0 ? 0 : (unsigned long)temp );
	}
	timer_uSec_t GetRestTime(void)
	{
		unsigned long ticks = GetRestTicks();

		return TicksTouSecs(ticks);  /* This will handle ticks value of zero.  Negative numbers are no longer permited form GetRestTicks call */
	}
#endif
private:
	enum
	{
		Second = 1000000000L,
	};

	timespec diff(timespec start, timespec end)
	{
		timespec temp;
		if ((end.tv_nsec-start.tv_nsec)<0)
		{
			temp.tv_sec = end.tv_sec-start.tv_sec-1;
			temp.tv_nsec = Second+end.tv_nsec-start.tv_nsec;
		}
		else
		{
			temp.tv_sec = end.tv_sec-start.tv_sec;
			temp.tv_nsec = end.tv_nsec-start.tv_nsec;
		}
		return temp;
	}
	
	timespec add(timespec time1, timespec time2)
	{
		timespec temp;
		temp.tv_sec = time1.tv_sec + time2.tv_sec ;
		temp.tv_nsec = time1.tv_nsec + time2.tv_nsec ;
		if (temp.tv_nsec >= Second)	//carry?
		{
			temp.tv_sec++ ;  
			temp.tv_nsec -= Second ;
		}

		return temp;
	}

	struct timespec	m_StartTime;
	struct timespec	m_DurationTime;
};
#endif
