/*=================================================================================
  DeviceComm.h
//=================================================================================
   $Id: DeviceComm.h 203 2019-01-31 16:41:15Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#pragma once
#include "CommLib.h"
#include "MessageDetails.h"

namespace HsmDeviceComm {

class CMessageDetails;
class CAsyncParser;
class CHModem;

enum CommMessage_t
{
	Unknown=0,
	HTag=1,
	RawData=2
};


///////////////////////////////////////////////////////////////////////////////
/// Handles all syncronos communication with the device.
/*!
	This is used to send commands to the device and wait for the response.
	It is the easiest way to communicate with the device.

 \ingroup Helpers
 \par requirements
 MFC\n
*/
class CDeviceComm : public CCommLib
{
public:
	CDeviceComm(HWND hClientWindow=NULL, int MessageNumberData=0, int MessageNumberCommStatus=0);
	virtual ~CDeviceComm(void);

	enum											//!< Constants
	{
		ProdConfig=1								//!< Use to tell the XXXXMenuCommand to use a Product config command
	};

	using CCommLib::Connect;
    using CCommLib::Disconnect;
    using CCommLib::Write;
	using CCommLib::Read;

	virtual bool	Connect(void);
	virtual bool	Connect(CStringA sDeviceName, int baudrate);
	virtual bool	DisConnect(void);
    bool EnableRobustCommunication();

	bool OnOptions(void);
	bool OnPorts(bool DoConnect=true);

	void PurgeIncompleteData();
	void  PurgeCommandResponse(void);
	CStringA GetCommandResponse(void);
	void SendRawCommand(CStringA sCommand);			// Sends the command without any changes, no wait

	// Sends the command with a menu header, no wait
	void SendMenuCommand(CStringA sCommand, bool ProdConfig=false);
	void SendMenuCommand(CStringW sCommand, bool ProdConfig=false);

	void SendNonMenuCommand(const char *szCommand);	// Sends the command with a non-menu header, no wait
	void SendNonMenuCommand(CStringA sCommand);		// Sends the command with a non-menu header, no wait
	void SendNonMenuCommand(CStringW sCommand);		// Sends the command with a non-menu header, no wait

	// Sends the command with a menu header, wait for response
	bool ExecuteMenuCommand(const char *szCommand, bool bOptional=false, bool ProdConfig=false);
	bool ExecuteMenuCommand(CStringA sCommand, bool bOptional=false, bool ProdConfig=false);
	bool ExecuteMenuCommand(CStringW sCommand, bool bOptional=false, bool ProdConfig=false);
	// Sends the command with a menu header, wait for response
	bool ExecuteMenuCommand(const char *szCommand, CStringA &sResponse, bool bOptional=false, bool ProdConfig=false);
	bool ExecuteMenuCommand(CStringA sCommand, CStringA &sResponse, bool bOptional=false, bool ProdConfig=false);
	bool ExecuteMenuCommand(CStringW sCommand, CStringW &sResponse, bool bOptional=false, bool ProdConfig=false);
	bool ExecuteMenuCommand(std::string sCommand);
    bool ExecuteNonMenuCommand(const char *szCommand, CStringA &sResponse);
	bool ExecuteNonMenuCommand(CStringA sCommand, CStringA &sResponse);
	bool ExecuteNonMenuCommand(CStringW sCommand, CStringW &sResponse);

	// Legacy, do not use for new designs. Use ExecuteMenuCommand instead.
	unsigned short SendMenuCommand(
		unsigned char *pInputBuffer,
		unsigned long nBytesInInputBuffer,
		unsigned char *pOutputBuffer,
		unsigned long nSizeOfOutputBuffer,
		unsigned long *pnBytesReturned);

	void SwitchToSyncronMode(void);
	void SwitchToAsyncronMode(void);

	CMessageDetails *ReadRxFifo(void);							//!< Reads a complete HTag out of the fifo
	int ReadAndParseMessage(void);
	int WriteCurrentPayloadToFile(CString sFileName);
	bool IsCurrentPayloadText(void)			{ return m_pCurrentMessage->IsText();		}
	bool IsCurrentPayloadCmdResponse(void)		{ return m_pCurrentMessage->IsCmdResponse();		}
	bool IsCurrentPayloadImage(void)			{ return m_pCurrentMessage->IsImage();	}
	CStringA GetRawPayloadData(void)			{ return m_pCurrentMessage->GetRawPayloadData();	}
	bool GetRawPayloadBuffer(char *&pData, size_t &Length)
			{ return m_pCurrentMessage->GetRawPayloadBuffer(pData, Length);	}

	bool GetRawPayloadBuffer(UCHAR *&pData, size_t &Length)
			{ return m_pCurrentMessage->GetRawPayloadBuffer((char*&)pData, Length);	}

