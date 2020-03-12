/*=================================================================================
  TransmitFirmware.h
//=================================================================================
   $RCSfile: CommonFirmwareReplace/TransmitFirmware.h $
   $Author: Fauth, Dieter $
   $Date: 2017-10-04 18:48:09 +0200 (Wed, 04 Oct 2017) $
   $Revision: 264 $

Copyright (C) Hand Held Products, Inc. 2006
//=================================================================================*/
//! \file
// TransmitFirmware.h : header file
//

#ifndef _CTXFIRMWARE_FW_H_
#define _CTXFIRMWARE_FW_H_

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include "DeviceFirmware.h"


///////////////////////////////////////////////////////////////////////////////
//! Connects the device communication to a low level communication class.
/*!
	Here we use the dfusblib as we do in the other Adaptus ESC.
	We overwrite virtual functions declared in CXModem and call into the dfusblib.
	This is a place you most likely need to change if you port to another plattform.
*/
class CTxFirmware : public CDeviceFirmware
{
public:
	CTxFirmware();
	~CTxFirmware();

	bool Connect(const char* szDevice, int baudrate);
	bool DisConnect(void);

protected:
	virtual void PrepareDisconnect(void);
	virtual bool IsUSBConnected(void);
	virtual void PurgeReceive(void);
	virtual void InitRxTx(void);
	virtual int	 Read(void);
	virtual void Write(UCHAR byte);
	virtual void Write(const UCHAR *pByte, size_t Size);
	virtual bool CheckForNewDevice(bool bForce=false);

// helper functions
protected:
	int GetBaudConstant(int baudrate);
	bool ReConnect(void);
	void StoreNames(const char* szDevice);
	bool FindOurDevice(void);

	void AssertValid(void)
	{
		ASSERT(m_fd >= 0);
	}


	// member variables
protected:
	int m_fd;	//!< File descriptor for the port
	int m_ReadTimeout;
	struct termios m_options;
	struct termios m_oldtio;
	CString m_DevicePathName;
	CString m_DevicePath;
	CString m_DeviceName;
	bool m_WeSawUsbDisconnect;
};

#endif
