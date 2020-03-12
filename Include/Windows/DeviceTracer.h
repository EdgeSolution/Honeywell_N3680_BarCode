/*=================================================================================
  DeviceTracer.h
//=================================================================================
   $Id: $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2017
//=================================================================================*/
//! \file
// DeviceTracer.h : header file
//

#pragma once

#include <map>
#include <string>
#include <iostream>
#include "PortFunctor.h"

class CPortMan;			//!< The main class of dfusblib.

namespace HsmDeviceComm {

	class CTxFirmware;

	///////////////////////////////////////////////////////////////////////////////
	//! Finds a device even if the communication iterface has been changed by a new firmware.
	/*!
		If a new firmware uses another USB VID/PID, the OS will assign another Com port.
		Normally we would not find our device and run into a timeout followed by an error message 
		to the user.
		Here we avoid the error situation for a better user experience.
		Before downloading, we store existing devices.
		While waiting for the device to show up on the USB, we check for new devices showing up.
		If a new device is one of interesting ones we try to open it temporarily and read its serial number.
		If serial numbers of new and old devices match, then we found the new interface.
		Then we close the old interface and re-open with the new name.
		Here we use the dfusblib as we do in the other HEDC places. It supports a lot of the boilerplate code
		for device change notifications with a simple interface.

		Currently this is Windows only because dfUsbLib is not available on Linux.
	*/

	// A class to help finding the device after firmware replace even if it has changed its interface.
	class CDeviceTracer
	{
	public:
		CDeviceTracer(CTxFirmware *pTx, CPortMan *pComm);
		~CDeviceTracer();

		bool Prepare();
		bool CheckForNewDevice(bool bForce);
	protected:
		//void GetDeviceList(bool bOld);
		void GetOldDeviceList();
		void GetNewDeviceList();
		bool AnalyzeNewDevices();
		bool DoubleCheckDevice(CString AnyName);

		bool StatusCallback(DWORD dwStatus, DWORD dwError);
		CallbackStatus_t *m_StatusDispatch;
		HANDLE m_hDeviceChangeEvent;					//!< a device appeared
		CPortMan *m_pComm;							//!< The low level comm port access
		CTxFirmware *m_pTx;
		typedef std::map<std::wstring, int> CMapStringToInt;
		CMapStringToInt m_Olddevices;
		CMapStringToInt m_Newdevices;
		unsigned int m_OldDeviceClass;
	};

} // HsmDeviceComm
