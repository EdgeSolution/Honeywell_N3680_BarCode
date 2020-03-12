// ============================================================================
// $Id: AndroidUsbManager.cpp 203 2019-01-31 16:41:15Z e411776 $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Class to manage USB interfaces.
	Details.
	\file */
//=============================================================================

#include "stdafx.h"
#include "AndroidUsbManager.h"

CUsbManager::CUsbManager(HWND hNotifictionReceiver, DWORD MsgStatus)
{
	m_type=Unknown;
	m_pUsb = NULL;
}

CUsbManager::~CUsbManager()	
{
	delete m_pUsb;
}

bool CUsbManager::isValid()
{
	bool Valid = (m_type!=Unknown);
	Valid &= (m_pUsb != NULL);
	if(Valid)
		Valid &= m_pUsb->isValid();
	return Valid;
}

// This will be called from java somehow
bool CUsbManager::Connect(UsbDetails_t &device, int baudrate)
{
	if(m_type!=device.m_type)
	{
		m_type=device.m_type;
		delete m_pUsb;
		m_pUsb=NULL;
		
		switch (m_type)
		{
		case CdcAcm:
			m_pUsb = new CUsbCdcAcm(device);
			break;
		case HidPos:
			m_pUsb = new CUsbHidPos(device);
			break;
		default:
			break;
		}
		if(m_pUsb!=NULL)
			m_pUsb->Connect();
	}
	return isValid();
}

bool CUsbManager::Disconnect()
{
	if(m_pUsb!=NULL)
		m_pUsb->Disconnect();
	delete m_pUsb;
	m_pUsb=NULL;
	m_type = Unknown;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//! Prepare for the potential disconnect of USB devices.
/*! You might want to close the handle or file descriptor if the communication lib
		does not do it automatically.
*/
void CUsbManager::PrepareDisconnect()
{
	if(m_pUsb!=NULL)
		m_pUsb->PrepareDisconnect();
}

bool CUsbManager::IsConnected()
{
	return (m_pUsb != NULL);
}

///////////////////////////////////////////////////////////////////////////////
//! Check whether we are connected to the USB device.
/*! This implementantion just calls the function of the communication lib.
 The lib handles quite some nasty details for us.

 For non_USB environments, this function can simply return true.
 @return true if connected, else false
*/
bool CUsbManager::IsUSBConnected()
{
	return IsConnected();
}

///////////////////////////////////////////////////////////////////////////////
//! Control whether a callback for data received shall be called.
/*!
 @param *pFunctorRead Pointer to a CallbackRead_t object. NULL turns of the callback. Default = NULL;
 @param nMaxLength The callback will use this as a maximum length. Default = 256.
 @return  True for success.
*/
bool CUsbManager::SetReadCallback(CallbackRead_t *pFunctorRead, size_t nMaxLength)
{
	UNUSED(nMaxLength);
	if(m_pUsb!=NULL)
		return m_pUsb->SetReadCallback(pFunctorRead);

	return false;
}

///////////////////////////////////////////////////////////////////////////////
//! Control whether a callback for status changes shall be called.
/*!
 @param *pFunctorStatus Pointer to a CallbackStatus_t object. NULL turns of the callback. Default = NULL;
 @param mask A bitmask that enables the kind of status changes you want to get notified.
 @return  True for success.

	Can also be used to set the bitmask if you use a windows message for the status change notifications.
	In this case call it with a NULL for the first parameter: SetStatusCallback(NULL, MyStatusMask);

	\remark In a lot of cases it is a good idea to use a windows message for status changes,
	but a callback for the "data arrived".

	The mask can be any combination of the following bits:
	EV_PNP_ARRIVE, EV_PNP_REMOVE,
	EV_RXCHAR, EV_RXFLAG, EV_TXEMPTY, EV_CTS, EV_DSR, EV_RLSD, EV_BREAK, EV_ERR, EV_RING,
	EV_RX80FULL, EV_EVENT1, EV_EVENT2

	Though some of them make not sense for some devices. In this case, they are saveley ignored.

	The two events EV_PNP_ARRIVE and EV_PNP_REMOVE are only generated when CPortMan handles the WM_DEVICECHANGE
	messages internaly.
*/
bool CUsbManager::SetStatusCallback(CallbackStatus_t *pFunctorStatus, DWORD mask)
{
	UNUSED(mask);
	UNUSED(pFunctorStatus);

	return false;
}

///////////////////////////////////////////////////////////////////////////////
//! Remove all received data from buffers.
/*! If you do not implement this, then the base class uses Reads to empty the buffers.
*/
void CUsbManager::PurgeReceive()
{
	if(m_pUsb!=NULL)
		m_pUsb->PurgeReceive();
}

// Currently not used, for true RS232 connections we are going to need it.
void CUsbManager::InitRxTx()
{
}


// READ FUNCTIONS: work for returning buffer size
// CAUTION: Timeout should be ~500ms
int CUsbManager::Read()
{
	if(m_pUsb==NULL)
		return -1;
	return m_pUsb->Read();
}

size_t CUsbManager::Read(UCHAR *buffer, size_t Size)
{
	if(m_pUsb==NULL)
		return 0;
	int return_val = m_pUsb->Read(buffer, Size);
    if(return_val > -1)
        return return_val;
    else return 0;
}

size_t CUsbManager::ReadBlock(UCHAR *buffer, size_t Size)
{
	if(m_pUsb==NULL)
		return 0;
	int return_val = m_pUsb->Read(buffer, Size);
	if(return_val > -1)
		return return_val;
	else return 0;
}

bool CUsbManager::Write(UCHAR byte)
{
	if(m_pUsb==NULL)
        return false;
	return m_pUsb->Write(byte);
}

bool CUsbManager::Write(const UCHAR *pByte, size_t Size)
{
    if(m_pUsb==NULL)
        return false;
    return m_pUsb->Write(pByte, Size);
}

///////////////////////////////////////////////////////////////////////////////
//! Check for reliable connections like USB
/*!
 @return true if USB
*/
bool CUsbManager::IsSecureConnection()
{
	return true;	// so far we only support USB, which does error handling by itself.
}

