
#include "stdafx.h"
#include "TransmitFirmware.h"
#include "HsmErrorDefs.h"
#include "HsmProgress.h"
#include "svn_rev.h"

namespace HsmDeviceComm {

using namespace std;

static CTxFirmware *gpFlasher = NULL;

CTxFirmware *CreateFlasher()
{
	if (gpFlasher == NULL)
	{
		gpFlasher = new CTxFirmware;
	}
	return gpFlasher;
}

LIB_API void HsmCleanup()
{
	CTxFirmware *temp = gpFlasher;
	gpFlasher = NULL;
	delete temp;
}

///////////////////////////////////////////////////////////////////////////////
// Simple C-Style interface to the flasher using FW from a file.
/* This is a blocking call, it will not return until the firmware is replaced.
 @param [in] pFirmwareId	firmware ID (e.g. BI000612AAA)
 @param [in] pProgress		callback
 @param [in] pFilename		firmware file
 @param [in] pDevice		device name
 @param [in] baudrate		baudrate
 @return ERROR_SUCCES on success
*/
LIB_API int HsmReplaceFirmwareFile(const char* pFirmwareId, CallbackProgress_t pProgress, 
		const char* pFilename, const char* pDevice, int baudrate)
{
	int RetVal = Framework_Init();
	if (RetVal != ERROR_SUCCESS)
		return RetVal;

	RetVal = CreateFlasher()->SetFWDetails(pFirmwareId, pProgress, pFilename, pDevice, baudrate);
	if(RetVal==ERROR_SUCCESS)
		RetVal = CreateFlasher()->ReplaceFirmware();
	
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
// Simple C-Style interface to the flasher  using FW from a buffer in memory.
/* This is a blocking call, it will not return until the firmware is replaced.
 @param [in] pFirmwareId	firmware ID (e.g. BI000612AAA)
 @param [in] pProgress		callback
 @param [in] pBuffer		buffer with firmware
 @param [in] Size			size of firmware
 @param [in] pDevice		device name
 @param [in] baudrate		baudrate
 @return ERROR_SUCCES on success
*/
LIB_API int HsmReplaceFirmwareBuffer(const char* pFirmwareId, CallbackProgress_t pProgress, 
		const unsigned char *pBuffer, size_t Size, const char* pDevice, int baudrate)
{
	int RetVal = Framework_Init();
	if (RetVal != ERROR_SUCCESS)
		return RetVal;

	RetVal = CreateFlasher()->SetFWDetails(pFirmwareId, pProgress, pBuffer, Size, pDevice, baudrate);
	if(RetVal==ERROR_SUCCESS)
		RetVal = CreateFlasher()->ReplaceFirmware();
	
	return RetVal;
}

LIB_API const char *HsmGetFRLibVersion()
{
	return SVN_RevisionStr;
}

LIB_API const char *HsmGetCurrentDevice()
{
	const char *RetVal = "unknown";
	if (gpFlasher != NULL)
	{
		RetVal = gpFlasher->GetDeviceName();
	}

	return RetVal;
}

}	// namespace HsmDeviceComm
