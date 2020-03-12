/*=================================================================================
  ThreadedDeviceCommands.cpp
//=================================================================================
   $Id: ThreadedDeviceCommands.cpp 298 2018-08-03 05:35:30Z Fauth, Dieter $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// ThreadedDeviceCommands.cpp : implementation file
//

#include "stdafx.h"
#include "ThreadedDeviceCommands.h"
#include "ValidateDevice.h"
#include "ValidateFirmware.h"
#include "HsmErrorDefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace HsmDeviceComm
{

#define ACK  0x06
#define NAK  0x15

///////////////////////////////////////////////////////////////////////////////
//! The constructor of the high level transmit and receive functions.
/*!
*/
CThreadedDeviceCommands::CThreadedDeviceCommands(HWND hNotifictionReceiver, DWORD MsgStatus)
	: CThreadedDeviceCommandsBase(hNotifictionReceiver, MsgStatus)
	, m_pFlashThread(NULL)
{
	// manual reset, initially reset
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ASSERT(m_hShutdownEvent != NULL);
}

///////////////////////////////////////////////////////////////////////////////
//! Destructor.
/*! The usual cleanup.
*/
CThreadedDeviceCommands::~CThreadedDeviceCommands()
{
}

///////////////////////////////////////////////////////////////////////////////
//! A cleanup call, must be called before the desctructor.
/*! Calling the CloseThread() in the destructor would fail becasue our vtable 
    for the virtual functions is already destroyed at this time. So it would cause a crash.
 @return bool true alwayse
*/
bool CThreadedDeviceCommands::Terminate(void)
{
	return CloseThread();
}

///////////////////////////////////////////////////////////////////////////////
//! Callback handler for showing progress.
/*! Here is is used to capture messages from the flash core.
@param phases_t ChangePhase
@param int Percent
@return true to abort, false to proceed
*/
bool CThreadedDeviceCommands::CallbackProgress(phases_t ChangePhase, int Percent)
{
	bool RetVal=false;
	PostStatusMessage(ChangePhase, Percent);
	return IsAborted();
}

///////////////////////////////////////////////////////////////////////////////
//! A "hook" to get information on an abort by the user.
/*! Overwritten here to get notified on abort.

@return true to abort, false to proceed
*/
bool CThreadedDeviceCommands::IsAborted(void)
{
	return (WaitForSingleObject(m_hShutdownEvent, 0) == WAIT_OBJECT_0);
}

///////////////////////////////////////////////////////////////////////////////
//! Checks whether the tread is stopped.
/*! It uses the m_hStoppedEvent that is maintained by the thread code itself.
 @return true for stopped, false for not stopped
*/
inline bool CThreadedDeviceCommands::IsThreadStopped(void)
{
	return (m_pFlashThread == NULL);
}

///////////////////////////////////////////////////////////////////////////////
//! Terminates the download thread if it runs.
/*! The m_hShutdownEvent will be set and then we wait until the thread is terminated.
	We will not return until the thread is terminated!
 @return true
*/
bool CThreadedDeviceCommands::CloseThread(void)
{
	if (!IsThreadStopped())
	{
		TRACE("Terminating HMmodem thread\n");
		delete m_pFlashThread;
		m_pFlashThread = NULL;
	}
	return true;	// success
}

