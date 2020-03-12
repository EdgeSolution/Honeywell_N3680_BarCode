/*=================================================================================
  DeviceComm.cpp
//=================================================================================
   $Id: DeviceComm.cpp 203 2019-01-31 16:41:15Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#include "stdafx.h"
#include "DeviceComm.h"
#include "AsyncParser.h"
#include "HModem.h"
#include "HsmErrorDefs.h"

namespace HsmDeviceComm {

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// This code is in some kind of a transition phase. The goal is to avoid switching from async. mode to sync.
// mode and back for commands. Also there is a new (experimental) communication protocol in the
// devices (not in std. FW). So expect more changes here and please forgive some confusion I might create here.
// If you find bugs or have ideas for improvements, please send me an email: dieter.fauth@honeywell.com.
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//! Constructor.
/*! The 3 optional parameters are used to connect a window to our class. This
way we can send notification messages.

Usage example:
\verbatim
	// Create the device interface. We pass 3 parameters:
	// Our window handle, a message number for the "got data" notification,
	// and a message number for the device removal/arrival notification (optional).
	m_pDevice = new CDeviceComm(m_hWnd, WM_DEVICE_MSG_COMPLETE, WM_DEVICE_MSG_COMMSTATUS);
\endverbatim

 @param hClientWindow that receives the notifications
 @param MessageNumberData used for notifying of arrived data
 @param MessageNumberCommStatus used for notifying of status changes
*/
CDeviceComm::CDeviceComm(HWND hClientWindow, int MessageNumberData, int MessageNumberCommStatus)
:	CCommLib(hClientWindow, MessageNumberCommStatus)
,	m_fUtf16le(false), m_fWantSynHeader(true), m_fUsesSynHeader(false), m_fAsyncMode(false)
,	m_ClientHandle(hClientWindow), m_ClientDataMessageNumber(MessageNumberData)
,	m_ClientStatusMessageNumber(MessageNumberCommStatus)
,	m_fOutHdrMissing(false)
,   m_fFirmwareReplace(false)
,	m_LastCmdError(ERROR_SUCCESS)
//,	m_fWaitingForResponse(false)
{
	m_pReadDispatch = new TSpecificPortFunctor2<CDeviceComm, bool, UCHAR*, size_t> (this, &CDeviceComm::ReadDispatch);
	m_pStatusDispatch = new TSpecificPortFunctor2<CDeviceComm, bool, DWORD, DWORD> (this, &CDeviceComm::StatusDispatch);
	m_pAsyncParser = new CAsyncParser;
	m_pHModem = new CHModem(this);
	m_pCurrentMessage=NULL;

	// we want notifications when our device arrives are leaves
	if(MessageNumberCommStatus!=0)
		SetStatusCallback(NULL, EV_PNP_ARRIVE|EV_PNP_REMOVE);

	// auto reset, initially reset
	m_hCommandResponse = CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(m_hCommandResponse != NULL);
	// auto reset, initially reset
	m_hAsyncMessage = CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(m_hAsyncMessage != NULL);

	SetReadTimeout(1000,10,2000);
}

///////////////////////////////////////////////////////////////////////////////
//! Destructor, does the usual cleanup.
/*!
*/
CDeviceComm::~CDeviceComm(void)
{
	TRACE("Closing Port/Device");
	DisConnect();
	delete m_pHModem;
	delete m_pAsyncParser;
	delete m_pStatusDispatch;
	delete m_pReadDispatch;
	if(m_pCurrentMessage!=NULL)
		delete m_pCurrentMessage;

	CloseHandle(m_hAsyncMessage);
	CloseHandle(m_hCommandResponse);

	TRACE("End of ~CDeviceComm");

}

///////////////////////////////////////////////////////////////////////////////
//! Wait until the device sends the Acknowledge
/*! Until the timeout we try to read bytes from the device and parse ACK, NAK and ENQ.
Any other bytes are discarded while waiting.
@return true if ACK was send, else false
*/
bool	CDeviceComm::WaitForAck(void)
{
	int ret=0;
	while(ret==0)
	{
		ret = Read();
		if ((ret==0x06)	|| (ret==0x15) || (ret==0x05) )
		{
			if(m_fUtf16le)
				Read();	// ignore the second byte if Unicode
			break;
		}
		else if (ret<0)	// timeout
		{
			break;
		}
		else
		{
			ret=0;	// ignore and delete others
		}
	}
	return (ret==0x06);
}

///////////////////////////////////////////////////////////////////////////////
//! Wait until the device sends the CR
/*! Until the timeout we try to read bytes from the device and parse the CR.
Any other bytes are discarded while waiting.
@return true if CR was send, else false
*/
bool	CDeviceComm::WaitForSyncronNonMenuResponse(void)
{
	int ret=0;
	while(ret==0)
	{
		ret = Read();
		m_sCommandResponse.AppendChar(ret);
		if ((ret==0x0D)	|| (ret==0x15) || (ret==0x05) )
		{
			if(m_fUtf16le)
				Read();	// ignore the second byte if Unicode
			break;
		}
		else if (ret<0)	// timeout
		{
			break;
		}
		else
		{
			ret=0;	// ignore and delete others
		}
	}
	return (ret==0x0D);
}

