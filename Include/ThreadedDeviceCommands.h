/*=================================================================================
  ThreadedDeviceCommands.h
//=================================================================================
   $Id: ThreadedDeviceCommands.h 298 2018-08-03 05:35:30Z Fauth, Dieter $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// ThreadedDeviceCommands.h : header file
//

#pragma once

#include "HsmProgress.h"
#include "TransmitFirmware.h"
#include "ThreadWorker.h"

namespace HsmDeviceComm
{

    class CValidateFirmware;

	///////////////////////////////////////////////////////////////////////////////
	//! This is our high level communication.
	/*! It uses a thread to keep the GUI responsive and allows us to send progress notifications to the GUI.
		 Instanciate it in one of your GUI classes (CFirmwareReplaceDlg) and pass the window handle plus a message ID.
	 \code
		m_pDevice = new CTxFirmware(m_hWnd, WM_DEVICE_MSG_COMMSTATUS);
	 \endcode

		Then you might want to setup and connect the serial line:
	 \code
		m_pDevice->ReadIni(szCommParamters);		// setup the communications parameters from last run
		m_pDevice->Connect();							// start to communicate
		m_PortStatus = m_pDevice->GetCurrentDisplayName();
	 \endcode

		Later call ExecuteFirmwareFlash() to start the download and flashing process.
	 \code
		long RetVal = m_pDevice->ExecuteFirmwareFlash(m_filename);
	 \endcode

		Do not forget to clean up:
	 \code
		delete m_pDevice;	// remove the device interface
		m_pDevice = NULL;
	 \endcode

		There is even a configuration dialog in dfusblib.
	 \code
		m_pDevice->OnOptions();
		m_PortStatus = m_pDevice->GetCurrentDisplayName();
	 \endcode

	*/
    #define CThreadedDeviceCommandsBase CTxFirmware

    class CThreadedDeviceCommands : public CThreadedDeviceCommandsBase
    {
    public:
        CThreadedDeviceCommands(HWND hNotifictionReceiver = NULL, DWORD MsgStatus = 0);
        ~CThreadedDeviceCommands();
		bool AutoConnect(void);
		bool DisConnect(void);
		bool Terminate(void);
        long ExecuteFirmwareFlash(CString Filename);
        long ExecuteFirmwareFlash(const unsigned char *pBuffer, size_t Size);
        void AbortFirmwareFlash(void)
        { SetEvent(m_hShutdownEvent); }
		long ExecuteBringDeviceIntoBootModeThread(void);

    protected:
        virtual bool IsAborted(void);
        virtual bool CallbackProgress(phases_t Phase, int Percent);
		long ExecuteFirmwareFlashInThread(void);

        bool IsThreadStopped(void);
        bool CloseThread(void);
        void *ThreadProcFlasher(void);

		void *ThreadConnectAndBringDeviceIntoBootMode(void);
		int ThreadBringDeviceIntoBootMode(void);

        HANDLE m_hShutdownEvent;                        //!< Used to shutdown the thread
	private:
        enum ThreadCommands_t
        {
            None, Stop, Start,
        };
        TThreadWorker<CThreadedDeviceCommands, ThreadCommands_t, None> *m_pFlashThread;
    };

} // namespace HsmDeviceComm