/*=================================================================================
  CommLib.cpp
//=================================================================================
   $Id: CommLib.cpp 158 2018-12-06 17:58:56Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#include "stdafx.h"
#include "CommLib.h"

CCommLib::CCommLib(HWND hNotifictionReceiver, DWORD MsgStatus)
: CommLibBase (hNotifictionReceiver, MsgStatus)
{
}

CCommLib::~CCommLib(void)
{
}

///////////////////////////////////////////////////////////////////////////////
//! Initialize the low level communication object.
/*! Called before the XModem transfer. Here we just set the timeout values for the Read.
	Baudrate etc. are handled long before this time.
*/
void CCommLib::InitRxTx(void)
{
	SetReadTimeout(1500, 10, 1500);
}

///////////////////////////////////////////////////////////////////////////////
//! Check whether we are connected to the USB device.
/*! This implementantion just calls the function of the communication lib.
 The lib handles quite some nasty details for us.
 Doing it here would require to create a hidden window, registering the WM_DEVICECHANGE message
 and in the handler if it to check wheter our device is disconnected.

 For non_USB environments, this function can simply return true.
 @return true if connected, else false
*/
bool CCommLib::IsUSBConnected(void)
{
	return IsConnected();
}

///////////////////////////////////////////////////////////////////////////////
//! Prepare for the potential disconnect of USB devices.
/*! You might want to close the handle or file descriptor if the communication lib
		does not do it automatically.
*/
void CCommLib::PrepareDisconnect(void)
{
	// noting to do if dfUsbLib is used. 
	// It handles all USB disconnect automatically driven by the WM_DEVICECHANGE event.
}

///////////////////////////////////////////////////////////////////////////////
//! Check for reliable connections like USB
/*!
 @return true if USB
*/
bool CCommLib::IsSecureConnection()
{
	return !IsUsesSerialparameters();	// so far only USB has no parameters
}