///////////////////////////////////////////////////////////////////////////////
//! Wait until the device sends the reponse
/*! Received all bytes from the device and parse it for valid response bytes.
Some otherwise hard to read bytes are translated into clear text to make debugging easier.
@param [in] ExtraTimeout Add some extra time
@param [in] bOptional Unsupported commands are not flagged as error
@return true for success, else false
*/
bool	CDeviceComm::WaitForSyncronCommandResponse(int ExtraTimeout, bool bOptional)
{
	m_sCommandResponse.Empty();
	enum { WAIT_FOR_ANY, WAIT_FOR_END, DONE } State;
	int ret=0;
	bool success=true;
	State=WAIT_FOR_ANY;
	while(State!=DONE)
	{
		ret = Read();
		if (ret<0)	// got a read timeout (500ms)
		{
			if(ExtraTimeout<=0)	// some commands need more time than our standard read timeout
			{
				State=DONE;
				success=false;
			}
			else
			{
				ExtraTimeout--;
			}
		} // if (ret<0)	// timeout
		else
		{
			m_sCommandResponse.AppendChar(ret);
			switch(State)
			{
				default:
				case WAIT_FOR_ANY:
					{
						if (ret==0x06)
						{
							State=WAIT_FOR_END;
						}
						else if (ret==0x05)
						{
							State=WAIT_FOR_END;
							if(!bOptional)
								success=false; 
						}
						else if (ret==0x15)
						{
							State=WAIT_FOR_END;
							success=false;
						}
						if(m_fUtf16le)
							Read();	// ignore the second byte if Unicode
					}
					break;
				case WAIT_FOR_END:
					{
						if ((ret=='!') || (ret=='.'))
						{
							State=DONE;
						}
						else if (ret!=0)
						{
							State=WAIT_FOR_ANY;
						}
						if(m_fUtf16le)
							Read();	// ignore the second byte if Unicode
					}
					break;
				case DONE:
					break;
			} // switch(State)
		} // if (ret<0)
	} // while(State!=DONE)
	return ((ret=='!') || (ret=='.') )&& success;
}

#ifdef _MFC_VER
bool CDeviceComm::ParseCommandResponse(bool bOptional)
{
	const size_t CMDLEN = 6;
    bool FoundAck=false;
	bool FoundNAK=false;
	bool FoundENQ=false;
	int curPos = CMDLEN;
	const char *Any="\x06\x15\x05";
	CStringA resToken = m_sCommandResponse.Tokenize(Any, curPos);
	while (curPos > 0)
	{
	    char AckByte = m_sCommandResponse[curPos-1];
		FoundAck |= (AckByte == '\x06');
		FoundNAK |= (AckByte == '\x15');
		if(!bOptional)
			FoundENQ |= (AckByte == '\x05');
		resToken = m_sCommandResponse.Tokenize(Any, curPos);
	}

	bool RetVal = FoundAck && !(FoundENQ || FoundNAK);
	if (RetVal)
		SetLastCmdError(ERROR_SUCCESS);
	else if (FoundENQ)
		SetLastCmdError(ERROR_CMD_UNKNOWN);
	else if (FoundNAK)
		SetLastCmdError(ERROR_CMD_FAILED);

    return RetVal;
}
#else // _MFC_VER
bool CDeviceComm::ParseCommandResponse(bool bOptional)
{
	const size_t CMDLEN = 6;
	bool FoundAck = false;
	bool FoundNAK = false;
	bool FoundENQ = false;
	const char *Any = "\x06\x15\x05";
	size_t pos = m_sCommandResponse.find_last_of(Any);
	while (pos != std::string::npos)
	{
		if (pos < CMDLEN)
			break;
		char AckByte = m_sCommandResponse.GetAt(pos);
		FoundAck |= (AckByte == '\x06');
		FoundNAK |= (AckByte == '\x15');
		if (!bOptional)
			FoundENQ |= (AckByte == '\x05');
		pos = m_sCommandResponse.find_last_of(Any, pos - 1);
	}

	bool RetVal = FoundAck && !(FoundENQ || FoundNAK);
	if (RetVal)
		SetLastCmdError(ERROR_SUCCESS);
	else if (FoundENQ)
		SetLastCmdError(ERROR_CMD_UNKNOWN);
	else if (FoundNAK)
		SetLastCmdError(ERROR_CMD_FAILED);

	return RetVal;
}
#endif // _MFC_VER

bool CDeviceComm::WaitForAsyncronCommandResponse(bool bOptional)
{
	bool RetVal=false;
	TRACE("CDeviceComm::WaitForAsyncronCommandResponse\r\n");
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hCommandResponse, 5000))
	{
		RetVal = m_pAsyncParser->GetCommandResponse(m_sCommandResponse);
        if(RetVal)
		    RetVal = ParseCommandResponse(bOptional);
	}
	return RetVal;
}

