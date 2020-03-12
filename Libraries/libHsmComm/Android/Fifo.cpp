//! \file
///////////////////////////////////////////////////////////////////////////////
// Taken from dfUsbLib
// With friendly permission from the original author Dieter Fauth.
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "syncobjects.h"
#include "Fifo.h"


CFifo::CFifo(int size)
{
	assert(size < INT_MAX);
	m_size = size;
	m_pBuffer = new unsigned char [size+1];
	InitializeCriticalSection(&m_lock);
	m_hDoneEvent		 = CreateEvent(NULL, TRUE, FALSE, NULL);
	assert(m_hDoneEvent != NULL);
	Purge();
	m_timeout=1500;
}

CFifo::~CFifo()
{
	delete [] m_pBuffer;
	DeleteCriticalSection(&m_lock);
	CloseHandle(m_hDoneEvent);
}

void CFifo::Purge(void)
{
	m_avail= 0;				/* prevent writes */
	m_tail = 0;
	m_head = 0;
	m_avail= m_size;		/* allow writes*/
	m_overflow = FALSE;
	m_expected=INT_MAX;
}


inline void CFifo::BumpPointer(volatile int &index, int step)
{
	int temp = index;
	temp += step;
	if (temp >= m_size)
	{
		temp = 0;
	}
	index = temp;
}

inline int CFifo::CheckForDone(int step)
{
	assert(m_hDoneEvent != NULL);
	::EnterCriticalSection(&m_lock);
	m_avail -= step;
	if(AnyData() >= m_expected)
		SetEvent(m_hDoneEvent);
	::LeaveCriticalSection(&m_lock);
	return step;
}

size_t CFifo::Write(char data)
{
	size_t retval = 0;
	if (m_avail > 0)
	{
		m_pBuffer[m_head] = data;
		BumpPointer(m_head);
		retval = CheckForDone(1);
	}
	else
	{
		m_overflow = TRUE;
	}
	return retval;
}

size_t CFifo::Write(const char *pData, size_t size)
{
	size_t retval = 0;
	if (m_avail >= size)
	{
		retval = size;
		int room=m_size-m_head;
		if(room < size)
		{
			memcpy(&m_pBuffer[m_head], pData, room);
			BumpPointer(m_head, room);
			pData += room;
			size = size-room;
		}
		memcpy(&m_pBuffer[m_head], pData, size);
		BumpPointer(m_head, size);
		CheckForDone(retval);
	}
	else
	{
		m_overflow = TRUE;
	}
	return retval;
}

short CFifo::Read()
{
	short retval = -1;
	if (m_avail < m_size)
	{
		retval = m_pBuffer[m_tail];
		BumpPointer(m_tail);
		::EnterCriticalSection(&m_lock);
		m_avail += 1;
		::LeaveCriticalSection(&m_lock);
	}
	return retval;
}

size_t CFifo::Read(char *pDestination, size_t size)
{
	int retval = 0;
	int contents = m_size - m_avail;
	if (contents>0)
	{
		if(size>contents)	// only return what we've got so far
			size=contents;

		retval=size;

		int rest=m_size-m_tail;
		if(rest < size)
		{
			memcpy(pDestination, &m_pBuffer[m_tail], rest);
			BumpPointer(m_tail, rest);
			pDestination += rest;
			size = size-rest;
		}
		memcpy(pDestination, &m_pBuffer[m_tail], size);
		BumpPointer(m_tail, size);
		::EnterCriticalSection(&m_lock);
		m_avail += retval;
		::LeaveCriticalSection(&m_lock);
	}
	return retval;
}

short CFifo::ReadWithWait()
{
	assert(m_hDoneEvent != NULL);
	if (AnyData() < 1)
		WaitTimeout(1);
	return Read();
}

size_t CFifo::ReadWithWait (char *pDestination, size_t size)
{
	assert(m_hDoneEvent != NULL);
	if (AnyData() < size)
		WaitTimeout(size);
	return Read(pDestination, size);
}

bool CFifo::IsTimeout0(void)
{
    return m_timeout==0;
}

void CFifo::SetTimeout(int NumRxBytes)
{
    // we use a fixed timeout here, so only the expected num bytes needs to be stored
	::EnterCriticalSection(&m_lock);
	m_expected = NumRxBytes;
	::LeaveCriticalSection(&m_lock);
}

void CFifo::WaitTimeout(int NumRxBytes)
{
	if (!IsTimeout0())
	{
		assert(m_hDoneEvent != NULL);
        ResetEvent(m_hDoneEvent);
        SetTimeout(NumRxBytes);
		WaitForSingleObject(m_hDoneEvent, m_timeout);
		m_expected=INT_MAX;
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Abort any pending reads
/*!
*/
void CFifo::AbortPendingReads(void)
{
	assert(this != NULL);
	assert(m_hDoneEvent != NULL);
	SetEvent(m_hDoneEvent);
}
