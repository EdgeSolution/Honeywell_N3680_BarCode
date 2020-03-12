/*=================================================================================
  TransmitFirmware.cpp
//=================================================================================
   $RCSfile: CommonFirmwareReplace/TransmitFirmware.cpp $
   $Author: Fauth, Dieter $
   $Date: 2018-07-10 19:03:44 +0200 (Tue, 10 Jul 2018) $
   $Revision: 293 $

Copyright (C) Hand Held Products, Inc. 2006
//=================================================================================*/
//! \file
// TransmitFirmware.cpp : implementation file
//
// useful info:
// http://www.easysw.com/~mike/serial/serial.html
//
#include "stdafx.h"
#include "TransmitFirmware.h"
#include <dirent.h>

///////////////////////////////////////////////////////////////////////////////
//! The constructor of the connector to the low level transmit and receive functions.
/*! Here we use the dfusblib, but other libs are possible as well.
	The low level communication object (m_pComm) is created here.
 \param hClientWindow Window handle to a window we send notifications.
 \param MessageNumberStatus Command ID so the receiver of the notifications can parse our messages.
*/
CTxFirmware::CTxFirmware()
: m_fd(-1), m_ReadTimeout(1500),
m_DevicePathName(""), m_DevicePath(""), m_DeviceName(""), m_WeSawUsbDisconnect(false)
{
}

///////////////////////////////////////////////////////////////////////////////
//! Destructor.
/*! Deletes the low level communication object.
*/
CTxFirmware::~CTxFirmware()
{
	DisConnect();
}

///////////////////////////////////////////////////////////////////////////////
//! Remove all received data from buffers.
/*! If you do not implement this, then the base class uses Reads to empty the buffers.
*/
void CTxFirmware::PurgeReceive(void)
{
	AssertValid();
	tcflush(m_fd, TCIFLUSH);
}

///////////////////////////////////////////////////////////////////////////////
//! Initialize the low level communication object.
/*! Called before the XModem transfer. Here we just set the timeout values for the Read.
	Baudrate etc. are handled long before this time.
*/
void CTxFirmware::InitRxTx(void)
{
	AssertValid();
	// noting to do for POSIX
}

///////////////////////////////////////////////////////////////////////////////
//! Read a byte from the port.
/*! The low level Read is setup to wait up to 500mSec for a byte.
 \return Received byte or -1 if there was nothing received (timeout).
*/
int CTxFirmware::Read(void)
{
	AssertValid();
	UCHAR Received;
	if(0 == read (m_fd, &Received, 1))
	{
		return -1;
	}
	return Received;

}

///////////////////////////////////////////////////////////////////////////////
//! Call the low level write function.
/*! This overload sends a single byte.
 \param byte The Data to send.
*/
void CTxFirmware::Write(UCHAR byte)
{
	AssertValid();
	ssize_t sent = write(m_fd, &byte, 1);
	// sent is merely there to remove the unused return warning
}


///////////////////////////////////////////////////////////////////////////////
//! Call the low level write function.
/*! This overload sends a binary block.
 \param *pByte	Point to the datablock
 \param Size	The size of the datablock
*/
void CTxFirmware::Write(const UCHAR *pByte, size_t Size)
{
	AssertValid();
	ssize_t sent = write(m_fd, pByte, Size);
	// sent is merely there to remove the unused return warning
}

///////////////////////////////////////////////////////////////////////////////
//! Check whether we are connected to the USB device.
/*! This implementantion just calls the function of the communication lib.
 The lib handles quite some nasty details for us.
 Doing it here would require to create a hidden window, registering the WM_DEVICECHANGE message
 and in the handler if it to check wheter our device is disconnected.

 For non_USB environments, this function can simply return true.
 \return true if connected, else false
*/
bool CTxFirmware::IsUSBConnected(void)
{
	if (m_fd < 0)
	{
		bool Visible = FindOurDevice();
		if(m_WeSawUsbDisconnect)
		{
			if(Visible)
			{
				ReConnect();
			}
		}
		else
		{
			if(!Visible)
			{
				m_WeSawUsbDisconnect = true;
			}
		}
	}

	return (m_fd >= 0);
}

///////////////////////////////////////////////////////////////////////////////
//! Prepare for the potential disconnect of USB devices.
/*! You might want to close the handle or file descriptor if the communication lib
		does not do it automatically.
*/
void CTxFirmware::PrepareDisconnect(void)
{
	// Only if we disconnect the device file, the kernel can remove and add the same device again.
	// Later the IsUSBConnected will reconnect if the device file is visible again.
	AssertValid();
	if(m_DeviceName.Find("ttyACM") >= 0)	// only do it for USB devices, not for regular RS232
	{
		if (m_fd >= 0)
		{
			tcflush(m_fd, TCIOFLUSH);
			close(m_fd);
			m_fd = -1;
			m_WeSawUsbDisconnect = false;
		}
	}
}