///////////////////////////////////////////////////////////////////////////////
//! Creates a thread to run the download and waiting.
/*! We use 2 Events to communicate with the thread. This is required to avoid race conditions.
	 - m_hStoppedEvent	Set if the thread is stopped
	 - m_hShutdownEvent	Set to shutdown the tread

 @return Error value as defined by Results_t
*/

	long CThreadedDeviceCommands::ExecuteFirmwareFlashInThread(void)
	{
	    if (!IsThreadStopped())
		{
			CloseThread();
		}

		ResetEvent(m_hShutdownEvent);

		m_pFlashThread = new TThreadWorker<CThreadedDeviceCommands,ThreadCommands_t, None>
		        (this, &CThreadedDeviceCommands::ThreadProcFlasher);

		if (m_pFlashThread!=NULL)
        {
            m_fFirmwareReplace = true;
            m_pFlashThread->Start();
            m_pFlashThread->Run();
        }
        else
		{
			TRACE(_T("Download thread failed to be created\n"));
			return ERROR_THREAD_NOT_CREATED;
		}

		return ERROR_SUCCESS;
	}

	///////////////////////////////////////////////////////////////////////////////
	//! Creates a thread to set the device to boot mode.
	/*! We use 2 Events to communicate with the thread. This is required to avoid race conditions.
		 - m_hStoppedEvent	Set if the thread is stopped
		 - m_hShutdownEvent	Set to shutdown the tread

	 @return Error value as defined by Results_t
	*/
	long CThreadedDeviceCommands::ExecuteBringDeviceIntoBootModeThread(void)
	{
		if (!IsThreadStopped())
		{
			CloseThread();
		}

		ResetEvent(m_hShutdownEvent);

		m_pFlashThread = new TThreadWorker<CThreadedDeviceCommands, ThreadCommands_t, None>
			(this, &CThreadedDeviceCommands::ThreadConnectAndBringDeviceIntoBootMode);

		if (m_pFlashThread != NULL)
		{
			m_fFirmwareReplace = true;
			m_pFlashThread->Start();
			m_pFlashThread->Run();
		}
		else
		{
			TRACE(_T("Download thread failed to be created\n"));
			return ERROR_THREAD_NOT_CREATED;
		}

		return ERROR_SUCCESS;
	}

///////////////////////////////////////////////////////////////////////////////
//! This is the thread code that controls the download and flashing process.
/*!
 @return dummy, always NULL
*/
void *CThreadedDeviceCommands::ThreadProcFlasher(void)
{
	long RetVal = ERROR_PORT_NOT_OPEN;
	if (AutoConnect())
	{
		CallbackProgress(EV_Validating, ERROR_SUCCESS);
		RetVal = PrepareFlashing();
		if (RetVal == ERROR_SUCCESS)
		{
			CallbackProgress(EV_DownloadStarted, 0);
			ASSERT(m_pFirmware != NULL);
			if (m_pFirmware != NULL)
			{
				// Download
				RetVal = DownloadFirmware(*m_pFirmware);

				// Wait until finshed
				if (RetVal == ERROR_SUCCESS)
				{
					CallbackProgress(EV_DownloadFinished, RetVal);
					RetVal = WaitForDeviceToFinishFlashing();
				}
			}
		}
	}
    if (RetVal != ERROR_SUCCESS)    // ensure there is a last message
        CallbackProgress(EV_FlashFinished, RetVal);
	return NULL; // dummy value
}

///////////////////////////////////////////////////////////////////////////////
//! The high level door to the firmware flashing.
/*! Call this function to do a complete validation and download.
 @param Filename A CString with the filename of the firmware file
 @return Error value as defined by Results_t
*/
long CThreadedDeviceCommands::ExecuteFirmwareFlash(CString Filename)
{
	m_Size = 0;
	m_pBuffer = NULL;
	m_Filename = Filename;
	m_valid = true;

    return ExecuteFirmwareFlashInThread();
}

///////////////////////////////////////////////////////////////////////////////
//! The high level door to the firmware flashing.
/*! Call this function to do a complete validation and download.
 @param pBuffer A pointer to a buffer containing the firmware
 @param Size Size of firmware
 @return Error value as defined by Results_t
*/
long CThreadedDeviceCommands::ExecuteFirmwareFlash(const unsigned char *pBuffer, size_t Size)
{
    m_Size = Size;
    m_pBuffer = pBuffer;
    m_Filename = "";
    m_valid = true;

    return ExecuteFirmwareFlashInThread();
}

void *CThreadedDeviceCommands::ThreadConnectAndBringDeviceIntoBootMode(void)
{
	if (AutoConnect())
	{
		ThreadBringDeviceIntoBootMode();
	}
	m_fFirmwareReplace = false;
	return NULL; // dummy value
}

