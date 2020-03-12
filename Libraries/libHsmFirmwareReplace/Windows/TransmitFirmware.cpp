/*=================================================================================
  TransmitFirmware.cpp
//=================================================================================
   $Id: TransmitFirmware.cpp 293 2018-07-10 17:03:44Z Fauth, Dieter $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// TransmitFirmware.cpp : implementation file
//

#include "stdafx.h"
#include "TransmitFirmware.h"
#include "PortManGui.h"
#include "DeviceTracer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace HsmDeviceComm {

///////////////////////////////////////////////////////////////////////////////
//! The constructor of the connector to the low level transmit and receive functions.
/*! Here we use the dfusblib, but other libs are possible as well.
	The low level communication object (m_pComm) is created here.
 @param hClientWindow Window handle to a window we send notifications.
 @param MessageNumberStatus Command ID so the receiver of the notifications can parse our messages.
*/
CTxFirmware::CTxFirmware(HWND hNotifictionReceiver, DWORD MsgStatus)
	: CTxFirmwareBase(hNotifictionReceiver, MsgStatus)
{
	m_pTracer = new CDeviceTracer(this, this);
	ASSERT(m_pTracer != NULL);
}

///////////////////////////////////////////////////////////////////////////////
//! Destructor.
/*! Deletes the low level communication object.
*/
CTxFirmware::~CTxFirmware()
{
	delete m_pTracer;
	m_pTracer = NULL;
}

const int SerialTypes = CPortType::HSMCDC | CPortType::JUNGO_CDC | CPortType::HHPCDC | CPortType::HIDPOS;
const int RemTypes = CPortType::HIDREM;

///////////////////////////////////////////////////////////////////////////////
//! A simple way to auto select the device.
/*! This example uses the very first (lowest COMx number) Imager with 
	the CDC-ACM interface or HidPos.
 @return true for success, else false.
*/
bool CTxFirmware::AutoSelect(CString &sFound)
{
	int nFound=0;

	int NumDevices = GetDeviceCount();	// How many devices do we have in the list?
	for (int index=0; index<NumDevices; index++)
 	{
		if (IsDeviceClass(index, SerialTypes))
		{
			// A very simplistic auto select: Just pick the very first device we see
			CString sDevice = GetDisplayName(index);
			if(sDevice.GetLength() > 0)	// paranoia
			{
				if(!nFound++)
					sFound = sDevice;
			}
		}
	}
	if (nFound == 0)	// search for Rem interface if there is nothing else
	{
		for (int index = 0; index<NumDevices; index++)
		{
			if (IsDeviceClass(index, RemTypes))
			{
				// A very simplistic auto select: Just pick the very first device we see
				CString sDevice = GetDisplayName(index);
				if (sDevice.GetLength() > 0)	// paranoia
				{
					if (!nFound++)
						sFound = sDevice;
				}
			}
		}
	}

	return (nFound == 1);
}
///////////////////////////////////////////////////////////////////////////////
//! Connects to the device with the curent port/device setting.
/*! This function is not used in the MFC version since we use the paramterless overload here.
 @return true for success, else false.
*/
bool CTxFirmware::Connect(const char* szDevice, int baudrate)
{
	CString sDevice(szDevice);
	return CTxFirmware::Connect(sDevice, baudrate);
}

///////////////////////////////////////////////////////////////////////////////
//! Connects to the device with passed port/device setting.
/*! 
 @return true for success, else false.
*/
bool CTxFirmware::Connect(const wchar_t* szDevice, int baudrate)
{
	bool success=true;
	if(baudrate==0)
		baudrate = CBR_115200;
	
	Baudrate(baudrate);
	Set8N1();

	CString sDevice(szDevice);
	if(sDevice.GetLength()==0)
	{
		AutoSelect(sDevice);
	}
	m_sDevice = sDevice;
	success &= SetDisplayNameWithCheck(sDevice);
	return success && CTxFirmware::Connect();
}

///////////////////////////////////////////////////////////////////////////////
//! Connects to the device with the curent port/device setting.
/*! If the port is used by another application, we try to borrow the port.
	This only works if the other application supports the borrow/grant mechanism of dfUsbLib.
 @return true for success, else false.
*/
bool CTxFirmware::Connect(void)
{
	bool success = false;
	// WIN10 does not work reliable with two apps having the same HidPos open.
	// Win7 and older was just fine. So lets workaround.
	// HIDREM still works as in the older days.
	// The reason why HidPos behaves different is the new driver for POS devices in Win10.
	if (IsDeviceClass(CPortType::HIDPOS))
	{
		success = DoBorrowPort();	
	}
			
	if (!success)
		success=OpenConnection();					// start to communicate

	if (!success)
		success = DoBorrowPort();

	success &= SetReadTimeout(500,10,500);
	if(m_pTracer!=NULL)
		success &= m_pTracer->Prepare();
	
	return success;
}

void	CTxFirmware::SetupBootModeParameters()
{
	Set8N1();
	Baudrate(CBR_115200);
}

bool CTxFirmware::CheckForNewDevice(bool bForce)
{
	bool retVal = false;
	if(m_pTracer != NULL)
		retVal = m_pTracer->CheckForNewDevice(bForce);
	return retVal;
}

} // HsmDeviceComm
