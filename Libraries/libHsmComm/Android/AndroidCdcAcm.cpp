// ============================================================================
// $Id: AndroidCdcAcm.cpp 158 2018-12-06 17:58:56Z e411776 $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Connection class for CDC-ACM aka ComPort emulation.
	\file */
//=============================================================================

#include "stdafx.h"
#include "AndroidCdcAcm.h"
#include <linux/usbdevice_fs.h>
#include <sys/ioctl.h>

CUsbCdcAcm::~CUsbCdcAcm(){}

void CUsbCdcAcm::PrepareDisconnect()
{
}

bool CUsbCdcAcm::IsUSBConnected()
{
	return false;
}

/*
void CUsbCdcAcm::PurgeReceive()
{
	
}
 */

void CUsbCdcAcm::InitRxTx()
{
	
}

// Returns size of data buffer
size_t CUsbCdcAcm::read_internal(UCHAR *buffer, size_t BufferSize)
{
    struct usbdevfs_bulktransfer bt;
    bt.ep = m_InEp;		// endpoint (received from Java)
    bt.len = BufferSize;			// length of data
    bt.timeout = read_timeout;		// timeout in ms
    bt.data = (void*)buffer;		// the data

    int ReturnVal = ioctl(m_fd, USBDEVFS_BULK, &bt);
    // Caution, need to check if return values are compatible
    if(ReturnVal<0)
		ReturnVal=0;
    return ReturnVal;
}

size_t CUsbCdcAcm::write_internal(const UCHAR *pByte, size_t Size)
{
	struct usbdevfs_bulktransfer bt;
	bt.ep = m_OutEp;		// endpoint (received from Java)
	bt.len = Size;			// length of data
	bt.timeout = write_timeout;		// timeout in ms
	bt.data = (void*)pByte;		// the data

	int RetVal = ioctl(m_fd, USBDEVFS_BULK, &bt);
	// fixme: check how to reliable get amount of bytes written
	return Size;
}














// Original write methods
/*
void CUsbCdcAcm::Write(UCHAR byte)
{
	// return Write(&byte, 1);
	Write(&byte, 1);
}

void CUsbCdcAcm::Write(const UCHAR *pByte, size_t Size)
{
	struct usbdevfs_bulktransfer bt;
	bt.ep = m_OutEp;		// endpoint (received from Java)
	bt.len = Size;			// length of data
	bt.timeout = write_timeout;		// timeout in ms
	bt.data = (void*)pByte;		// the data

	int RetVal = ioctl(m_fd, USBDEVFS_BULK, &bt);
	// Caution, need to check if return values are compatible
	// return RetVal;
}
 */