const int OpenFlags = O_RDWR | O_NOCTTY | O_NDELAY;

bool CTxFirmware::ReConnect(void)
{
	m_fd = open(m_DevicePathName, OpenFlags);
	if (m_fd >= 0)
	{
		AssertValid();
		tcsetattr (m_fd, TCSANOW, &m_options);
	}

	return (m_fd >= 0);
}

///////////////////////////////////////////////////////////////////////////////
//! Connects to the device with the curent port/device setting.
/*!
 \return true for success, else false.
*/
bool CTxFirmware::Connect(const char* szDevice, int baudrate)
{
	StoreNames(szDevice);
	m_fd = open(m_DevicePathName, OpenFlags);
	if (m_fd >= 0)
	{
		fcntl(m_fd, F_SETFL, 0);

		tcgetattr(m_fd, &m_oldtio);	// Get the current options for the port

		bzero(&m_options, sizeof(m_options)); // clear struct for new port settings
		// Set the baud rates
		int BaudConstant = GetBaudConstant(baudrate);
		cfsetispeed(&m_options, BaudConstant);
		cfsetospeed(&m_options, BaudConstant);

		//m_options.c_cflag &= ~(CSIZE|CSTOPB|PARENB|CRTSCTS);

		/*
		CS8     : 8n1 (8bit,no parity,1 stopbit)
      CLOCAL  : local connection, no modem contol
      CREAD   : enable receiving characters
		*/
		m_options.c_cflag |= CS8|CLOCAL|CREAD;

		/*
       IGNPAR  : ignore bytes with parity errors
       ICRNL   : map CR to NL
		*/
		//m_options.c_iflag &= ~(IGNPAR);
		m_options.c_iflag |= ICRNL;


		/*
			Raw output.
		*/
      //m_options.c_oflag = 0;

		m_options.c_cc[VTIME]    = 15;    // 1500 mSec timeout for read
		m_options.c_cc[VMIN]     = 0;     // just timeout

		tcsetattr (m_fd, TCSANOW, &m_options);
		tcflush(m_fd, TCIOFLUSH);
	}
	return (m_fd >= 0);
}

///////////////////////////////////////////////////////////////////////////////
//! Disconnects the port/device.
/*!
 \return true for success, else false.
*/
bool CTxFirmware::DisConnect(void)
{
	if (m_fd >= 0)
	{
		tcflush(m_fd, TCIOFLUSH);
		tcsetattr(m_fd,TCSANOW,&m_oldtio);
		close(m_fd);
		m_fd = -1;
	}
	return true;
}



/**
	Helpers for GetBaudConstant.
*/
static const int BaudValue[]=	{  921600,	 460800,	230400,	 115200,	 57600,	 38400,	 19200,	 9600,	 0 };
static const int BaudBits[] =	{ B921600,	B460800,	B230400, B115200,	B57600,	B38400,	B19200,	B9600,	B115200 };

///////////////////////////////////////////////////////////////////////////////
//! Translate a value based bautrate into a constant used by POSIX.
/*! Any unknown baudrate gets translated into 115200.
 \param int baudrate
 \return int Baud constant
*/
int CTxFirmware::GetBaudConstant(int baudrate)
{
	int BaudConstant = B115200;
	int i=0;
	while(BaudValue[i] > 0)
	{
		if(BaudValue[i] == baudrate)
		{
			BaudConstant = BaudBits[i];
			break;
		}
		i++;
	}
	return BaudConstant;
}

///////////////////////////////////////////////////////////////////////////////
//! Split and store the parts of a unix device name.
/*! The last part (ttyACM1) goes into m_DeviceName, the first part (/dev) into m_DevicePath.
 \param CString DevicePathName
*/
void CTxFirmware::StoreNames(const char* szDevice)
{
	m_DevicePathName = szDevice;
	int LastSlash = m_DevicePathName.ReverseFind('/');
	m_DeviceName = m_DevicePathName.Mid(LastSlash+1);
	m_DevicePath = m_DevicePathName.Left(LastSlash);
}

///////////////////////////////////////////////////////////////////////////////
//! Check whether our device is visible.
/*!
 \return bool true if our device is visible
*/
bool CTxFirmware::FindOurDevice(void)
{
	dirent *entry( NULL );
	DIR *dir = opendir( m_DevicePath.c_str() );
	if ( dir != NULL )
	{
		do
		{
			entry = readdir( dir );
			if ( entry != NULL )
			{
				if(m_DeviceName == entry->d_name)
				{
					break;
				}
			}
		}
		while ( entry!= NULL );

		if ( closedir( dir ) )
		{
			// closedir failed
			// what are we ging to do?
		}
	}
	return ( entry!= NULL );
}

bool CTxFirmware::CheckForNewDevice(bool bForce)
{
	bool retVal = false;
// Until we implement the Device Tracer on Linux as well.
//	if(m_pTracer != NULL)
//		retVal = m_pTracer->CheckForNewDevice(bForce);
	return retVal;
}
