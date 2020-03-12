// ============================================================================
// $Id: AndroidHidPos.h 158 2018-12-06 17:58:56Z e411776 $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Connection class for HidPos.
	Details.
	\file */
//=============================================================================
#pragma once
#include "AndroidUsbBase.h"

class CUsbHidPos : public CUsbBase
{
public:
	CUsbHidPos(UsbDetails_t &device)
	: CUsbBase(device) 	{}
	
	virtual ~CUsbHidPos();
    virtual void Connect();
protected:
	virtual void PrepareDisconnect(void);
	virtual bool IsUSBConnected(void);
	virtual void InitRxTx(void);

protected:
    virtual size_t read_internal(UCHAR *buffer, size_t size);
    virtual size_t write_internal(const UCHAR *pByte, size_t Size);
	void HidPosRedirectOutput();

private:
	// No copy and assignment allowed.
	CUsbHidPos(const CUsbHidPos &);
	void operator =(const CUsbHidPos &);
};

