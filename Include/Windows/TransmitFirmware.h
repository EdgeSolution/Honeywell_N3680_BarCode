/*=================================================================================
  TransmitFirmware.h
//=================================================================================
   $Id: TransmitFirmware.h 286 2017-10-11 09:48:01Z Fauth, Dieter $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// TransmitFirmware.h : header file
//

#pragma once

#include "DeviceFirmware.h"

class CPortManGui;									//!< The main class of dfusblib.

namespace HsmDeviceComm {

	class CDeviceTracer;

	///////////////////////////////////////////////////////////////////////////////
	//! Connects the device communication to a low level communication class.
	/*!
		Here we use the dfusblib as we do in the other HEDC places.
		We overwrite virtual functions declared in CXModem and call into the dfusblib.
		This is a place you most likely need to change if you port to another plattform.
	*/
	#define CTxFirmwareBase CDeviceFirmware
	class CTxFirmware : public CTxFirmwareBase
	{
		friend CDeviceTracer;
	public:
		CTxFirmware(HWND hNotifictionReceiver = NULL, DWORD MsgStatus = 0);
		virtual ~CTxFirmware();

		virtual bool Connect(const char* szDevice, int baudrate = 115200);
		virtual bool Connect(const wchar_t* szDevice, int baudrate = 115200);
		bool Connect(void);
		bool AutoSelect(CString &sFound);
		void SetupBootModeParameters();

		virtual bool CheckForNewDevice(bool bForce = false);
	protected:
		CDeviceTracer *m_pTracer;
	};

} // HsmDeviceComm

