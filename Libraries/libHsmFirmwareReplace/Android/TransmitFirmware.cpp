/*=================================================================================
  TransmitFirmware.cpp
//=================================================================================
   $Id: TransmitFirmware.cpp 203 2019-01-31 16:41:15Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// TransmitFirmware.cpp : implementation file
//
#include "stdafx.h"
#include "TransmitFirmware.h"
#include "AndroidUsbManager.h"
//#include "DeviceTracer.h"
#include<jni.h>

namespace HsmDeviceComm {

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//CTxFirmware g_CTxFirmware;
#if 0
void setFileDescriptorParams(int a, int b, int c, int d){
	// g_pComm->
}

void sendString(){
	g_CTxFirmware.Write("\x16m\r", 3);
	// send string to device m_return
}
#endif

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
//	m_pTracer = new CDeviceTracer(this, m_pComm);
//	ASSERT(m_pTracer != NULL);
}

///////////////////////////////////////////////////////////////////////////////
//! Destructor.
/*! Deletes the low level communication object.
*/
CTxFirmware::~CTxFirmware()
{
//	delete m_pTracer;
//	m_pTracer = NULL;
}
//////////////////////////////////////////////////////////////////////////////
//! Initialize the low level communication object.
/*! Called before the XModem transfer. Here we just set the timeout values for the Read.
	Baudrate etc. are handled long before this time.
*/
void CTxFirmware::InitRxTx(void)
{
	SetReadTimeout(500, 10, 500);
}

//const int SerialTypes = CPortType::HSMCDC | CPortType::JUNGO_CDC | CPortType::HHPCDC | CPortType::HIDPOS;
//const int RemTypes = CPortType::HIDREM;

///////////////////////////////////////////////////////////////////////////////
//! A simple way to auto select the device.
/*! This example uses the very first (lowest COMx number) Imager with 
	the CDC-ACM interface or HidPos.
 @return true for success, else false.
*/
bool CTxFirmware::AutoSelect(CString &sFound)
{
//	ASSERT(m_pComm!=NULL);
//	int nFound=0;
//
//	int NumDevices = m_pComm->GetDeviceCount();	// How many devices do we have in the list?
//	for (int index=0; index<NumDevices; index++)
// 	{
//		if (m_pComm->IsDeviceClass(index, SerialTypes))
//		{
//			// A very simplistic auto select: Just pick the very first device we see
//			CString sDevice = m_pComm->GetDisplayName(index);
//			if(sDevice.GetLength() > 0)	// paranoia
//			{
//				if(!nFound++)
//					sFound = sDevice;
//			}
//		}
//	}
//	if (nFound == 0)	// search for Rem interface if there is nothing else
//	{
//		for (int index = 0; index<NumDevices; index++)
//		{
//			if (m_pComm->IsDeviceClass(index, RemTypes))
//			{
//				// A very simplistic auto select: Just pick the very first device we see
//				CString sDevice = m_pComm->GetDisplayName(index);
//				if (sDevice.GetLength() > 0)	// paranoia
//				{
//					if (!nFound++)
//						sFound = sDevice;
//				}
//			}
//		}
//	}
//
//	return (nFound == 1);
	return false;
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

#if 0
// Was used in older implementations. Needs cleanup after windows and Linux versions have been done.
// In Android we do not need it.
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
//	if (m_pComm->IsDeviceClass(CPortType::HIDPOS))
//	{
//		success = m_pComm->DoBorrowPort();	
//	}
			
	if (!success)
		success=OpenConnection();					// start to communicate

//	if (!success)
//		success = m_pComm->DoBorrowPort();

	success &= SetReadTimeout(500,10,500);
//	if(m_pTracer!=NULL)
//		success &= m_pTracer->Prepare();
	
	return success;
}
#endif

///////////////////////////////////////////////////////////////////////////////
//! Disconnects the port/device.
/*!
 @return true for success, else false.
*/
bool CTxFirmware::DisConnect(void)
{
	return CloseConnection();					// stop communication
}

///////////////////////////////////////////////////////////////////////////////
//! Calls the port paramters dialog.
/*! Also ensures the internal connection logic is maintained (callbacks etc.).
 @return true for success, else false.
*/
bool CTxFirmware::OnOptions(void)
{
//	const bool DoConnect=true;
//	return m_pComm->OnOptions(!DoConnect);
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//! Calls the port selction dialog.
/*! Also ensures the internal connection logic is maintained (callbacks etc.).
 @return true for success, else false.
*/
bool CTxFirmware::OnPorts(void)
{
//	const bool DoConnect=true;
//	return m_pComm->OnPorts(!DoConnect);
	return false;
}


///////////////////////////////////////////////////////////////////////////////
//! Read settings from the registry or an ini file.
/*! If the application calls SetRegistryKey(_T("CompanyName")), then the registry is used.
	The low level lib handles the corrosponding write in its destructor automatically.
 @param szSec Used as a registry subkey for the settings.
*/
void CTxFirmware::ReadIni(const TCHAR *szSec)
{
//	ASSERT(m_pComm!=NULL);
//	m_pComm->ReadIni(szSec);
}

///////////////////////////////////////////////////////////////////////////////
//! Returns the current DisplayName
/*! This function can be used for status lines etc.
	It also returns "Offline", BadConfig".
 @return DisplayName as CString
*/
CString CTxFirmware::GetTrueDisplayName(void)
{
	return GetTrueDisplayName();
}

void CTxFirmware::SetDisplayName(CString AnyName)
{
	SetCurrentDisplayName(AnyName);
}

void	CTxFirmware::SetupBootModeParameters()
{
	Set8N1();
	Baudrate(CBR_115200);
}

// Was used in Windows to catch changed COMx numbers
// This can happen if a new FW uses a different VID/PID
bool CTxFirmware::CheckForNewDevice(bool bForce)
{
	bool retVal = false;
//	if(m_pTracer != NULL)
//		retVal = m_pTracer->CheckForNewDevice(bForce);
	return retVal;
}

}	// namespace HsmDeviceComm
