// ============================================================================
// $Id: AndroidHidPos.cpp 158 2018-12-06 17:58:56Z e411776 $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Connection class for HidPos.
	Details.
	\file */
//=============================================================================

#include "stdafx.h"
#include "AndroidHidPos.h"
#include <linux/usbdevice_fs.h>
#include <sys/ioctl.h>

// Some portions taken from dfUsbLib sources with friendly permission from the author
#define HID_COMMON_OUT_REDIRECT_REPORT_ID		254  // redirect the scanners output
#define HID_COMMON_OUT_1252_REPORT_ID			253  // sending data to the scanner
#define HID_COMMON_OUT_REPORT_SIZE 64
#define HID_COMMON_FEATURE_REDIRECT_SIZE	2
#define HID_OUT_1252_SIZE	(HID_COMMON_OUT_REPORT_SIZE-1-1)

#define HID_COMMON_IN_REPORT_SIZE 64
#define HID_COMMON_OUT_REPORT_SIZE 64
#define HID_OUT_1252_SIZE (HID_COMMON_OUT_REPORT_SIZE-1-1)
#define HID_OUT_1252_HEADER_SIZE 2
#define HID_COMMON_FEATURE_REDIRECT_SIZE 2
#define HID_COMMON_FEATURE_REDIRECT_DUMMY_SIZE (HID_COMMON_OUT_REPORT_SIZE-HID_COMMON_FEATURE_REDIRECT_SIZE)

#define HIDPOS_BARCODE_ID 2
#define HIDPOS_BARCODE_SIZE 56
#define HID_COMMON_OUT_REDIRECT_REPORT_ID 254
#define HID_COMMON_OUT_1252_REPORT_ID 253

struct UsbCommonHidReportBuffer_t
{
    union
    {
        struct
        {
            unsigned char ucArray[HID_COMMON_OUT_REPORT_SIZE];
        }bytes;
        struct
        {
            unsigned char ucReportId;  									/* HID ID */
        }id;
        struct
        {
            unsigned char ucReportId;  									/* HID ID */
            unsigned char ucLength;  										/* HID size */
            unsigned char ucAIM[3];  										/* AIM ID */
            unsigned char ucContent[HIDPOS_BARCODE_SIZE];			    /* contents */
            unsigned char ucHHP[2];  								        /* HHP ID */
            unsigned char ucFlags;  										/* Bit0=Decode Data Continued */
        }barcode;
        struct
        {
            unsigned char ucReportId;  									/* HID ID */
            unsigned char ucFlags; 										/* Response contents */
        }triggerreport;
        struct
        {
            unsigned char ucReportId;  									/* HID ID */
            unsigned char ucInterface;										/* interface for redirection */
        }feature_redirect;
        struct
        {
            unsigned char ucReportId;  									/* HID ID */
            unsigned char ucLength;  										/* HID size */
            unsigned char ucContent[HID_OUT_1252_SIZE];				    /* contents */
        }outreport;
    }u;
};


CUsbHidPos::~CUsbHidPos()
{
}

void CUsbHidPos::Connect()
{
    HidPosRedirectOutput();
    CUsbBase::Connect();
}

void CUsbHidPos::PrepareDisconnect()
{
}

bool CUsbHidPos::IsUSBConnected()
{
	return false;
}

void CUsbHidPos::InitRxTx()
{
}

void CUsbHidPos::HidPosRedirectOutput()
{
    UsbCommonHidReportBuffer_t buffer;
    buffer.u.feature_redirect.ucReportId = HID_COMMON_OUT_REDIRECT_REPORT_ID;
    buffer.u.feature_redirect.ucInterface = 1;
    setFeature(HID_COMMON_OUT_REDIRECT_REPORT_ID, buffer.u.bytes.ucArray, HID_COMMON_FEATURE_REDIRECT_SIZE);
}

size_t CUsbHidPos::read_internal(UCHAR *pByte, size_t BufferSize)
{
    UsbCommonHidReportBuffer_t buffer;
    struct usbdevfs_bulktransfer bt;
    bt.ep = m_InEp;		// endpoint (received from Java)
    bt.len = HID_COMMON_IN_REPORT_SIZE;	    // length of data
    bt.timeout = read_timeout;		            // timeout in ms
    bt.data = (void*)buffer.u.bytes.ucArray;	// the data

    int ReturnVal = ioctl(m_fd, USBDEVFS_BULK, &bt);
    if((ReturnVal==HID_COMMON_IN_REPORT_SIZE) && (buffer.u.barcode.ucReportId==HIDPOS_BARCODE_ID))
    {
        int size = buffer.u.barcode.ucLength;
        if(size>BufferSize)
            size=0;

        memcpy(pByte, buffer.u.barcode.ucContent, size);
        ReturnVal=size;
    }
    else
    {
        ReturnVal=0;
    }
    return ReturnVal;
}

size_t CUsbHidPos::write_internal(const UCHAR *pByte, size_t Size)
{
    UsbCommonHidReportBuffer_t buffer;
    buffer.u.outreport.ucReportId = HID_COMMON_OUT_1252_REPORT_ID;
    buffer.u.outreport.ucLength = Size;
    memcpy(buffer.u.outreport.ucContent, pByte, Size);
    memset(&buffer.u.outreport.ucContent[Size], 0, HID_OUT_1252_SIZE-Size);

    struct usbdevfs_bulktransfer bt;
    bt.ep = m_OutEp;		// endpoint (received from Java)
    bt.len = HID_COMMON_OUT_REPORT_SIZE;	    // length of data
    bt.timeout = write_timeout;		            // timeout in ms
    bt.data = (void*)buffer.u.bytes.ucArray;	// the data

    int RetVal = ioctl(m_fd, USBDEVFS_BULK, &bt);
    // fixme: check how to reliable get amount of bytes written
    return Size;
}
