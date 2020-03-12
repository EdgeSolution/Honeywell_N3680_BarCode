// ============================================================================
// $Id: AndroidUsbBase.h 158 2018-12-06 17:58:56Z e411776 $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Base class for USB connections.
	Details.
	\file */
//=============================================================================
#pragma once
#include <queue>
#include <semaphore.h>
#include <pthread.h>
#include "ThreadWorker.h"
#include "PortFunctor.h"

class CFifo;

enum Interfacetype_t
{
	Unknown=0,
	CdcAcm, HidPos,
};

struct UsbDetails_t
{
	Interfacetype_t m_type;
	int	m_fd;	// file descriptor
	unsigned int m_OutEp;
	unsigned int m_InEp;
	unsigned int m_OutEpSize;
	unsigned int m_InEpSize;
};

class CUsbBase
{
public:
	CUsbBase(UsbDetails_t &device);
    CUsbBase();
    virtual ~CUsbBase();

	virtual bool isValid();
// protected:
	virtual void Connect();
	virtual void Disconnect();

	virtual void PrepareDisconnect()=0;
	virtual bool IsUSBConnected()=0;
	virtual void PurgeReceive();
	virtual void InitRxTx()=0;

	virtual int Read();
	virtual size_t Read(UCHAR *buffer, size_t size);
	virtual size_t ReadBlock(UCHAR *buffer, size_t size);	// non waiting flavour

	virtual bool Write(UCHAR byte);
	virtual bool Write(const UCHAR *pByte, size_t Size);

	bool SetReadCallback(CallbackRead_t *pFunctorRead);

protected:
    virtual size_t read_internal(UCHAR *buffer, size_t size)=0;
    virtual size_t write_internal(const UCHAR *pByte, size_t Size)=0;
	void* ReadThreadLoop();

    int setFeature(unsigned char reportId, unsigned char *buffer, int size);
    uint16_t cpu_to_le16(const uint16_t in)
    {
        union
        {
            uint8_t  bits8[2];
            uint16_t bits16;
        }u;

        u.bits8[1] = (uint8_t) (in >> 8);
        u.bits8[0] = (uint8_t) (in & 0xff);

        return u.bits16;
    }

protected:
	int	m_fd;
	unsigned int m_OutEp;
	unsigned int m_InEp;
	unsigned int m_OutEpSize;
	unsigned int m_InEpSize;
	class CFifo *m_pFifo;
    enum ThreadCommands_t
    {
        None, Stop, Start,
    };
    TThreadWorker<CUsbBase, ThreadCommands_t, None> *m_pReadThread;
	int read_timeout;
	int write_timeout;

	CallbackRead_t *m_ReadDispatch;					//!< Callback for data arrived
private:
	// No copy and assignment allowed.
	CUsbBase(const CUsbBase &);
	void operator =(const CUsbBase &);

};