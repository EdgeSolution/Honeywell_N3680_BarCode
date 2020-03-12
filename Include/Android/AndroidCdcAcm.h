// ============================================================================
// $Id: AndroidCdcAcm.h 158 2018-12-06 17:58:56Z e411776 $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Connection class for CDC-ACM aka ComPort emulation.
	Details.
	\file */
//=============================================================================
#pragma once

#include "AndroidUsbBase.h"

class CUsbCdcAcm : public CUsbBase
{
public:
	CUsbCdcAcm(UsbDetails_t &device)
	: CUsbBase(device) 	{}

	virtual ~CUsbCdcAcm();

protected:
	virtual void PrepareDisconnect();
	virtual bool IsUSBConnected();
	virtual void InitRxTx();

protected:
    virtual size_t read_internal(UCHAR *buffer, size_t size);
    virtual size_t write_internal(const UCHAR *pByte, size_t Size);
private:
	// No copy and assignment allowed.
	CUsbCdcAcm(const CUsbCdcAcm &);
	void operator =(const CUsbCdcAcm &);
};
