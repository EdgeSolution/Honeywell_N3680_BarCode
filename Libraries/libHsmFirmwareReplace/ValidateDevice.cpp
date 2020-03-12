/*=================================================================================
  validatedevice.cpp
//=================================================================================
   $Id: ValidateDevice.cpp 202 2019-01-31 12:13:16Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// validatedevice.cpp : implementation file
//

#include "stdafx.h"
#include "ValidateDevice.h"
#include "ValidateFirmware.h"
#include "HsmErrorDefs.h"
#include "DeviceComm.h"

namespace HsmDeviceComm {

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ACK  0x06

CValidateDevice::CValidateDevice()
: m_MaxRomSize(0)
, m_DeviceType(0)
, m_CompId(-1)
, m_AppMask(0)
, m_DeviceIsRunningBootCode(false)
, m_AllowUnknownDevices(false)
{
}

///////////////////////////////////////////////////////////////////////////////
//! Retrieve the device details from the device.
/*! It uses the CDeviceCommands object to be more portable.
	There are different devices with varous behaviors. 
	Gen5 devices (4600, ..) repond to the DEVICETYPE command.
	GEN6 devices (Xenon, ..) respond APP,0 (== not valid) to the DEVICETYPE command.
	These devices use the COMPID command to provide details about the device.
 @param &Comm Communication object
 @return Error value as defined by Results_t (ERROR_SUCCESS for OK)
*/
long CValidateDevice::GetDeviceDetails(CDeviceComm &Comm)
{
	long RetVal = ERROR_COMMUNICATION;
	CStringA sResponse;

	// Turn certain protocolls off, they could disturb our communication
	Comm.InitialCommunication();

	// A retry for the next step makes it more robust if the device has certain debug messages enabled.
	// In normal setups we will leave the loop after the first try.
	for(int j=0; j<5; j++)
	{
		Comm.ExecuteNonMenuCommand("DEVICETYPE", sResponse);
		TRACE("DEVICETYPE=%s\r", (const char*)sResponse);
		if (sscanf_s(sResponse, "APP,%d", &m_DeviceType) == 1)
		{
			RetVal = ERROR_SUCCESS;
		}
		else if (sscanf_s(sResponse, "BOOT,%d", &m_DeviceType) == 1)
		{
			m_DeviceIsRunningBootCode = true;	// just in case we need it later
			RetVal = ERROR_SUCCESS;
		}
		if (RetVal == ERROR_SUCCESS)	
			break;

		Sleep(500);	// give some time for extra debug messages
	}

	if((RetVal == ERROR_SUCCESS) && (m_DeviceType==0))	
	{
		// Device responded with APP,0, this is could mean it is a Generation 6 device.
		// So lets try the COMPID command.
		// Could also be a bad flash chip or more likely a bad firmware. You can try with boot mode in this case.
		for(int j=0; j<5; j++)
		{
			RetVal = ERROR_COMMUNICATION;
			if (Comm.ExecuteMenuCommand("COMPID?.", sResponse))
			{
				if (sscanf_s(sResponse, "COMPID%d", &m_CompId) == 1)
				{
					RetVal = ERROR_SUCCESS;
					TRACE(_T("CompId=%x\n"), m_CompId);
				}
			}
			if (RetVal == ERROR_SUCCESS)	
				break;

			Sleep(500);	// give some time for extra debug messages
		}
	}

	if(RetVal == ERROR_SUCCESS)
	{
		RetVal = ERROR_COMMUNICATION;
		if (Comm.ExecuteMenuCommand("MSZAPP?.", sResponse))
		{
			if (sscanf_s(sResponse, "MSZAPP%ld", &m_MaxRomSize) == 1)
			{
				RetVal = ERROR_SUCCESS;
			}
		}
		else	// Older FW does not support this command yet, so assume we are ok.
		{		// The device will complain if this assumption is wrong.
			m_MaxRomSize=917504;	
			RetVal = ERROR_SUCCESS;
		}
	}
#ifdef USE_APPMSK	// for later
	bool CommAppMask=false;
	if (Comm.ExecuteMenuCommand("APPMSK?.", sResponse))
	{
		const int AppMaskStringSize = 6;
		const int MinAppMaskResponseSize = 6+8+1+1;
		if(sResponse.GetLength() >= MinAppMaskResponseSize)
		{
			CommAppMask=true;
			// we need to read 8 binary bits represented as '0' or '1'.
			// Scanf does not support binary.
			int mask=0x1;
			for (int i=MinAppMaskResponseSize-1; i>AppMaskStringSize; i--)	// start at the last digit
			{
				if (sResponse[i] == '1')
				{
					m_AppMask |= mask;
					mask = mask<<1;
				}
				else if (sResponse[i] == '0')
				{
					mask = mask<<1;
				}
			}
		}
	}
#endif	// USE_APPMSK

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Verify device and firmware for gen5 devices.
/*! Called by VerifyFirmware.
 @param &File Firmware object
 @return Error value as defined by Results_t (ERROR_SUCCESS for OK)
*/
long CValidateDevice::VerifyFirmwareGen5(CValidateFirmware &File)
{
	long RetVal=ERROR_WRONG_FW;

// First we check whether the flag for the devicetype is set in the firmware file.
	// The type is an index into the flag bits, so it has to be smaller than the number of flagsbits.
	if (GetDeviceType() <= (File.GetNumMaskFlags() * 8))
	{
		// Get the address of the byte containing the flag for this device
		const UCHAR *pMaskAddr = File.GetMaskFlags() + ((GetDeviceType() -1) / 8);
		// Get the position of the flag
		UCHAR BitPosition = (UCHAR) (1 << ((GetDeviceType() - 1) % 8));
		// Now get the flag itself
		bool Flag = (*pMaskAddr & BitPosition) ? true : false;
		if(Flag)
		{
			// If we've succeeded this far, then the code matches the device
			// We could do a few more detailed tests with m_AppMask, but for now we are happy with the result.

			// Check whether the flashrom is big enough
			if (File.GetFirmwareSize() < GetMaxRomSize())
			{
				RetVal = ERROR_SUCCESS;
			}
			else
			{
				RetVal = ERROR_FILE_TO_BIG;
			}
		}
	}

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! List of known gen6 devices.
static const int CompatibleID[] =
{
	0,		// Gen6 Boot Firmware 
	1,		// Gen6 Corded Scanner Application Firmware 
	2,		// Gen6 Cordless Scanner Application Firmware 
	3,		// Gen6 Cordless Base Application Firmware 
	4,		// Vuquest 3310 Scanner Application Firmware
	5,		// Xenon Lite 1500 Boot Firmware
	6,		// Xenon Lite 1500 Application Firmware
	7,		// Vuquest 3310 Scanner Boot Firmware

	8,		// 7580RD boot
	9,		// 7580RD Application

	11,	// 0x0b	Andaman 1980i Boot code
	12,	// 0x0c	Andaman 1980i Application Firmware
	13,	// 0x0d	Andaman 1981i Boot code
	14,	// 0x0e	Andaman 1981i Application Firmware

	15,	// RF base boot
	16,	// RF base Application

	0x19,	// RF base 1452 boot
	0x1A,	// RF base 1452 Application

	0x40,	// 1300g Boot
	0x41,	// 1300g Application

	0x80,	// N8680 Boot
	0x81,	// N8680 Application

	0x82,	// N4850 Application

	0xa0,	// Voyager 2D 1400 Boot Firmware
	0xa1,	// Voyager 2D 1400 Application Firmware

	0xE0,	// Youjie 2D 4600 Boot Firmware
	0xE1,	// Youjie 2D 4600 Application Firmware

	0xE5,	// HF500

	0x120,// Voyager 145X Boot Firmware
	0x121,// Voyager 1450 Application Firmware
	0x121,// Voyager 1452 Application Firmware

	0x123,// 1450g2DHR Application Firmware
	0x130,// 1450g2DHR2 Application Firmware

	0x140,// N3680 Boot Firmware
	0x141,// N3680 Application Firmware

	0x131,

	0x132,

	0x142,// HP2D Boot Firmware (HP N3680)
	0x143,// HP2D Application Firmware (HP N3680)
	0x148,// HP2D Boot Firmware (HP N3680 - Flash EOL)
	0x149,// HP2D Application Firmware (HP N3680 - Flash EOL)

	0x174,
	0x175,

	0x160,
	0x161,

	0x186,
	0x187,
	0x405,
	0x406,
	0x407,
	0x408,

	-1,		// end of list
};

///////////////////////////////////////////////////////////////////////////////
//! Check whether the device is known.
/*! You can define RELAXED_DEVICE_CHECK for a much relaxed test.
	Then all positive COMPID responses will be accepted.
 @return true if device is in the list
*/
bool CValidateDevice::IsCompatibleDevice()
{
#ifdef RELAXED_DEVICE_CHECK	// older code only checked for a positive value.
	return GetCompId()>=0;
#else // RELAXED_DEVICE_CHECK
	if(m_AllowUnknownDevices)
	{
		return GetCompId()>=0;
	}
	else
	{
		for(const int *pTable=CompatibleID; *pTable>=0; pTable++)
		{
			if(GetCompId()==*pTable)
				return true;
		}
	}
#endif // RELAXED_DEVICE_CHECK
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//! Verify device and firmware for gen6 devices.
/*! Called by VerifyFirmware.
 @param &File currently unused
 @return Error value as defined by Results_t (ERROR_SUCCESS for OK)
*/
long CValidateDevice::VerifyFirmwareGen6(CValidateFirmware &/*File*/)
{
	long RetVal=ERROR_DEVICETYPE;
	// Check whether the device is known.
	if(IsCompatibleDevice())
	{
		RetVal=ERROR_SUCCESS;
	}
	// Here you could add some custom checks for the firmware file.

	// Currently we do not check the firmware file itself. This will be done inside the device.
	// Though a test here might save some time (download time).
	// However the .moc format is not available for public.

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Verify device and firmware devices.
/*! 
 @param &File Firmware object
 @return Error value as defined by Results_t (ERROR_SUCCESS for OK)
*/
long CValidateDevice::VerifyFirmware(CValidateFirmware &File)
{
	long RetVal=ERROR_WRONG_FW;

	if (GetDeviceType() > 0)
	{
		RetVal = VerifyFirmwareGen5(File);
	}
	else if (GetCompId() >= 0)
	{
		RetVal = VerifyFirmwareGen6(File);
	}

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Retrieve device details and compare whether is it compatible to the new firmware.
/*! The do it all function of this class.
 @param &Comm Communication object
 @param &File Firmware object
 @return Error value as defined by Results_t (ERROR_SUCCESS for OK)
*/
long CValidateDevice::IsFileCompatibleToDevice(CDeviceComm &Comm, CValidateFirmware &File)
{
	long RetVal=GetDeviceDetails(Comm);
	if (RetVal == ERROR_SUCCESS)
	{
		RetVal = VerifyFirmware(File);
	}

	return RetVal;
}

}	// namespace HsmDeviceComm
