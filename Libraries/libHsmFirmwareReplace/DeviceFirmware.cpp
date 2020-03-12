
#include "stdafx.h"
#include "ValidateFirmware.h"
#include "ValidateDevice.h"
#include "HsmErrorDefs.h"
#include "HsmProgress.h"
#include "DeviceFirmware.h"
#include "crc16.h"

namespace HsmDeviceComm {

using namespace std;

#ifndef _
#define _(x)    x
#endif

#define ACK  0x06
#define NAK  0x15


///////////////////////////////////////////////////////////////////////////////
//! Constructor.
/*!
*/
CDeviceFirmware::CDeviceFirmware(HWND hNotifictionReceiver, DWORD MsgStatus)
: CDeviceFirmwareBase(hNotifictionReceiver, 0, MsgStatus)
, m_pProgress(NULL)
, m_pFirmware(NULL)
, m_pFirmwareId(NULL)
, m_pBuffer(NULL)
, m_Size(0)
, m_baudrate(0)
, m_valid(false)
, m_DeviceType(0)
, m_CompId(0)
, m_AllowHtag(false)
, m_AllowUnknownDevices(false)
{
}

///////////////////////////////////////////////////////////////////////////////
//! Destructor.
/*! The usual cleanup.
*/
CDeviceFirmware::~CDeviceFirmware()
{
	delete m_pFirmware;
	m_pFirmware = NULL;
}

// Wire the Xmodem functions to the real communication class
// Required while we use mulitple inheritance and same names would be unambiguous
void CDeviceFirmware::XM_InitRxTx()
{
	InitRxTx();
}

void CDeviceFirmware::XM_PurgeReceive()
{
	PurgeReceive();
}

int CDeviceFirmware::XM_Read(void)
{
	return Read();
}

void CDeviceFirmware::XM_Write(UCHAR byte)
{
	Write(byte);
}

void CDeviceFirmware::XM_Write(const UCHAR *pByte, size_t Size)
{
	Write(pByte, Size);
}


///////////////////////////////////////////////////////////////////////////////
//! A "hook" to get information on an abort by the user.
/*! Overwritten here to get notified on abort.

@return true to abort, false to proceed
*/
bool CDeviceFirmware::IsAborted(void)
{
	return false;
}

const int DownloadPercentage	= 75;					//!< Amount of progess bar are for the download
const int FlashPercentage		= 25;					//!< Amount of progess bar are for the flashing

///////////////////////////////////////////////////////////////////////////////
//! A "hook" to get progress information.
/*! Overwritten here to get notified on progress of downloading.
	See the class CXModem for details.

 @param BytesSend Number of bytes send so far
 @return true to abort, false to proceed
*/
bool CDeviceFirmware::ShowWriteProgress(size_t BytesSend)
{
	ASSERT(m_pFirmware!=NULL);
	// threat the download as xx% of the overall time
	size_t percent = (BytesSend*DownloadPercentage)/m_pFirmware->GetFirmwareSize();
	return CallbackProgress(EV_Downloading, percent);
}

///////////////////////////////////////////////////////////////////////////////
//! Callback handler for showing progress.
/*! 
 @param phases_t ChangePhase 
 @param int Percent 
 @return true to abort, false to proceed
*/
bool CDeviceFirmware::CallbackProgress(phases_t ChangePhase, int Percent)
{
	bool RetVal=false;
	if(m_pProgress!=NULL)
	{
		RetVal = m_pProgress(ChangePhase,Percent);
	}
	return RetVal;
}

//! Called before we try to do any firmware update.
/*! Use it to cleanup every old data you might have. */
void CDeviceFirmware::CleanupBeforeFlashing(void)
{
	m_sOldDeviceVersion.Empty();
	m_sNewDeviceVersion.Empty();
	m_sOldSerial.Empty();
	m_sNewSerial.Empty();
	delete m_pFirmware;							// remove old firmware in case we had one open
	m_pFirmware = new CValidateFirmware;
}

///////////////////////////////////////////////////////////////////////////////
//! Call it to retrieve the old firmware version.
/*! Be sure to call at the right time. It will be valid after you called ExecuteFirmwareFlash().
	So the best is to call it from the progress callback and the DownloadStarted event.
  This will also ensure you do not get mixed outputs due to race condition between the threads.
 @return Version string
*/
CString CDeviceFirmware::GetOldDeviceVersion(void)
{
	return (CString) m_sOldDeviceVersion;
}

///////////////////////////////////////////////////////////////////////////////
//! Call it to retrieve the new firmware version.
/*! Be sure to call at the right time. It will be valid after you called ExecuteFirmwareFlash().
	So the best is to call it from the progress callback and the FlashFinished event.
  This will also ensure you do not get mixed outputs due to race condition between the threads.
 @return Version string
*/
CString CDeviceFirmware::GetNewDeviceVersion(void)
{
	return (CString) m_sNewDeviceVersion;
}

///////////////////////////////////////////////////////////////////////////////
//! Call it to retrieve the old serial string of the device.
/*! Be sure to call at the right time. It will be valid after you called ExecuteFirmwareFlash().
So the best is to call it from the status handler and the EV_DownloadStarted event.
This will also ensure you do not get mixed outputs due to race condition between the threads.
@return Version string
*/
CString CDeviceFirmware::GetOldSerial(void)
{
	return (CString)m_sOldSerial;
}

///////////////////////////////////////////////////////////////////////////////
//! Call it to retrieve the old serial string of the device.
/*! Be sure to call at the right time. It will be valid after you called ExecuteFirmwareFlash().
So the best is to call it from the status handler and the EV_FlashFinished event.
This will also ensure you do not get mixed outputs due to race condition between the threads.
@return Version string
*/
CString CDeviceFirmware::GetNewSerial(void)
{
	return (CString)m_sNewSerial;
}

///////////////////////////////////////////////////////////////////////////////
//! Call it to retrieve the error response text in case of some flash failures.
/*! The string could be empty. Only makes sense if the error code is ERROR_FLASH_WRONG_FW.
 	Be sure to call at the right time. It will be valid after you called ExecuteFirmwareFlash().
	So the best is to call it from the progress callback and the FlashFinished event.
	This will also ensure you do not get mixed outputs due to race condition between the threads.
 @return Error response from the device in case of a flash failure.
*/
CString CDeviceFirmware::GetFlashErrorResponse(void)
{
	return (CString) m_sFlashErrorResponse;
}

///////////////////////////////////////////////////////////////////////////////
//! Wait until the flashing process in the device is finished.
/*! After the device is up and running with the new firmware it is supposed to send us (once) an ACK
	to notify about the success. While this works for serial (RS232) connections, it does not reliably for
	the USB connections. Therefore we use the disconnect notification from the low level communication class to
	recognize when the device gets connected again.
*/

int CDeviceFirmware::WaitForDeviceToFinishFlashing(void)
{
	// now wait for the last ACK that shows the device is up and running again
	const unsigned int MaxFlashTime = 120 * 1000;	// in mSec
	bool bSentStillRunning = false;
	bool bWeSawUsbDisconnect = false;
	size_t OldPercent = 0;
	int RetVal = ERROR_FLASH_FAILED;
	m_sFlashErrorResponse.Empty();
	int Received;

	unsigned int time;
	unsigned long long Starttime = GetTickCount64();	// get current time
	PrepareDisconnect();		// let USB devices go
	do
	{
		if (IsUSBConnected())	// USB devices do a full disconnect
		{
			Received = Read();	// read with timeout of 500 mSec
			if (Received == ACK)
			{
				TRACE("Got ACK\n");
				RetVal = ERROR_SUCCESS;
				break;
			}
			else if (Received == NAK)
			{
				TRACE("Got NAK\n");
				RetVal = ERROR_FLASH_WRONG_FW;
				break;
			}
			else if (Received>0)
			{
				TRACE("Got ERROR\n");
				m_sFlashErrorResponse += (char)(Received & 0xFF);	// a debugging helper
			}
			else if (bWeSawUsbDisconnect)
			{
				// device was disconnected while flashing, now it is running again with new firmware
				TRACE("Usb Reconnect\n");
				RetVal = ERROR_SUCCESS;
				break;	// work fine even if the ACK from device is lost
			}
		}
		else
		{
			TRACE("Usb Disconnect\n");
			bWeSawUsbDisconnect = true;	// If the device will be connected later, then it must have been rebooted.
			Sleep(100);
			if (CheckForNewDevice())
			{
				// Looks like the COMx name hase been changed. Happens if the new FW has a different VID/PID.
				CallbackProgress(EV_DeviceChangedInterface, 0);
				RetVal = ERROR_SUCCESS;
				break;
			}
		}

		time = (UINT)(GetTickCount64() - Starttime);	// time we are waiting already
											// show an correct progress on most devices, but allow a longer timeout for the slow ones.
		const int MaxFlashProgress = MaxFlashTime / 4;
		// This causes some progress seen by the user,
		// but we know it is just faked and runs by the time.
		size_t Percent = DownloadPercentage + (time*FlashPercentage) / MaxFlashProgress;

		// for slow devices, we reduce the progress (it is an older and slower chip)
		if (Percent > 100)
		{
			// Try even if we did not get a message from OS
			// This is just a conservative fallback, not really needed.
			const bool bForce = true;
			if (CheckForNewDevice(bForce))
			{
				// Looks like the COMx name hase been changed. Happens if the new FW has a different VID/PID.
				CallbackProgress(EV_DeviceChangedInterface, 0);
				RetVal = ERROR_SUCCESS;
				break;
			}

			Percent = DownloadPercentage + (time*FlashPercentage) / MaxFlashTime;
			if (!bSentStillRunning)
			{
				CallbackProgress(EV_FlashStillRunning, 0);
				bSentStillRunning = true;	// only send it once
			}
		}

		// dont send it too often, it is a waste of resources
		if (OldPercent != Percent)
		{
			OldPercent = Percent;
			CallbackProgress(EV_Flashing, Percent);
		}

		if (IsAborted())
			break;

	} while (time < MaxFlashTime);	// timeout?

	// Try to communicate with the device
	ReconnectAfterFlashing(RetVal);
	CallbackProgress(EV_FlashFinished, RetVal);
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Try to reconnect to the device after the firmware has been stored to the flash rom.
/*! We retrieve the firmware version from the device. If that fails, we assume the device is dead.
 @param long &RetVal reference to the error status of our caller
*/
void CDeviceFirmware::ReconnectAfterFlashing(int &RetVal)
{
	// Try to communicate with the device
	// We do not try to talk to disconnected USB devices (5 Sec timeout!)
	// Non USB ports return connected all the time.
	if(IsUSBConnected())
	{
		// Turn certain protocolls off, they could disturb our communication
		InitialCommunication();

		m_sNewDeviceVersion.Empty();
		// Do a retry in case the device did not start fast enough
		for(int i=0; i<2; i++)
		{
			if (RetrieveVersionFromDevice(m_sNewDeviceVersion))
			{
				if (RetVal==ERROR_FLASH_FAILED)	// did device timeout while flashing?
				{
					RetVal = ERROR_FLASH_UNSURE;	// at least we could communicate
															// we should check the version, but the file is compressed ==  not so easy yet.
				}
				RetrieveSerialStringFromDevice(m_sNewSerial);
				break;
			}
		}

		if (m_sNewDeviceVersion.IsEmpty())	// still no response?
		{
			if (RetVal==ERROR_SUCCESS)			// did we have no timeout while flashing?
			{
				RetVal = ERROR_FLASH_NO_RESPOND;	// communicate is dead (unlikely)
			}
		}
		m_fFirmwareReplace = false;
		m_fOutHdrMissing = false;
		m_fWantSynHeader = true;
		EnableRobustCommunication();
	}
}

///////////////////////////////////////////////////////////////////////////////
//! This is the thread code that controls the download and flashing process.
/*!
 @return Error value as defined by Results_t
*/
int CDeviceFirmware::DownloadAndFlash(void)
{
	ASSERT(m_pFirmware!=NULL);

	// Download
	int RetVal = DownloadFirmware(*m_pFirmware);
	CallbackProgress(EV_FlashStarted, DownloadPercentage);

	// Wait untill finshed
	if (RetVal == ERROR_SUCCESS)
	{
		RetVal = WaitForDeviceToFinishFlashing();
	}

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Prepare the firmware flashing.
/*! Reset counters, load firmware image, check whether device is compatible
	and finally retrieve version and serial number from device.
@return Error value as defined by Results_t
*/
int CDeviceFirmware::PrepareFlashing()
{
	if (!m_valid)
		return ERROR_INVALID_PARAMETER;

	CleanupBeforeFlashing();
	CValidateDevice Device;
	Device.AllowUnknownDevices(m_AllowUnknownDevices);
	int RetVal;
	if (m_Filename.GetLength() > 0)
		RetVal = m_pFirmware->LoadFile(m_Filename);
	else
		RetVal = m_pFirmware->LoadBuffer(m_pBuffer, m_Size);

	if (RetVal == ERROR_SUCCESS)
	{
		RetVal = Device.IsFileCompatibleToDevice(*this, *m_pFirmware);
		m_DeviceType = Device.GetDeviceType();
		m_CompId = Device.GetCompId();
	}
	if (RetVal == ERROR_SUCCESS)
	{
		RetrieveVersionFromDevice(m_sOldDeviceVersion);
		RetrieveSerialStringFromDevice(m_sOldSerial);
	}

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Prepare the firmware flashing.
/*! Reset counters, load firmware image, check whether device is compatible
and finally retrieve version and serial number from device.
@return Error value as defined by Results_t
*/
int CDeviceFirmware::PrepareConfig(CallbackProgress_t pProgress)
{
	InstallProgressCallback(pProgress);
	InitialCommunication();
	RetrieveVersionFromDevice(m_sOldDeviceVersion);
	RetrieveSerialStringFromDevice(m_sOldSerial);

	return ERROR_SUCCESS;
}

std::string sLastGoodConnection = "";

///////////////////////////////////////////////////////////////////////////////
//! The high level door to the firmware flashing.
/*! Call this function to do a complete validation and download.
 @return Error value as defined by Results_t
*/
int CDeviceFirmware::ExecuteFirmwareFlash()
{
	int RetVal = PrepareFlashing();

	if (RetVal == ERROR_SUCCESS)
	{
		CallbackProgress(EV_DownloadStarted, 0);
		RetVal = DownloadAndFlash();
	}
	
	if (RetVal == ERROR_SUCCESS)
	{
		sLastGoodConnection = (const char*)m_sDevice;
	}
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Simple interface to the flasher.
/*! This is a blocking call, it will not return until the firmware is replaced.
 * Setup the Details before with one of the overloads of SetFWDetails.
 @return ERROR_SUCCES on success
*/
int CDeviceFirmware::ReplaceFirmware()
{
	if(!m_valid)
		return ERROR_INVALID_PARAMETER;

	int RetVal=ERROR_PORT_NOT_OPEN;
	bool bNeedFlashing=true;

	if(Connect(m_sDevice, m_baudrate))
	{
		InitialCommunication();

		if((m_pFirmwareId!=NULL)&&(m_pFirmwareId[0]!=0))
		{
			CStringA sNewDeviceVersion(m_pFirmwareId);
			bNeedFlashing = !isSameVersion(sNewDeviceVersion);
		}

		if(bNeedFlashing)
		{
			RetVal = ExecuteFirmwareFlash();
		}
		else
		{
			CallbackProgress(EV_NothingToDo, 0);
			RetVal = ERROR_SUCCESS;
		}

		DisConnect();
	}
	return RetVal;
}


///////////////////////////////////////////////////////////////////////////////
//! Setup flash parameters.
/*! Call to prepare flashing from a file.
 @param [in] pFirmwareId	firmware ID (e.g. BI000612AAA)
 @param [in] pProgress		callback
 @param [in] pFilename		firmware file
 @param [in] pDevice		device name
 @param [in] baudrate		baudrate
 @return ERROR_SUCCES on success
*/
int CDeviceFirmware::SetFWDetails(const char* pFirmwareId, CallbackProgress_t pProgress, 
		const char* pFilename, const char* pDevice, int baudrate)
{
	m_valid = false;
#ifdef WIN32
	if (pFilename == NULL)
		return ERROR_INVALID_PARAMETER;
#else
	if ((pFilename == NULL) || (pDevice == NULL))
		return ERROR_INVALID_PARAMETER;
#endif

	m_Size=0;
	m_pBuffer=NULL;
	m_Filename = pFilename;
	m_pFirmwareId = pFirmwareId;
	if (pDevice != NULL)
		m_sDevice = pDevice;
	m_baudrate = baudrate;
	InstallProgressCallback(pProgress);
	m_valid = true;
	return ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//! Setup flash parameters.
/*! Call to prepare flashing from a buffer.
 @param [in] pFirmwareId	firmware ID (e.g. BI000612AAA)
 @param [in] pProgress		callback
 @param [in] pBuffer		buffer with firmware
 @param [in] Size			size of firmware
 @param [in] pDevice		device name
 @param [in] baudrate		baudrate
 @return ERROR_SUCCES on success
*/
int CDeviceFirmware::SetFWDetails(const char* pFirmwareId, CallbackProgress_t pProgress, 
		const UCHAR *pBuffer, size_t Size, const char* pDevice, int baudrate)
{
	m_valid = false;
#ifdef WIN32
	if((pBuffer==NULL)||(Size==0))
		return ERROR_INVALID_PARAMETER;
#else
	if ((pBuffer == NULL) || (pDevice == NULL) || (Size == 0))
		return ERROR_INVALID_PARAMETER;
#endif


	m_Size=Size;
	m_pBuffer=pBuffer;
	m_Filename.Empty();
	m_pFirmwareId = pFirmwareId;
	if(pDevice!=NULL)
		m_sDevice = pDevice;
	m_baudrate = baudrate;
	InstallProgressCallback(pProgress);
	m_valid = true;
	return ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//! This is the low level one shot download and flash command.
/*! It is a blocking call, so your GUI is not responsive for more than a minute if you call it directly from a
	message handler. It is better to call it from a separate thread. \sa CThreadedDeviceCommands
 @param [in,out] Firmware This object contains the complete firmware image.
 @return Error value as defined by Results_t
*/
long CDeviceFirmware::DownloadFirmwareWithXModem(CValidateFirmware &Firmware)
{
	TRACE("Using XModem\r");
	SendNonMenuCommand("NEWAPP");
	return Transmit(Firmware.GetFirmware(), Firmware.GetFirmwareSize());
}


///////////////////////////////////////////////////////////////////////////////
// Below is code to download the firmware with a special header called htag.
// This is an easy method (would also work on a cmd line with copy command).
// Due to less overheader and turn around times, it can be much faster,
// but requires a save connection. Therefore we limit its use to USB connections.
// Speed: For N5680 the download takes ~two seconds
//		(CDC-ASCM interface, USB high speed),
// xmodem needs ~20 seconds on the same system.
// For HidPos or full speed connections, the difference is much smaller.
///////////////////////////////////////////////////////////////////////////////

#define ACK  0x06

///////////////////////////////////////////////////////////////////////////////
//! Download firmware.
/*! For secure connections we do not need additional protocol,
	so we send with the htag metod since it is faster.
	RS232 connections use the xmodem protocol.

 @param [in,out] Firmware This object contains the complete firmware image.
 @return Error value as defined by Results_t
*/
long CDeviceFirmware::DownloadFirmware(CValidateFirmware &Firmware)
{
	int RetVal=ERROR_COMMUNICATION;
	if(m_AllowHtag&&IsSecureConnection())
	{
		RetVal = DownloadFirmwareWithHtag(Firmware);
		if(RetVal==ERROR_SUCCESS)
		{
			// Device sends an ACK after receiving the hatg format succesfully.
			// We must receive that here, otherwise the later reconnect check would
			//	terminate way too early (seeing the ACK).
			//	BTW: The xmodem method sends the ACK after the flash.
			int Received = Read();	// read with timeout of 500 mSec
			if (Received==ACK)
			{
				RetVal=ERROR_SUCCESS;
			}
		}
	}
	else
	{
		RetVal = DownloadFirmwareWithXModem(Firmware);
	}
	return RetVal;
}

// If you do not need a progress notification, you can uncomment the next line.
// That also shows how simple this method is.
//#define NO_PROGESS_INDICATOR 1

#ifndef NO_FASTMODE

///////////////////////////////////////////////////////////////////////////////
//! Transmit with Htag header.
/*! This function adds a header and sends both the header and firmware to the device.

@param [in,out] Firmware This object contains the complete firmware image.
@return Error value as defined by Results_t
*/
long CDeviceFirmware::DownloadFirmwareWithHtag(CValidateFirmware &Firmware)
{
    TRACE("Using Htag\r");
	int RetVal=ERROR_SUCCESS;
	int crc = crc16_ccitt(Firmware.GetFirmware(), Firmware.GetFirmwareSize());
	CStringA sDesc = CreateHtagDescriptor(crc);
	CStringA sHtag = CreateHTag(Firmware.GetFirmwareSize(), sDesc);

	Write(sHtag);
#ifdef NO_PROGESS_INDICATOR
	Write(Firmware.GetFirmware(), Firmware.GetFirmwareSize());
#else
	// Send shorter packets so we get some progress indication
	int PacketSize=1024*50;	// leads to ~+20 progress notifications
	int RestSize=(int) Firmware.GetFirmwareSize();
	const UCHAR *pSource = Firmware.GetFirmware();

	while(RestSize > 0)
	{
		int PayloadSize = RestSize;
		if(PayloadSize > PacketSize)
			PayloadSize = PacketSize;
		RestSize -= PayloadSize;
		// send progress here for a better user experience
		if (ShowWriteProgress(Firmware.GetFirmwareSize()-RestSize))
		{
			RetVal = ERROR_CANCELED_BY_USER;
			break;
		}
		Write(pSource, PayloadSize);
		pSource += PayloadSize;
	}
#endif // NO_PROGESS_INDICATOR
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Create htag descriptor.
/*!

@param [in] crc of firmware.
@return htag descriptor
*/
CStringA CDeviceFirmware::CreateHtagDescriptor(int crc)
{
	CStringA sDesc;
	sDesc.Format("APPCOD2P1STORE%dCRC1RSPREQ1PRSPREQ\x1D", crc);
	return sDesc;
}

///////////////////////////////////////////////////////////////////////////////
//! Create Htag header.
/*!

@param [in] FirmwareSize Size of Firmware.
@param [in] sDesc htag descriptor.
@return htag
*/
CStringA CDeviceFirmware::CreateHTag(size_t FirmwareSize, CStringA sDesc)
{
	FirmwareSize += sDesc.GetLength();
	CStringA sHTagHeader = "\x16\xFE\x01\x02\x03\x04\r";
	sHTagHeader.SetAt(2, FirmwareSize&0xFF);
	sHTagHeader.SetAt(3, (FirmwareSize>>8)&0xFF);
	sHTagHeader.SetAt(4, (FirmwareSize>>16)&0xFF);
	sHTagHeader.SetAt(5, (FirmwareSize>>24)&0xFF);
	return sHTagHeader + sDesc;
}
#else // NO_FASTMODE
long CDeviceFirmware::DownloadFirmwareWithHtag(CValidateFirmware &Firmware)
{
	return -1;
}
#endif // NO_FASTMODE

}	// namespace HsmDeviceComm
