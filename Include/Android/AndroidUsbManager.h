// ============================================================================
// $Id: AndroidUsbManager.h 203 2019-01-31 16:41:15Z e411776 $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Class to manage USB interfaces.
	Details.
	\file */
//=============================================================================
#pragma once
#include "PortEvents.h"
#include "AndroidCdcAcm.h"
#include "AndroidHidPos.h"


class CUsbManager
{
    enum { MAX_DISPATCH_LENGHT = 256 };
public:
	CUsbManager(HWND hNotifictionReceiver=NULL, DWORD MsgStatus=0);
	virtual ~CUsbManager();
    virtual bool Connect(UsbDetails_t &device, int baudrate=0);
    virtual bool Disconnect();
	virtual bool isValid();
//protected:
	virtual void PrepareDisconnect();
	virtual bool IsUSBConnected();
    virtual bool IsConnected();
	virtual void PurgeReceive();
	virtual void InitRxTx();

	virtual int Read();
	virtual size_t Read(UCHAR *buffer, size_t size);
	virtual size_t ReadBlock(UCHAR *buffer, size_t size);	// non waiting flavour

	virtual bool Write(UCHAR byte);
	virtual bool Write(const UCHAR *pByte, size_t Size);
	virtual bool Write(const char *pByte, size_t Size)
	{
		return Write((const UCHAR *)pByte, Size);
	}


    bool SetReadCallback(CallbackRead_t *pFunctorRead=NULL, size_t nMaxLength=MAX_DISPATCH_LENGHT);
    bool SetStatusCallback(CallbackStatus_t *pFunctorStatus=NULL, DWORD mask=EV_PORTMAN_DEFAULT);

    bool SetReadTimeout(int, int, int) { return true; }   // dummy for now
    bool Baudrate(int) {  return true; }
    bool Set8N1() { return true; }
    bool OpenConnection() { return true;}
    bool CloseConnection() { return true;}
    void SetCurrentDisplayName(const char*) {}
    bool SetDisplayNameWithCheck(const char*) { return true; }
    const char* GetTrueDisplayName() { return "unknown"; }

	virtual bool IsSecureConnection();


protected:
	Interfacetype_t	m_type;	//
	CUsbBase *m_pUsb;

private:
	// No copy and assignment allowed.
	CUsbManager(const CUsbManager &);
	void operator =(const CUsbManager &);
};
