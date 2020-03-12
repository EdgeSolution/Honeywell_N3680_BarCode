/*=================================================================================
  ValidateFirmware.h
//=================================================================================
   $Id: ValidateFirmware.h 202 2019-01-31 12:13:16Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// ValidateFirmware.h : header file
//
#pragma once

namespace HsmDeviceComm {

///////////////////////////////////////////////////////////////////////////////
//! A class to read and validate the firmware file.
/*! We store the firmware image here until we send it to the device.
	It uses the CFile for accessing the file on disk, later this might be
 	replaced with lower level but protable API calls.

*/
class CValidateFirmware
{
public:
	CValidateFirmware();
	~CValidateFirmware();
	long LoadFile(CString Filename);
	long LoadBuffer(const UCHAR *pBuffer, size_t Size);

	size_t GetFirmwareSize(void)		const { return m_FirmwareSize;	}
	const UCHAR *GetFirmware(void)		const { return (const UCHAR *)m_pBuffer;	}
	size_t GetNumMaskFlags(void)		const { return m_NumMaskFlags;	}
	const UCHAR *GetMaskFlags(void)		const { return m_pMaskFlags;	}

protected:
	void CleanBuffers();
	long CreateFreshBuffer(size_t Size);

	long ParseLoadedFileGen5(void);
	long ParseLoadedFileGen6(void);
	long ParseLoadedFile(void);

protected:
	char *m_pFileBuffer;							//!< The buffer for storing the firmware image (gets deleted in destructor)
	const char *m_pBuffer;							//!< Pointer to firmware image (no delete, just pointer to access buffer)
	size_t m_FirmwareSize;							//!< The size of the firmware image
	size_t m_NumMaskFlags;							//!< Amount of flag bytes in the firmware
	const UCHAR *m_pMaskFlags;						//!< Point to the flag bytes in the firmware image
};

}