// Here we set the device into boot mode.
// The user must power cycle whilae we send out ESC characters.
// Send ESC until we get the boot message or we timeout
int CThreadedDeviceCommands::ThreadBringDeviceIntoBootMode(void)
{
	const size_t BufSize = 2000;
	const size_t Timeout = 60000;
	const size_t SleepTime = 10;
	// Each ESC takes about 85microseconds. For a robust operation even on slower machines we should ensure
	// that we send a block with a few milliseconds time.
	const char szEsc[] = "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b"	// each line is about 1 millisecond
		"\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b"
		"\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b"
		"\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b"
		"\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b"

		"\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b"
		"\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b"
		"\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b"
		"\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b"
		"\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b\x1b\x1b\x1b" "\x1b\x1b";
	const char szReset[] = "\x16n,RESET\r";

	int RetVal = ERROR_BOOTMODE_FAILED;
	UCHAR Buffer[BufSize + 2];
	int Received = 0;
	int NewReceived = 0;
	Buffer[0] = 0;

	// Below line helps debugging this flasher tool
	Write((const UCHAR*)szReset, sizeof(szReset) - 1);	// no need to unplug device

	CallbackProgress(EV_BringDeviceIntoBootMode, 0);

	for (int j = Timeout / SleepTime; j > 0; j--)	// 60 Seconds timeout
	{
		Write((const UCHAR*)szEsc, sizeof(szEsc) - 1);
		CallbackProgress(EV_Percent, 100 - ((j * 100) / (Timeout / SleepTime)));	// show progress
		if (WaitForSingleObject(m_hShutdownEvent, 0) == WAIT_OBJECT_0)
		{
			RetVal = ERROR_CANCELED_BY_USER;
			break;
		}
		Sleep(SleepTime);

		int RestSize = BufSize - Received;
		if (RestSize < 1)	// give up?
		{
			break;
		}

		NewReceived = ReadBlock(Buffer + Received, RestSize);	// does not wait!
		if (NewReceived > 0)
		{
			int i;
			// remove garbage, do it backwards
			for (i = Received + NewReceived; i > Received; i--)
			{
				if ((Buffer[i] < 127) && isprint(Buffer[i]))
				{
					break;
				}
				NewReceived--;	// ignore this character
			}

			// ignore a series of same characters
			for (i = Received + NewReceived; i > Received + 4; i -= 4)
			{
				bool ignore = false;
				// ignore 4 equal characters
				if (Buffer[i] == Buffer[i - 1])
				{
					if (Buffer[i] == Buffer[i - 2])
					{
						if (Buffer[i] == Buffer[i - 3])
						{
							ignore = true;	// ignore these characters
						}
					}
				}
				if (ignore)
				{
					NewReceived -= 4;	// ignore these characters
				}
				else
				{
					break;	// keep the rest
				}
			}

			// remove potential \0
			for (i = Received; i < Received + NewReceived; i++)
			{
				if (!isprint(Buffer[i]))
				{
					Buffer[i] = ' ';
				}
			}

			Received += NewReceived;
			Buffer[Received + 1] = 0;	// ensure a zero at the end

			if ((NULL != strstr((const char*)Buffer, "Boot"))
				|| (NULL != strstr((const char*)Buffer, "pplication"))
				|| (NULL != strstr((const char*)Buffer, "Skipping ")))
			{
				if (NULL != strstr((const char*)Buffer, "..."))
				{
					RetVal = ERROR_SUCCESS;
					break;
				}
			}
		}
	}
	Sleep(500);
	PurgeReceive();

	CallbackProgress(EV_BootMode, RetVal);

	return RetVal;
}

#ifdef _MFC_VER
bool CThreadedDeviceCommands::AutoConnect(void)
{
	bool connected = true;
	if (!IsConnected())
	{
		connected = CThreadedDeviceCommandsBase::Connect();		// start to communicate
		CallbackProgress(EV_ConnectionStatus, (LPARAM)connected);
	}
	return connected;
}

bool CThreadedDeviceCommands::DisConnect(void)
{
	bool RetVal = true;
	if (IsConnected())
	{
		RetVal = CThreadedDeviceCommandsBase::DisConnect();
	}
	return RetVal;
}
#else
bool CThreadedDeviceCommands::AutoConnect(void)
{
	return true;
}
bool CThreadedDeviceCommands::DisConnect(void)
{
	return true;
}
#endif

} // namespace HsmDeviceComm
