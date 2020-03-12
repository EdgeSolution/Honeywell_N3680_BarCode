/*=================================================================================
  DeviceTracer.cpp
//=================================================================================
   $Id: $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2017
//=================================================================================*/
//! \file
// DeviceTracer.cpp : implementation file
//

#include "stdafx.h"
#include "DeviceTracer.h"
#include "PortMan.h"
#include "TransmitFirmware.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// we can turn of debug TRACE here
#define TRACEHERE 1
#ifdef TRACEHERE
#define LTRACE TRACE
#else
#define LTRACE(...)
#endif

namespace HsmDeviceComm {

///////////////////////////////////////////////////////////////////////////////
//! The constructor of the tracer.
/*! Here we use the dfusblib, but other libs are possible as well if they support
	Device change messages.
 @param pTx Pointer to controlling class.
 @param pComm Pointer to Comunication class.
*/
CDeviceTracer::CDeviceTracer(CTxFirmware *pTx, CPortMan *pComm)
: m_StatusDispatch(NULL)
, m_pComm(pComm)
, m_pTx(pTx)
, m_OldDeviceClass(0)
{
	ASSERT(m_pComm != NULL);
	ASSERT(m_pTx != NULL);
	m_StatusDispatch = new TSpecificPortFunctor2<CDeviceTracer, bool, DWORD, DWORD>(this, &CDeviceTracer::StatusCallback);
	// manual reset, initially reset
	m_hDeviceChangeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ASSERT(m_hDeviceChangeEvent != NULL);

}

CDeviceTracer::~CDeviceTracer()
{
	m_pComm->SetStatusCallback(NULL, 0);
	delete m_StatusDispatch;
	m_StatusDispatch = NULL;
	::CloseHandle(m_hDeviceChangeEvent);
}

bool CDeviceTracer::Prepare()
{
	ASSERT(m_pComm != NULL);
	GetOldDeviceList();
	ResetEvent(m_hDeviceChangeEvent);
	m_OldDeviceClass = m_pComm->GetDeviceClass();
	return m_pComm->SetStatusCallback(m_StatusDispatch, EV_PNP_NEWDEV);
}

bool CDeviceTracer::StatusCallback(DWORD dwStatus, DWORD dwError)
{
	LTRACE(_T("StatusCallback %x,%x\r\n"), dwStatus, dwError);
	if ((dwStatus&EV_PNP_NEWDEV) && (dwError == ERROR_SUCCESS))
	{
		SetEvent(m_hDeviceChangeEvent);
	}
	return 0;
}

static void* pDummy = NULL;

void CDeviceTracer::GetOldDeviceList()
{
	ASSERT(m_pComm != NULL);
	m_Olddevices.clear();
	int count = m_pComm->GetDeviceCount();
	for (int i = 0; i < count; i++)
	{
		CString Device = m_pComm->GetDisplayName(i);
		if (m_pComm->IsDeviceAvailable(Device))
		{
			LTRACE(_T("Old Device %s\r\n"), (const TCHAR*)Device);
			std::wstring sKey(Device);
			m_Olddevices[sKey] = 1;
		}
	}
}

void CDeviceTracer::GetNewDeviceList()
{
	ASSERT(m_pComm != NULL);
	CMapStringToInt::iterator it;
	m_Newdevices.clear();
	int count = m_pComm->GetDeviceCount();
	for (int i = 0; i < count; i++)
	{
		std::wstring Device = m_pComm->GetDisplayName(i);
		// only add devices not seen before
		it = m_Olddevices.find(Device);
		if (it == m_Olddevices.end())
		{
			LTRACE(_T("New Device stored %s\r\n"), Device.c_str());
			m_Newdevices[Device] = 1;
		}
	}
}

/*
void CDeviceTracer::GetDeviceList(bool bOld)
{
	CStringArray &Devices = bOld ? m_Olddevices : m_Newdevices;
	Devices.RemoveAll();

	int count = m_pComm->GetDeviceCount();
	Devices.SetSize(count);
	for (int i = 0; i < count; i++)
	{
		Devices[i] = m_pComm->GetDisplayName(i);
	}
}
*/

bool CDeviceTracer::CheckForNewDevice(bool bForce)
{
	ASSERT(m_pComm != NULL);
	bool RetVal = false;
	const int NoWait = 0;

	if (WaitForSingleObject(m_hDeviceChangeEvent, NoWait) == WAIT_OBJECT_0)
	{
		ResetEvent(m_hDeviceChangeEvent);
		LTRACE(_T("Got new decice\r\n"));
		GetNewDeviceList();
		RetVal = AnalyzeNewDevices();
	}
	else if (bForce)
	{
		LTRACE(_T("Force searching new decices\r\n"));
		GetNewDeviceList();
		RetVal = AnalyzeNewDevices();
	}

	return RetVal;
}

const unsigned int Prefereddevices = CPortType::HHPCDC | CPortType::JUNGO_CDC | CPortType::USBSER | CPortType::HIDPOS | CPortType::HSMCDC;
const unsigned int SecondaryDevices = CPortType::HIDREM;

bool CDeviceTracer::AnalyzeNewDevices()
{
	ASSERT(m_pComm != NULL);
	bool Found = false;
	CString Device;

	for (CMapStringToInt::iterator it = m_Newdevices.begin(); it != m_Newdevices.end(); it++)
	{
		std::wstring sListed = it->first;
		Device = sListed.c_str();
		LTRACE(_T("AnalyzeNewDevices %s\r\n"), (const TCHAR*)Device);
		if (m_pComm->IsThisOurDevice(Device)) // sanity
		{
			Found = true;	// nothing else to do
			break;
		}
		else
		{
			if (m_pComm->IsDeviceClass(Device, Prefereddevices))
			{
				Found = DoubleCheckDevice(Device);	// found potential device
				break;
			}
			else if (m_pComm->IsDeviceClass(Device, SecondaryDevices))
			{
				if (m_OldDeviceClass&SecondaryDevices)	// same class as before?
				{
					Found = DoubleCheckDevice(Device);	// found potential device
					break;
				}
				else
				{
					// we better wait some time, the primary device might show up soon
					// later I might add switching to this type as well.
					LTRACE(_T("Found REM %s\r\n"), (const TCHAR*)Device);
				}
			}
		}
	}

	return Found;
}

bool CDeviceTracer::DoubleCheckDevice(CString AnyName)
{
	ASSERT(m_pComm != NULL);
	ASSERT(m_pTx != NULL);
	bool Found = false;
	LTRACE(_T("DoubleCheckDevice %s\r\n"), (const TCHAR*)AnyName);
	// prepare a temorary connection so we can check the serial number
	CTxFirmware *pDevice = new CTxFirmware();
	if (pDevice->Connect(AnyName))
	{
		// device wants to send an ACK for successfull flash.
		// try to read it, but it is optional.
		int Ack = pDevice->Read();
		if (Ack == 0x06)
		{
			LTRACE(_T("Got ACK from %s\r\n"), (const TCHAR*)AnyName);
		}
		CString NewSerial;
		int RetVal = ERROR_SUCCESS;
		// Reconnect to new device and read serial number.
		// Device needs some time to setup its command parsers.
		// Without this delay, output might come out via the kbd interface (TERMID144).
		// 500ms was not enough with DR4850. 600ms is good, choosing 900ms.
		// Only delay if we changed TERMID. A succesfully flash shows up as the ACK.
		if (Ack != 0x06)
			Sleep(900);
		pDevice->ReconnectAfterFlashing(RetVal);
		if (RetVal == ERROR_SUCCESS)
		{
			if (0 == pDevice->GetNewSerial().CompareNoCase(m_pTx->GetOldSerial()))
			{
				Found = true;	// it is same device
			}
			else
			{
				LTRACE(_T("Serial numbers do not match: %s - %s\r\n"), (const TCHAR*)AnyName, (const TCHAR*) pDevice->GetNewSerial());
				LTRACE(_T("Serial numbers do not match: %s - %s\r\n"), (const TCHAR*)m_pTx->GetTrueDisplayName(), (const TCHAR*) m_pTx->GetOldSerial());
			}
		}
		else
		{
			LTRACE(_T("ReconnectAfterFlashing failed with %X ------------------------------------\r\n"), RetVal);
		}
	}
	else
	{
		LTRACE(_T("Connect failed ------------------------------------\r\n"));
	}

	delete pDevice;	// remove temporary connection

	if (Found)
	{
		// Open a new connection to our device after closing the old one.
		m_pTx->DisConnect();
		LTRACE(_T("Connect %s\r\n"), (const TCHAR*)AnyName);
		m_pTx->Connect(AnyName);
	}
	return Found;
}

} // HsmDeviceComm

