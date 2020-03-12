/*=================================================================================
  validatedevice.h
//=================================================================================
   $Id: ValidateDevice.h 202 2019-01-31 12:13:16Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// validatedevice.h : header file
//

#pragma once

namespace HsmDeviceComm {

class CDeviceComm;
class CValidateFirmware;

///////////////////////////////////////////////////////////////////////////////
//! Class to read firmware related details from the device.
/*! Here we retrieve the maximum rom size and other similar parameters from the device.
	 We also compare whether the new firmware is compatible with the device.
*/
class CValidateDevice
{
public:
	CValidateDevice();

	size_t GetMaxRomSize(void)	const	{ return m_MaxRomSize;	} //!< readonly accessor
	UINT GetDeviceType(void) 	const	{ return m_DeviceType;	} //!< readonly accessor
	int  GetCompId(void) 		const	{ return m_CompId;		} //!< readonly accessor
	UINT GetAppMask(void)		const	{ return m_AppMask;		} //!< readonly accessor
	void AllowUnknownDevices(bool newval)	{ m_AllowUnknownDevices=newval;	}

	long IsFileCompatibleToDevice(CDeviceComm &Comm, CValidateFirmware &File);

protected:
	bool IsCompatibleDevice();
	long GetDeviceDetails (CDeviceComm &Comm);
	long VerifyFirmwareGen5 (CValidateFirmware &File);
	long VerifyFirmwareGen6 (CValidateFirmware &File);
	long VerifyFirmware (CValidateFirmware &File);

protected:
	size_t m_MaxRomSize;						//!< Maximum size of a new firmware
	UINT m_DeviceType;							//!< Device type to avoid flasing wrong firmware (used by Generation 4 and 5)
	int  m_CompId;								//!< Compatible Device ID to avoid flasing wrong firmware (used by Generation 6)
	UINT m_AppMask;								//!< Future expansion
	bool m_DeviceIsRunningBootCode;				//!< In case we need it later
	bool m_AllowUnknownDevices;
};

}