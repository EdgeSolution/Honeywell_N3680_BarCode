
#pragma once

#include "DeviceComm.h"
#include "HsmProgress.h"
#include "xmodem.h"

namespace HsmDeviceComm {

class CValidateFirmware;
#define CDeviceFirmwareBase CDeviceComm

class CDeviceFirmware : public CDeviceFirmwareBase, public CXModem
{
public:
	CDeviceFirmware(HWND hNotifictionReceiver = NULL, DWORD MsgStatus = 0);
	virtual ~CDeviceFirmware();
	int SetFWDetails(const char* pFirmwareId, CallbackProgress_t pProgress, 
			const char* pFilename, const char* pDevice=NULL, int baudrate=0);
	int SetFWDetails(const char* pFirmwareId, CallbackProgress_t pProgress, 
			const UCHAR *pBuffer, size_t Size, const char* pDevice=NULL, int baudrate=0);

	int ReplaceFirmware();

// high level functions
	long DownloadFirmware(CValidateFirmware &Firmware);
	long DownloadFirmwareWithHtag(CValidateFirmware &Firmware);
	long DownloadFirmwareWithXModem(CValidateFirmware &Firmware);

// low level functions (internal)

	void AllowHtagMethod(bool newval)       { m_AllowHtag=newval;	}
	void AllowUnknownDevices(bool newval)   { m_AllowUnknownDevices=newval;	}

	CString GetOldDeviceVersion(void);
	CString GetNewDeviceVersion(void);
	CString GetOldSerial(void);
	CString GetNewSerial(void);
	CString GetFlashErrorResponse(void);
	UINT GetDeviceType(void) 	const { return m_DeviceType; } //!< readonly accessor
	int  GetCompId(void) 		const { return m_CompId; } //!< readonly accessor


	virtual bool CallbackProgress(phases_t Phase, int Percent);
	virtual bool CheckForNewDevice(bool bForce = false) = 0;
	int PrepareConfig(CallbackProgress_t pProgress);
	void InstallProgressCallback(CallbackProgress_t pProgress) { m_pProgress = pProgress; }
	const char *GetDeviceName() { return m_sDevice; }

protected:
	int ExecuteFirmwareFlash();
	virtual bool ShowWriteProgress(size_t BytesSend);
	void ReconnectAfterFlashing(int &RetVal);

protected:
	virtual bool IsAborted(void);
	int DownloadAndFlash(void);
	void CleanupBeforeFlashing(void);
	int PrepareFlashing(void);
	int WaitForDeviceToFinishFlashing(void);

	CStringA CreateHtagDescriptor(int crc);
	CStringA CreateHTag(size_t FirmwareSize, CStringA sDesc);

	protected:
	// re-wire to real read/write functions
	virtual void XM_InitRxTx();
	virtual void XM_PurgeReceive();
	virtual int  XM_Read();
	virtual void XM_Write(UCHAR byte);
	virtual void XM_Write(const UCHAR *pByte, size_t Size);

protected:
	CallbackProgress_t m_pProgress;			//!< A callback to show progress
	CValidateFirmware *m_pFirmware;			//!< Point to the image of the firmware
	CStringA m_sOldDeviceVersion;			//!< Device response to the REF_WA?. command before flashing
	CStringA m_sNewDeviceVersion;			//!< Device response to the REF_WA?. command after flashing
	CStringA m_sFlashErrorResponse;			//!< Device response to the NEWAPP command in case of failure
	CStringA m_sOldSerial;					//!< Serial number of device before flashing
	CStringA m_sNewSerial;					//!< Serial number of device after flashing

	const char* m_pFirmwareId;				//!< Point to a FirmwareID, can be NULL or point to "".
	CString m_Filename;						//!< Filename of firmware file (we need either a filename or a buffer).
	const UCHAR *m_pBuffer;					//!< Buffer containg a firmware (we need either a filename or a buffer).
	size_t m_Size;							//!< Size of firmware in buffer.
	CStringA m_sDevice;						//!< Point to device name.
	int m_baudrate;							//!< Baudrate.
	bool m_valid;							//!< Flash parameters are valid or not.
	UINT m_DeviceType;						//!< Device type to avoid flasing wrong firmware (used by Generation 4 and 5)
	int  m_CompId;							//!< Compatible Device ID to avoid flasing wrong firmware (used by Generation 6)
	bool m_AllowHtag;						//!< Do not use Xmodem and send FW as one block (can be faster with USB high speed)
	bool m_AllowUnknownDevices;				//!< Do not use the list of known devices and accept all CompID > 0
};

}