CStringA CDeviceComm::GetCommandResponse(void)
{
    return m_sCommandResponse.TrimRight("\x06\x15\x05.! ");
}

void CDeviceComm::PurgeCommandResponse(void)
{
	TRACE("++ Reset m_hAsyncMessage\r");
	ResetEvent(m_hAsyncMessage);
	ResetEvent(m_hCommandResponse);
	m_pAsyncParser->PurgeCommandResponses();
	m_sCommandResponse.Empty();
	SetLastCmdError(ERROR_SUCCESS);
}

void CDeviceComm::PurgeIncompleteData()
{
	TRACE("++ PurgeIncompleteData\r");
	m_pAsyncParser->PurgeReceiver();
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command without any changes
/*!
 @param sCommand A CStringA containg the raw command
*/
void CDeviceComm::SendRawCommand(CStringA sCommand)
{
	Write(sCommand);
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands. */
void CDeviceComm::SendMenuCommand(CStringA sCommand, bool ProdConfig)
{
	if(ProdConfig)
		Write ("\x16Y\r" + sCommand);
	else
		Write ("\x16M\r" + sCommand);
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands. */
void CDeviceComm::SendMenuCommand(CStringW sCommandW, bool ProdConfig)
{
	CStringA sCommandA(sCommandW);
	SendMenuCommand(sCommandA, ProdConfig);
}
///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a non-menu header
/*! Add "\x16n\r" as a header and send it to the device.
That header is used for the so called non-menu commands. */
void CDeviceComm::SendNonMenuCommand(CStringA sCommand)
{
	CStringA sFinalCommand = "\x16n," + sCommand + "\r";
	Write(sFinalCommand);
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a non-menu header
/*! Add "\x16n\r" as a header and send it to the device.
That header is used for the so called non-menu commands. */
void CDeviceComm::SendNonMenuCommand(CStringW sCommandW)
{
	CStringA sCommandA(sCommandW);
	SendNonMenuCommand(sCommandA);
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a non-menu header
/*! Add "\x16n\r" as a header and send it to the device.
That header is used for the so called non-menu commands. */
void CDeviceComm::SendNonMenuCommand(const char *szCommand)
{
	CStringA sCommandA(szCommand);
	SendNonMenuCommand(sCommandA);
}

int CDeviceComm::CalculateExtraTimeout(CStringA sCommand)
{
	int ExtraTimeout=0;
	int Length = sCommand.GetLength();
	if (Length == 0)
		return ExtraTimeout;
		
	if(sCommand[Length-1] != '!')
	{
		if(sCommand[Length-2] != '?')
		{
			ExtraTimeout = 1500/500;	// The device needs some extra time to store settings to flash.
		}
	}
	return ExtraTimeout;
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
@param [in] sCommand	menu command
@param [in] bOptional	optional command
@param [in] ProdConfig	ProdConfig command
@return true on success
*/
bool CDeviceComm::ExecuteSyncronMenuCommand(CStringA sCommand, bool bOptional, bool ProdConfig)
{
	int Length = sCommand.GetLength();
	if (Length == 0)
		return false;

	bool RetVal=true;
	TemporallySwitchToSyncronMode();
	PurgeReceive();
	PurgeCommandResponse();
	SendMenuCommand(sCommand, ProdConfig);

	int ExtraTimeout=CalculateExtraTimeout(sCommand);
	if(!WaitForSyncronCommandResponse(ExtraTimeout, bOptional))
	{
		RetVal=false;
	}
	PurgeReceive();
	RestoreAsyncronMode();
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
@param [in] sCommand	menu command
@param [in] bOptional	optional command
@param [in] ProdConfig	ProdConfig command
@return true on success
*/
bool CDeviceComm::ExecuteSyncronMenuCommand(CStringW sCommandW, bool bOptional, bool ProdConfig)
{
	CStringA sCommandA(sCommandW);
	return ExecuteSyncronMenuCommand(sCommandA, bOptional, ProdConfig);
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
@param [in] sCommand	menu command
@param [in] bOptional	optional command
@param [in] ProdConfig	ProdConfig command
@return true on success
*/
bool CDeviceComm::ExecuteAsyncronMenuCommand(CStringA sCommand, bool bOptional, bool ProdConfig)
{
	TRACE("CDeviceComm::ExecuteAsyncronMenuCommand: %s\r\n", (const char*) sCommand);
	PurgeCommandResponse();
	SendMenuCommand(sCommand, ProdConfig);
	return WaitForAsyncronCommandResponse(bOptional);
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
@param [in] sCommand	menu command
@param [in] bOptional	optional command
@param [in] ProdConfig	ProdConfig command
@return true on success
*/
bool CDeviceComm::ExecuteAsyncronMenuCommand(CStringW sCommandW, bool bOptional, bool ProdConfig)
{
	CStringA sCommandA(sCommandW);
	return ExecuteAsyncronMenuCommand(sCommandA, bOptional, ProdConfig);
}


///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
@param [in] sCommand	menu command
@param [in] bOptional	optional command
@param [in] ProdConfig	ProdConfig command
@return true on success
*/
bool CDeviceComm::ExecuteMenuCommand(const char *szCommand, bool bOptional, bool ProdConfig)
{
	return ExecuteMenuCommand(CStringA(szCommand), bOptional, ProdConfig);
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
@param [in] sCommand	menu command
@param [in] bOptional	optional command
@param [in] ProdConfig	ProdConfig command
@return true on success
*/
bool CDeviceComm::ExecuteMenuCommand(CStringA sCommand, bool bOptional, bool ProdConfig)
{
	if (m_fOutHdrMissing)
	{
		return ExecuteSyncronMenuCommand(sCommand, bOptional, ProdConfig);
	}
	else
	{
		return ExecuteAsyncronMenuCommand(sCommand, bOptional, ProdConfig);
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
@param [in] sCommand	menu command
@param [in] bOptional	optional command
@param [in] ProdConfig	ProdConfig command
@return true on success
*/
bool CDeviceComm::ExecuteMenuCommand(CStringW sCommandW, bool bOptional, bool ProdConfig)
{
	CStringA sCommandA(sCommandW);
	return ExecuteMenuCommand(sCommandA, bOptional, ProdConfig);
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
@param [in] sCommand	menu command
@param [out] sResponse	menu response
@param [in] bOptional	optional command
@param [in] ProdConfig	ProdConfig command
@return true on success
*/
bool CDeviceComm::ExecuteMenuCommand(CStringA sCommand, CStringA &sResponse, bool bOptional, bool ProdConfig)
{
	bool RetVal=ExecuteMenuCommand(sCommand, bOptional, ProdConfig);
	sResponse = GetCommandResponse();

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
@param [in] sCommand	menu command
@param [out] sResponse	menu response
@param [in] bOptional	optional command
@param [in] ProdConfig	ProdConfig command
@return true on success
*/
bool CDeviceComm::ExecuteMenuCommand(const char *szCommand, CStringA &sResponse, bool bOptional, bool ProdConfig)
{
	bool RetVal=ExecuteMenuCommand(CStringA(szCommand), bOptional, ProdConfig);
	sResponse = GetCommandResponse();

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
@param [in] sCommand	menu command
@param [out] sResponse	menu response
@param [in] bOptional	optional command
@param [in] ProdConfig	ProdConfig command
@return true on success
*/
bool CDeviceComm::ExecuteMenuCommand(CStringW sCommandW, CStringW &sResponseW, bool bOptional, bool ProdConfig)
{
	CStringA sCommandA(sCommandW);
	CStringA sResponseA;
	bool RetVal = ExecuteMenuCommand(sCommandA, sResponseA, bOptional, ProdConfig);
	sResponseW = sResponseA;
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
Wrapper from std::string until we changed all from Using CString.
@param [in] sCommand	menu command
@return true on success
*/
bool CDeviceComm::ExecuteMenuCommand(std::string sCommand)
{
	return ExecuteMenuCommand(CStringA(sCommand.c_str()));
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a menu header and wait for the response
/*! Add "\x16m\r" as a header and send it to the device.
That header is used for the so called menu commands.
Legacy, do not use for new designs. Use ExecuteMenuCommand instead.
@param [in] pInputBuffer	buffer with command
@param [in] nBytesInInputBuffer	size of buffer with command
@param [out] pOutputBuffer	buffer with response
@param [in] nSizeOfOutputBuffer	size of buffer with response
@param [out] pnBytesReturned	size of response
@return 0 on success
*/
unsigned short CDeviceComm::SendMenuCommand(
	unsigned char *pInputBuffer,
	unsigned long nBytesInInputBuffer,
	unsigned char *pOutputBuffer,
	unsigned long nSizeOfOutputBuffer,
	unsigned long *pnBytesReturned)
{
	unsigned short RetVal = ERROR_SUCCESS;
	if (pInputBuffer==NULL)
		RetVal = HEDC_INVALID_INPUT_BUFFER;

	if (RetVal == ERROR_SUCCESS)
	{
		CStringA sCommand((char*)pInputBuffer, nBytesInInputBuffer);
		CStringA sResponse;
		const bool bOptional = true;
		const bool ProdConfig = true;
		bool Success = ExecuteMenuCommand(sCommand, sResponse, !bOptional, !ProdConfig);
		if (Success)
		{
			if ((pOutputBuffer == NULL) || (sResponse.GetLength() < nSizeOfOutputBuffer))
			{
				RetVal = HEDC_OUTPUT_BUFFER_TOO_SMALL;
			}
			else
			{
				memcpy(pOutputBuffer, (const char*)sResponse, sResponse.GetLength());
			}
			if (pnBytesReturned != NULL)
			{
				*pnBytesReturned = sResponse.GetLength();
			}
		}
		else
		{
			// Note: This sounds like mixed up, but it minics behavior of old lib.
			RetVal = HEDC_NOT_SUPPORTED;
			if (GetLastCmdError() == ERROR_NOT_SUPPORTED)
				RetVal = HEDC_INVALID_COMMAND;
		}
	}
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a non-menu header and wait for the response
/*! Add "\x16n\r" as a header and send it to the device.
That header is used for the so called non-menu commands.
@param [in] sCommand	menu command
@param [out] sResponse	menu response
@return true on success
*/
bool CDeviceComm::ExecuteSyncronNonMenuCommand(CStringA sCommand, CStringA &sResponse)
{
	bool RetVal=true;
	TemporallySwitchToSyncronMode();
	PurgeReceive();
	PurgeCommandResponse();
	SendNonMenuCommand(sCommand);
	if(!WaitForSyncronNonMenuResponse())
	{
		RetVal=false;
	}
	sResponse = GetCommandResponse();
	PurgeReceive();
	RestoreAsyncronMode();
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a non-menu header and wait for the response
/*! Add "\x16n\r" as a header and send it to the device.
That header is used for the so called non-menu commands.
@param [in] sCommand	menu command
@param [out] sResponse	menu response
@return true on success
*/
bool CDeviceComm::ExecuteAsyncronNonMenuCommand(CStringA sCommand, CStringA &sResponse)
{
	PurgeCommandResponse();
	SendNonMenuCommand(sCommand);
	bool RetVal=WaitForAsyncronCommandResponse();
	if(RetVal)
		sResponse = GetCommandResponse();
	else
	{
		sResponse.Empty();	// sanity
	}

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a non-menu header and wait for the response
/*! Add "\x16n\r" as a header and send it to the device.
That header is used for the so called non-menu commands.
@param [in] sCommand	menu command
@param [out] sResponse	menu response
@return true on success
*/
bool CDeviceComm::ExecuteNonMenuCommand(CStringA sCommand, CStringA &sResponse)
{
	if (m_fOutHdrMissing)
	{
		return ExecuteSyncronNonMenuCommand(sCommand, sResponse);
	}
	else
	{
		return ExecuteAsyncronNonMenuCommand(sCommand, sResponse);
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a non-menu header and wait for the response
/*! Add "\x16n\r" as a header and send it to the device.
That header is used for the so called non-menu commands.
@param [in] sCommand	menu command
@param [out] sResponse	menu response
@return true on success
*/
bool CDeviceComm::ExecuteNonMenuCommand(CStringW sCommandW, CStringW &sResponseW)
{
	CStringA sCommandA(sCommandW);
	CStringA sResponseA;
	bool RetVal = ExecuteNonMenuCommand(sCommandA, sResponseA);
	sResponseW = sResponseA;
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Sends the command with a non-menu header and wait for the response
/*! Add "\x16n\r" as a header and send it to the device.
That header is used for the so called non-menu commands.
@param [in] sCommand	menu command
@param [out] sResponse	menu response
@return true on success
*/
bool CDeviceComm::ExecuteNonMenuCommand(const char *szCommand, CStringA &sResponseA)
{
	bool RetVal = ExecuteNonMenuCommand(CStringA(szCommand), sResponseA);
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Turns the use of HTag header on/off
/*! On means that the device sends the data encapsuled into a structure (the HTag).
@param newval New value for the setting
@return True if switch was successfully
*/
bool CDeviceComm::UseHTagHeader(bool newval)
{
	bool RetVal=true;
	m_fWantSynHeader = newval;
	if(IsConnected())
	{
		RetVal = PrepareHTagHeader();
	}
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Turns the use of HTag header on/off
/*! On means that the device sends the data encapsuled into a structure (the HTag).
@return True if switch was successfully
*/
#define USE_OUTHDR 1
bool CDeviceComm::PrepareHTagHeader()
{
	bool success=false;
	CStringA sCmd;
#ifdef USE_OUTHDR
	// first we try to turn on the newer method. (More reliable communication).
	if (!m_fOutHdrMissing)
	{
		sCmd.Format("OUTHDR%i!", m_fWantSynHeader ? 0x7f : 0);
		success = ExecuteSyncronMenuCommand(sCmd);
		if(success)
		{
			m_fUsesSynHeader=m_fWantSynHeader;
		}
	}
#endif
	if(!success)
	{
		m_fOutHdrMissing=true;	// no not bother trying the OUTHDR method in future
		// use DECHDR if OUTHDR is not available yet.
		sCmd.Format("DECHDR%i!", (int)m_fWantSynHeader);
		success = ExecuteSyncronMenuCommand(sCmd);
		if(success)
		{
			m_fUsesSynHeader=m_fWantSynHeader;
		}
	}
	return success;
}

bool CDeviceComm::Connect(CStringA sDeviceName, int baudrate)
{
// #ifndef Android
// #error "Needs some adjustments here for non-Android builds"
// #endif
// FIXME for non-Android
	return Connect();
}

///////////////////////////////////////////////////////////////////////////////
//! Connects to the device with the curent port/device setting.
/*!
 @return true for success, else false.
*/
bool CDeviceComm::Connect(void)
{
	bool success=false;
	success = CCommLib::OpenConnection();		// start to communicate
	if(!success)
		success=DoBorrowPort();					// try to borrow port from other dfusblib tools

    SetStatusCallback(m_pStatusDispatch);
	return success && EnableRobustCommunication();
}

///////////////////////////////////////////////////////////////////////////////
//! Enables Asyncronuous communication with HTagHeader.
/*!
 @return true for success, else false.
*/
bool CDeviceComm::EnableRobustCommunication()
{
    bool success=true;
    if(!m_fFirmwareReplace) // we must not disturb communication after waking from FW replace!
    {
		PurgeReceive();
		PurgeIncompleteData();
		success = UseHTagHeader(true);
        SwitchToAsyncronMode();
    }
    return success;
}
///////////////////////////////////////////////////////////////////////////////
//! Disconnects the port/device.
/*!
 @return true for success, else false.
*/
bool CDeviceComm::DisConnect(void)
{
	UseHTagHeader(false);
	return CCommLib::CloseConnection();					// stop communication
}

///////////////////////////////////////////////////////////////////////////////
//! Calls the port paramters dialog.
/*! Also ensures the internal connection logic is maintained (callbacks etc.).
 @return true for success, else false.
*/
bool CDeviceComm::OnOptions(void)
{
	bool success=CCommLib::OnOptions();
	if(success == true)
	{
		success = Connect();
	}
	return success;
}

///////////////////////////////////////////////////////////////////////////////
//! Opens a dialog for port/device only.
/*! Use this to present a dialog to the user so he can pick his port//device.
 \return true on success
*/
bool CDeviceComm::OnPorts(bool DoConnect) 
{
	bool success=CCommLib::OnPorts();
	if(DoConnect && (success == true))
	{
		success = Connect();
	}
	return success;
}


///////////////////////////////////////////////////////////////////////////////
//! Handler for status messages from low level communication class.
/*! 
	CAUTION: Called by a different thread context!!!
	This is a callback that gets called for any status chnages in the lower level 
	communicatrion class (dfUsbLib). We wire the dispatch functions in our constructor.
	The dwStatus can be any combination of the following bits:
	EV_PNP_ARRIVE, EV_PNP_REMOVE,
	EV_RXCHAR, EV_RXFLAG, EV_TXEMPTY, EV_CTS, EV_DSR, EV_RLSD, EV_BREAK, EV_ERR, EV_RING,
	EV_RX80FULL, EV_EVENT1, EV_EVENT2

@param dwStatus A Port Event
@param dwError Win32 error number if any error occured 
@return true if post was succesfull, else false
*/
bool CDeviceComm::StatusDispatch(DWORD dwStatus, DWORD dwError)
{
	if(dwStatus & EV_PNP_ARRIVE)
	{
		PrepareHTagHeader();	// we must turn it on after the device was unplugged.
	}

	return PostStatusMessage(dwStatus, dwError);
}

// ---------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//! Receives the bytes received asyncronuously by the Communication Lib
/*!
CAUTION: Called by a different thread context!!!
Receiving can occure at any given time. We receive and parse the received bytes
with a second thread. If there is a complete message packet received, we can notify
the application with a windows message.

@param RxChar Pointer the received bytes
@param nLength Amount of the received bytes
@return Always false
*/
bool CDeviceComm::ReadDispatch(unsigned char* RxChar, size_t nLength)
{
//	TRACE(":: %z\r",nLength);
//	TRACE(":: %i, %hs\r"),nLength, RxChar);

	int RxStatus;
	if(m_fUsesSynHeader)
	{
		RxStatus = m_pAsyncParser->ParseAsyncMessage(RxChar, nLength);
		if(RxStatus & CAsyncParser::UseHModemRx)
		{
			RxStatus = ReadHModem();
		}
	}
	else
	{
		m_pAsyncParser->StoreRawToFifo(RxChar, nLength);
		RxStatus = CAsyncParser::ReceivedRawData;
	}

	if(RxStatus & CAsyncParser::ReceivedCommandResponse)
	{
		TRACE("++ set m_hCommandResponse\r");
		SetEvent(m_hCommandResponse);
	}

	if (RxStatus & CAsyncParser::ReceivedImagePortion)
	{
		ShowProgress();
	}

	// Notify a client if there is any
	if(RxStatus & (CAsyncParser::ReceivedHTag | CAsyncParser::ReceivedRawData | CAsyncParser::HModemError))
	{
		if (!PostDataMessage())
		{
			TRACE("++ set m_hAsyncMessage\r");
			SetEvent(m_hAsyncMessage);
		}
	}
	return false; // dummy return value
}


int CDeviceComm::ReadHModem(void)
{
	int RxStatus = CAsyncParser::HModemError;
	if(m_pHModem->Receive(m_pAsyncParser->GetCollector(), m_pAsyncParser->GetExpectedBufferSize()))
	{
		RxStatus = m_pAsyncParser->FinalizeCollector();
//		RxStatus = CAsyncParser::ReceivedHTag;	// flag that we've got a valid data message
	}
	else
	{
		RxStatus = m_pAsyncParser->SetBadImage();
	}

	return RxStatus;
}

///////////////////////////////////////////////////////////////////////////////
//! Send a progress message.
/*! This can be used to inform another thread of current progess.
*/
void CDeviceComm::ShowProgress(void)
{
	PostStatusMessage(EV_PROGRESS);
}

///////////////////////////////////////////////////////////////////////////////
//! Send a message to show we have a new status.
/*!
 @param wParam Typical windows param
 @param lParam Typical windows param
*/
bool CDeviceComm::PostStatusMessage(WPARAM wParam, LPARAM lParam)
{
	// Notify a client if there is any
	if ((m_ClientHandle!=NULL) && (m_ClientStatusMessageNumber!=0))
	{
		::PostMessage(m_ClientHandle, m_ClientStatusMessageNumber, wParam, lParam);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//! Send a message to show we have new data.
/*!
 @param wParam Typical windows param
 @param lParam Typical windows param
*/
bool CDeviceComm::PostDataMessage(WPARAM wParam, LPARAM lParam)
{
	// Notify a client if there is any
	if ((m_ClientHandle!=NULL) && (m_ClientDataMessageNumber!=0))
	{
		::PostMessage(m_ClientHandle, m_ClientDataMessageNumber, wParam, lParam);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//! Read a fresh message from the receive fifo.
/*!
@return Pointer to a CMessageDetails containing received data
*/
CMessageDetails *CDeviceComm::ReadRxFifo(void)
{
	return m_pAsyncParser->ReadRxFifo();
}

///////////////////////////////////////////////////////////////////////////////
//! Read oldest message from fido and parse it for details
/*! Call this function from your OnDeviceMessage handler to get the required
	details about the received data. Here is an example:
	\verbatim
	LRESULT CYourClass::OnDeviceMessage(WPARAM param1, LPARAM param2)
	{
		UNUSED(param1);
		UNUSED(param2);

		switch(m_pDevice->ReadAndParseMessage())
		{
			case CMessageDetails::Text:
				HandleText();
			break;
			case CMessageDetails::Image:
				HandleImage();
			break;
			case CMessageDetails::CmdResponse:
				if(m_pDevice->GetAckByte()==0x15)
				{
					HandleNAK();
				}
				break;
			default:
			break;
		}
		return 0; // dummy value
	}

 \endverbatim
 @return Message type
*/
int CDeviceComm::ReadAndParseMessage(void)
{
	if(m_pCurrentMessage!=NULL)
		delete m_pCurrentMessage;	// remove the old message

	m_pCurrentMessage = ReadRxFifo();
	if(m_pCurrentMessage!=NULL)
		return m_pCurrentMessage->ParseMessage();
	else
		return CMessageDetails::None;
}

int CDeviceComm::WriteCurrentPayloadToFile(CString sFileName)
{
	if(m_pCurrentMessage!=NULL)
		return m_pCurrentMessage->WritePayloadToFile(sFileName);
	else
		return 0;
}
///////////////////////////////////////////////////////////////////////////////
//! Stops the watch thread
/*! All communication works now with simple Read and Write.
*/
void CDeviceComm::SwitchToSyncronMode(void)
{
	TRACE("++ SwitchToSyncronModee\r");
	SetReadCallback(NULL);
	m_fAsyncMode = false;
}

///////////////////////////////////////////////////////////////////////////////
//! Stops the watch thread, but maintains the original state.
/*! All communication works now with simple Read and Write.
	Use it together with RestoreAsyncronMode().
*/
void CDeviceComm::TemporallySwitchToSyncronMode(void)
{
	SetReadCallback(NULL);
}

///////////////////////////////////////////////////////////////////////////////
//! Starts the watch thread
/*! Receiving can occure at any given time. We receive and parse the received bytes
with a second thread. If there is a complete HTag packet received, we can notify
the application with a windows message.
*/
void CDeviceComm::SwitchToAsyncronMode(void)
{
	ASSERT(m_pReadDispatch!=NULL);
	m_fAsyncMode = true;
	const size_t MAX_COMM_BLOCK_SIZE = 30000;
	SetReadCallback(m_pReadDispatch, MAX_COMM_BLOCK_SIZE);
}


///////////////////////////////////////////////////////////////////////////////
//! Restarts the watch thread if we had it running before.
/*! Use it togehter with TemporallySwitchToSyncronMode().
*/
void CDeviceComm::RestoreAsyncronMode(void)
{
	if (m_fAsyncMode)
		SwitchToAsyncronMode();
}

///////////////////////////////////////////////////////////////////////////////
//! Wait until we've received a message from the device.
/*! 
 @param timeout
 @return true on success
*/
bool	CDeviceComm::WaitForAsyncronMessage(int timeout)
{
	TRACE("++ WaitForAsyncronMessage\r");
	return (WAIT_OBJECT_0 == WaitForSingleObject(m_hAsyncMessage, timeout));
}

///////////////////////////////////////////////////////////////////////////////
//! Wait until a barcode has been read. Can trigger the device.
/*! The timeout paramter is optional.
 @param [out] &sAsyncMessage Contains the response after a successfull return
 @param bTrigger Trigger device if true
 @param bPurge Purge responses if true (normal case)
 @param timeout if it takes too long 
*/
CMessageDetails::PayloadType_t CDeviceComm::ReadBarcode(CStringA &sAsyncMessage, bool bTrigger, bool bPurge, int timeout)
{
	CMessageDetails::PayloadType_t RetVal=CMessageDetails::None;
	if(bPurge)
	{
		sAsyncMessage.Empty();
		PurgeCommandResponse();
	}
	if(bTrigger)
		SendRawCommand("\x16T\r");	// trigger device
	if(WaitForAsyncronMessage(timeout))
	{
		delete m_pCurrentMessage;	// remove the old message

		m_pCurrentMessage = ReadRxFifo();
		if(m_pCurrentMessage!=NULL)
		{
			RetVal = m_pCurrentMessage->ParseMessage();
			sAsyncMessage = m_pCurrentMessage->GetRawPayloadData();
			TRACE(":: %s\r",(const char*)sAsyncMessage);
		}
		else
		{
			TRACE(":- m_pCurrentMessage==NULL\r");
		}
	}
	else
	{
		TRACE(":- TIMEOUT\r");
	}
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Ask the device for firmware version string.
/*!
 @param [out] VersionString
 @return bool
*/
    bool CDeviceComm::RetrieveVersionFromDevice(CStringA &VersionString)
    {
        bool RetVal = false;
        int Retry = 2;
        while (Retry--)
        {
            RetVal = ExecuteMenuCommand("REV_WA?.", VersionString);
            RetVal &= CleanupResponseString("REV_WA", VersionString);
            if (RetVal)
                break;
        }
        return RetVal;
    }

///////////////////////////////////////////////////////////////////////////////
//! Internal function to remove unwanted parts from a version response.
/*!
 @param Version A version response from the device will be cleaned
*/
    bool CDeviceComm::CleanupResponseString(const char *str, CStringA &Version)
    {
        int start = Version.Find(str);
        if (start >= 0)
        {
            Version = Version.Mid(start+strlen(str));
        }
        Version.Replace("\x06", "");	// Note: Trim is not available in all string classes
        Version.Replace(":", "");
        Version.Replace(".", "");
        Version.Replace("!", "");
        Version.Replace(" ", "");
        return (start >= 0);
    }

///////////////////////////////////////////////////////////////////////////////
//! Ask the device for serial number string.
/*!
@param [out] SerialString
@return bool
*/
    bool CDeviceComm::RetrieveSerialStringFromDevice(CStringA &SerialString)
    {
        const bool bOptional = true;
        const bool bProdConfig = true;
        bool RetVal = false;
        int Retry = 2;
        while (Retry--)
        {
            RetVal = ExecuteMenuCommand("SERNUM?.", SerialString, !bOptional, bProdConfig);
            RetVal &= CleanupResponseString("SERNUM", SerialString);
            if (RetVal)
                break;
        }
        return RetVal;
    }

///////////////////////////////////////////////////////////////////////////////
//! Check if version of device is same as passed.
/*!
 @param [out] sNewDeviceVersion
 @return true if same
*/
    bool CDeviceComm::isSameVersion(CStringA &sNewDeviceVersion)
    {
        bool RetVal=false;
        CStringA sCurrentDeviceVersion;
        if (RetrieveVersionFromDevice(sCurrentDeviceVersion))
        {
            RetVal = (sCurrentDeviceVersion==sNewDeviceVersion);
        }
        return RetVal;
    }

///////////////////////////////////////////////////////////////////////////////
//! Try to setup device for a good communication.
/*! We send several commands that turn off special communication settings
that could disturb our communication. Some of these commands might not be supported by all devices,
so we ignore errors.
*/
void CDeviceComm::InitialCommunication()
{
    const bool bOptional = true;	// optional commands do not flag an error
    m_fWantSynHeader = false;
    m_fUsesSynHeader = false;
    m_fOutHdrMissing = true;
    SwitchToSyncronMode();
    // Turn certain protocolls off, they could disturb our communication
    // Also ensure manual trigger is active since all auto trigger could read a barcode and distrub communication.
    for(int j=0; j<4; j++)
    {
        // Send a few ACK to ensure the ACKNAK modes do not disturb us here
        // Regular command parser will ignore the extra ACKs
        // In case there are much more packets in the out queue of the reader, this might not be enough.
        // Then you can add some code that reads all packets here.
        Write("\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06",10);
        // Some of the  commands might be not supported by a given device.
        bool success = ExecuteMenuCommand("ECIOUT0;DECHDR0;OUTHDR0;232ACK0;USBACK0,CTS0;TRGMOD0;DBGGEN0,LOG0!", bOptional);
        Write(0x06);	// Make the device happy if it was in ACKNAK mode. It will be ignored in other modes.

        if (success)
            break;
        else
            Sleep(200);
    }

}

}	// namespace HsmDeviceComm
