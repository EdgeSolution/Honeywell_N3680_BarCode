// ============================================================================
// $Id: AndroidUsbBase.cpp 158 2018-12-06 17:58:56Z e411776 $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Base class for Android USB connector.
	Details.
	\file */
//=============================================================================
#include "stdafx.h"
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#include <linux/usb/ch9.h>
#include "AndroidUsbBase.h"
#include "Fifo.h"

bool CUsbBase::isValid()
{
	return (m_fd != -1)&&(m_OutEp!=0)&&(m_InEp!=0);
}

// Constructor
CUsbBase::CUsbBase(UsbDetails_t &device)
: read_timeout(500)     // used in derived classes
, write_timeout(500)    // used in derived classes
, m_ReadDispatch(NULL)
{
	m_pFifo = new CFifo(10*1024);
	assert(m_pFifo!=NULL);

	m_fd = device.m_fd;
	m_OutEp = device.m_OutEp;
	m_InEp = device.m_InEp;
	m_OutEpSize = device.m_OutEpSize;
	m_InEpSize = device.m_InEpSize;

	m_pReadThread = new TThreadWorker<CUsbBase,ThreadCommands_t, None> (this, &CUsbBase::ReadThreadLoop);
}

// Destructor
CUsbBase::~CUsbBase()
{
	delete m_pReadThread;
	m_pReadThread = NULL;
	delete m_pFifo;
	m_pFifo=NULL;
}

void CUsbBase::Connect()
{
	ASSERT(m_pReadThread!=NULL);
	m_pReadThread->Start();
    m_pReadThread->Run();
}

void CUsbBase::Disconnect()
{
	ASSERT(m_pReadThread!=NULL);
	m_pReadThread->WaitForStop();
}

bool CUsbBase::SetReadCallback(CallbackRead_t *pFunctorRead)
{
	m_ReadDispatch = pFunctorRead;
    return true;
}

int CUsbBase::Read()
{
	assert(m_pFifo!=NULL);
	int RetVal = m_pFifo->ReadWithWait();
	return RetVal;
}

size_t CUsbBase::Read(UCHAR *buffer, size_t size)
{
	assert(m_pFifo!=NULL);
    size_t RetVal = m_pFifo->ReadWithWait(buffer, size);
	return RetVal;
}

// Does not wait, just returns what is already there
size_t CUsbBase::ReadBlock(UCHAR *buffer, size_t size)
{
	assert(m_pFifo!=NULL);
	size_t RetVal = m_pFifo->Read(buffer, size);
	return RetVal;
}

bool CUsbBase::Write(UCHAR byte)
{
	return write_internal(&byte, 1);
}

bool CUsbBase::Write(const UCHAR *pByte, size_t Size)
{
	size_t written=0;
	size_t outsize =  m_OutEpSize; // here we need to use the size from endpoint
	assert(outsize!=0);
	assert(outsize<=512);
	DWORD rest;
	while(Size!=0)
	{
		rest=Size;
		if(rest> outsize)
		{
			rest = outsize;
		}
		written += write_internal(pByte, rest); // that is your write_internal
		Size-=rest;
		pByte +=rest;
	}
	return (written==Size);
}

void* CUsbBase::ReadThreadLoop()
{
    CallbackRead_t *ReadDispatch;
	while(m_pReadThread->ShallWeRun())
	{
		int SIZE = m_InEpSize;
		UCHAR buffer[SIZE + 1];    // fixme: avoid double copy
		int read_size = read_internal(buffer, SIZE);
		if (read_size > 0)
		{
            ReadDispatch = m_ReadDispatch;
            if (ReadDispatch != NULL)
            {
                ReadDispatch->Call(buffer, read_size);
            }
			else
			{
				m_pFifo->Write(buffer, read_size);
			}
		}
	}
	return NULL;
}

void CUsbBase::PurgeReceive()
{
	m_pFifo->Purge();
}


int CUsbBase::setFeature(unsigned char reportId, unsigned char *buffer, int size)
{
	const int8_t SET_FEATURE=9;
    usbdevfs_ctrltransfer ct;

    ct.bRequestType =  USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_INTERFACE;
    ct.bRequest = SET_FEATURE;
    ct.wValue = cpu_to_le16((USB_REQ_SET_FEATURE << 8) | reportId);
    ct.wIndex = cpu_to_le16(1);
    ct.wLength = cpu_to_le16(size);
    ct.timeout = write_timeout;
    ct.data = buffer;

    int RetVal = ioctl(m_fd, USBDEVFS_CONTROL, &ct);

    return RetVal;
}

