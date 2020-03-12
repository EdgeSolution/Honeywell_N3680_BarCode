/*=================================================================================
  TransmitFirmware.h
//=================================================================================
   $Id: TransmitFirmware.h 203 2019-01-31 16:41:15Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// TransmitFirmware.h : header file
//

#pragma once

#include "DeviceFirmware.h"
#include "AndroidUsbManager.h"

namespace HsmDeviceComm {

//class CUsbManager;
//class CDeviceTracer;

///////////////////////////////////////////////////////////////////////////////
//! Connects the device communication to a low level communication class.
/*!
	Here we use the dfusblib as we do in the other Adaptus ESC.
	We overwrite virtual functions declared in CXModem and call into the dfusblib.
	This is a place you most likely need to change if you port to another plattform.
*/
#define CTxFirmwareBase CDeviceFirmware
class CTxFirmware : public CTxFirmwareBase
{
//	friend CDeviceTracer;
public:
	CTxFirmware(HWND hNotifictionReceiver = NULL, DWORD MsgStatus = 0);
	virtual ~CTxFirmware();

	using CTxFirmwareBase::Connect;
	using CTxFirmwareBase::Disconnect;

	virtual bool Connect(const char* szDevice, int baudrate=115200);
	virtual bool Connect(const wchar_t* szDevice, int baudrate=115200);
	//virtual bool Connect(void);
	virtual bool DisConnect(void);
	bool OnOptions(void);
	bool OnPorts(void);
	void ReadIni(const TCHAR *szSec);
	CString GetTrueDisplayName(void);
	void SetDisplayName(CString AnyName);
	bool AutoSelect(CString &sFound);
	void SetupBootModeParameters();

	virtual bool CheckForNewDevice(bool bForce = false);

protected:
	virtual void InitRxTx(void);

protected:
	// CUsbManager *m_pComm;							//!< The low level comm port access
//	CDeviceTracer *m_pTracer;
};

}	// namespace HsmDeviceComm
