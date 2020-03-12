
#pragma once

#include "HsmErrorDefs.h"
#include "HsmProgress.h"

#ifndef LIB_API
#define LIB_API extern "C"
#endif

namespace HsmDeviceComm {

///////////////////////////////////////////////////////////////////////////////
//! Simple C-Style interface to the flasher using FW from a file.
/*! This is a blocking call, it will not return until the firmware is replaced.
 @param [in] pFirmwareId	firmware ID (e.g. BI000612AAA)
 @param [in] pProgress		callback
 @param [in] pFilename		firmware file
 @param [in] pDevice		device name
 @param [in] baudrate		baudrate
 @return ERROR_SUCCES on success
*/
LIB_API int HsmReplaceFirmwareFile(const char* pFirmwareId, CallbackProgress_t pProgress,
		const char* pFilename, const char* pDevice, int baudrate);

///////////////////////////////////////////////////////////////////////////////
//! Simple C-Style interface to the flasher  using FW from a buffer in memory.
/*! This is a blocking call, it will not return until the firmware is replaced.
 @param [in] pFirmwareId	firmware ID (e.g. BI000612AAA)
 @param [in] pProgress		callback
 @param [in] pBuffer		buffer with firmware
 @param [in] Size			size of firmware
 @param [in] pDevice		device name
 @param [in] baudrate		baudrate
 @return ERROR_SUCCES on success
*/
LIB_API int HsmReplaceFirmwareBuffer(const char* pFirmwareId, CallbackProgress_t pProgress,
		const unsigned char *pBuffer, size_t Size, const char* pDevice, int baudrate);

///////////////////////////////////////////////////////////////////////////////
//! Retrieve version of library.
/*!
 @return Pointer to version string.
*/
LIB_API const char *HsmGetFRLibVersion();

#define HsmGetRFVersion HsmGetFRLibVersion

///////////////////////////////////////////////////////////////////////////////
//! Retrieve error text.
/*!
 @param [in] error		error number
 @param [out] pText		pointer to text
 @return true if text was found, else false.
*/
LIB_API bool HsmGetErrorText(unsigned int error, const char **pText);

///////////////////////////////////////////////////////////////////////////////
//! Configure device.
/*! Parses the file pFileName and executes the commands to the device.
 @param [in] pFileName		config file
 @param [in] pDeviceName	device name
 @param [in] baudrate		baudrate
 @return ERROR_SUCCES on success
* */
LIB_API int HsmConfigureDevice(const char *pFileName, const char * pDeviceName, int baudrate);

///////////////////////////////////////////////////////////////////////////////
//! Configure device.
/*! Parses the file pFileName and executes the commands to the device.
@param [in] pFileName		config file
@param [in] pProgress		callback
@param [in] pDeviceName	device name
@param [in] baudrate		baudrate
@return ERROR_SUCCES on success
* */
LIB_API int HsmConfigureDevice2(const char *pFileName, CallbackProgress_t pProgress, const char * pDeviceName, int baudrate);

///////////////////////////////////////////////////////////////////////////////
//! Retrieve current device name.
/*! This function must only be called form a progress callback.
	It esures that the flash object are valid.
	The returned string is must be stored to another place before leaving the callback.
@return Device interface (port) as string.
*/
LIB_API const char *HsmGetCurrentDevice();

///////////////////////////////////////////////////////////////////////////////
//! Free all objects
/*!
@return Device interface (port) as string.
*/
LIB_API void HsmCleanup();

}	// namespace HsmDeviceComm