	UCHAR GetAckByte(void)
			{ return m_pCurrentMessage->GetAckByte();	}

	int  GetImageFormat(void)					{ return m_pCurrentMessage->GetImageFormat();	}

	bool UseHTagHeader(bool newval);
	void ShowProgress(void);
	CMessageDetails::PayloadType_t ReadBarcode(CStringA &sAsyncMessage, bool bTrigger, bool bPurge, int timeout);
	CMessageDetails::PayloadType_t TriggerAndReadBarcode(CStringA &sAsyncMessage, int timeout=10000)
	{
		return ReadBarcode(sAsyncMessage, true, true, timeout);
	}

	bool isFirmwareReplace()	{ return m_fFirmwareReplace; }

    bool RetrieveVersionFromDevice(CStringA &VersionString);
    bool RetrieveSerialStringFromDevice(CStringA &SerialString);
    bool isSameVersion(CStringA &sNewDeviceVersion);
    void InitialCommunication();			 	// Try to setup device for a good communication
	unsigned int GetLastCmdError() { return m_LastCmdError; };

protected:

	// Sends the command with a menu header, wait for response
	bool ExecuteSyncronMenuCommand(CStringA sCommand, bool bOptional=false, bool ProdConfig=false);
	bool ExecuteSyncronMenuCommand(CStringW sCommand, bool bOptional=false, bool ProdConfig=false);
	// Sends the command with a menu header, wait for response
	bool ExecuteAsyncronMenuCommand(CStringA sCommand, bool bOptional=false, bool ProdConfig=false);
	bool ExecuteAsyncronMenuCommand(CStringW sCommand, bool bOptional=false, bool ProdConfig=false);

	bool ExecuteSyncronNonMenuCommand(CStringA sCommand, CStringA &sResponse);
	bool ExecuteAsyncronNonMenuCommand(CStringA sCommand, CStringA &sResponse);

	int CalculateExtraTimeout(CStringA sCommand);
	bool	WaitForAck(void);								//!< Wait until the device sends the Acknowledge
	bool	WaitForSyncronNonMenuResponse(void);				//!< Wait until the device sends the CR
	bool	WaitForSyncronCommandResponse(int ExtraTimeout=0, bool bOptional=false);	//!< Wait until the device sends the reponse
	bool	WaitForAsyncronCommandResponse(bool bOptional=false);
	bool	WaitForAsyncronMessage(int timeout=10000);
	bool	ParseCommandResponse(bool bOptional=false);
	void SetLastCmdError(unsigned int err)	{ m_LastCmdError = err; };
	void TemporallySwitchToSyncronMode(void);
	void RestoreAsyncronMode(void);
	bool PrepareHTagHeader();

    bool CleanupResponseString(const char *str, CStringA &Version);

    bool ReadDispatch(unsigned char* RxChar, size_t nLength);
	bool StatusDispatch(DWORD dwStatus, DWORD dwDetail);
	int ReadHModem(void);

	virtual bool PostStatusMessage(WPARAM wParam, LPARAM lParam=0);
    virtual bool PostDataMessage(WPARAM wParam=0, LPARAM lParam=0);

protected:
	bool m_fUtf16le;						//!< Flags that the device is set to UTF16LE
	bool m_fWantSynHeader;					//!< Flags that we want to to use the HTag header
	bool m_fUsesSynHeader;					//!< Flags that the device is set to use the HTag header
	bool m_fAsyncMode;						//!< Flags that we are in the Async mode
	bool m_fOutHdrMissing;					//!< Flags that the device does not support the OUTHDR command
    bool m_fFirmwareReplace;				//!< Flags shows FW replace is running
	CStringA m_sCommandResponse;			//!< Contains the response from the device
	unsigned int m_LastCmdError;			//!< Last command error
	CAsyncParser *m_pAsyncParser;			//!< Helper to parse received bytes
	CallbackRead_t *m_pReadDispatch;		//!< Callback helper for received data
	CallbackStatus_t *m_pStatusDispatch;	//!< Callback helper for status changes
	HWND	m_ClientHandle;					//!< Window handle to send a packet complete message
	int	m_ClientDataMessageNumber;			//!< WM_xxx for client messages
	int	m_ClientStatusMessageNumber;		//!< WM_xxx for client messages
	CMessageDetails *m_pCurrentMessage;
	CHModem *m_pHModem;						//!< Accessor for the HModem receiver
	HANDLE m_hCommandResponse;				//!< We received a command response
	HANDLE m_hAsyncMessage;					//!< We received a message
};

enum
{
	EV_PROGRESS = EV_USER1
};

}	// namespace HsmDeviceComm

//using namespace HsmDeviceComm;

