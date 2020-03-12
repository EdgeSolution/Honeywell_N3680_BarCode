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


typedef unsigned long timer_uSec_t;

///////////////////////////////////////////////////////////////////////////////
//! A simple to use timeout class.
/*!  A simple adaption of the class from matrix.
*/
class	CTimeoutuS
{
public:
	///////////////////////////////////////////////////////////////////////////////
	//! Default Constructor that sets all times to 0.
	/*! 
	*/
	CTimeoutuS()
	{
		m_Startus = 0;
		m_Durationus = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Constructor that also sets the expiration time.
	/*! 
	 \param usec Timeout in milliseconds (1ms .. 6000 seconds is tested)
	*/
	CTimeoutuS( timer_uSec_t usec )
	{
		SetExpiration( usec );
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Restart the timer.
	/*! 
	*/
	void Reset(void)
	{
		m_Startus = GetTickCount();
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Set the expiration time.
	/*! 
	 \param usec Timeout in milliseconds (1ms .. 6000 seconds is tested)
	*/
	void SetExpiration( timer_uSec_t usec )
	{
		m_Startus = GetTickCount();
		m_Durationus = usec;
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Check whether the timer has expired.
	/*!
	 \return true if expired
	*/
	bool HasExpired()
	{
		int nDiff = GetTickCount() - m_Startus;
		return (((unsigned long)nDiff) >= m_Durationus);  // overflow is automatically handled by binary complement. See below.
	}

	timer_uSec_t GetExpiration(void)
	{
		return m_Durationus;
	}

	// Can be used to see how long we still have to wait. Returns 0 if time expired.
	unsigned long GetRestTime(void)
	{
		long temp = (long)m_Startus+(long)m_Durationus-(long)GetTickCount();

		return ( temp < 0 ? 0 : (unsigned long)temp );
	}

	timer_uSec_t GetElapsed(void)
	{
		return GetTickCount() - m_Startus;
	}
protected:
	static unsigned long GetTickCount(void)
	{
 		struct timespec ts;
 		clock_gettime(CLOCK_MONOTONIC, &ts);
 		return ts.tv_sec * 1000 *1000 + ts.tv_nsec / 1000;
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

	unsigned long	m_Startus;
	unsigned long	m_Durationus;
};

