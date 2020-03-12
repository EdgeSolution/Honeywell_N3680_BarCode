//! \file
///////////////////////////////////////////////////////////////////////////////
// Taken from dfUsbLib
// With frienly permission from the original author Dieter Fauth.
///////////////////////////////////////////////////////////////////////////////

#ifndef CPP_FIFO_H_CDD7933B_4209_4291_B69D_1788BB71CCDC
#define CPP_FIFO_H_CDD7933B_4209_4291_B69D_1788BB71CCDC
#include "syncobjects.h"

/** A fifo for bytes
 */
class CFifo
{
public:
	CFifo(int size);
	~CFifo();

	int IsAvail(void) 			{ return m_avail;	}

	bool IsEmpty(void)
	{
		return (m_avail==m_size);
	}
	int AnyData(void)
	{
		return (m_size-m_avail);
	}

    size_t	Write(char data);
    size_t	Write(UCHAR data)
	{
		return 	Write((char)data);
	}
    size_t Write(const char *pData, size_t size);
    size_t Write(const UCHAR *pData, size_t size)
	{
		return 	Write((const char *)pData, size);
	}

	short Read();
    size_t Read(char *pDestination, size_t size);
    size_t Read(UCHAR *pDestination, size_t size)
	{
		return 	Read((char*)pDestination, size);
	}

	void Purge();

	short ReadWithWait();
    size_t ReadWithWait(char *pDestination, size_t size);
    size_t ReadWithWait(UCHAR *pDestination, size_t size)
	{
		return 	ReadWithWait((char*)pDestination, size);
	}

	void AbortPendingReads(void);

protected:
	inline void BumpPointer(volatile int &index, int step=1);
	inline int CheckForDone(int step);
	bool	IsTimeout0(void);
	void	SetTimeout(int NumRxBytes);
	void	WaitTimeout(int NumRxBytes);
	DWORD m_timeout;

protected:
	int		 m_size;
	volatile int m_head;
	volatile int m_tail;
	volatile int m_avail;
	unsigned char *m_pBuffer;
	bool m_overflow;
	volatile int m_expected;
	HANDLE m_hDoneEvent;

	CRITICAL_SECTION m_lock;
};



#endif /* CPP_FIFO_H_CDD7933B_4209_4291_B69D_1788BB71CCDC */

/*=============================================================================
 * $Log: fifo.h $
 *============================================================================*/
