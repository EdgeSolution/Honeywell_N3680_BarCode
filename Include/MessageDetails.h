/*=================================================================================
  MessageDetails.h
//=================================================================================
   $Id: MessageDetails.h 159 2018-12-07 15:55:44Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#pragma once

namespace HsmDeviceComm {

///////////////////////////////////////////////////////////////////////////////
/// Acts as a storage space for incomming data packets from the device
/*! This class adds additional status information and helpers to the 
	CStringA that holds the received data stream.
	
	
 \ingroup Helpers
 \par requirements
 MFC\n
*/
	class CMessageDetails : public CStringA
	{
	public:
	CMessageDetails(void);
	CMessageDetails(CStringA sString);
	CMessageDetails(const char *pText, size_t Length);
	~CMessageDetails(void);

	//! Declare types of received messages
	/*! Messages from a device can contain a barcode contents, a picture or a command response */
	enum PayloadType_t
	{
		None, Unknown, Text, Image, BadImage, CmdResponse, BadCmdResponse
	};

	void Reset(void);
	void SetInvalid(void) { m_valid = false; }	//!< Sets status to invalid
	void SetValid(void) { m_valid = true; }	//!< Sets status to valid
	bool IsValid(void) { return m_valid; }	//!< Returns the status of the message
	void SetCmdResponse(void) { m_type = CmdResponse; }	//!< Set type to command response

	bool IsText(void) { return (m_type == Text); }	//!< Is it a text message
	bool IsImage(void) { return (m_type == Image); }	//!< Is is a picture message
	bool IsCmdResponse(void) { return (m_type == CmdResponse); }	//!< Is is a command response
	int  GetImageFormat(void) { return m_ImageFormat; }	//!< Returns the format ID of the image
	size_t GetPayloadSize(void) { return m_PayloadLength; }	//!< Returns the size of the true payload
	CStringA GetRawPayloadData(void);
	UCHAR GetAckByte(void) { return m_AckByte; }
	void SetBufferSize(int size) { m_BufferSize = size; }
	void Collect(const char *pReceived, int AddSize);

	bool GetRawPayloadBuffer(char *&pData, size_t &Length);
	PayloadType_t ParseMessage(void);
	int ParseCmdMessage(void);
	int ParseStructuredMessage(void);
	int WritePayloadToFile(CString sFileName);
	void SetBadImage(void);
#ifndef _MFC_VER
	UCHAR GetAt(int index)  { return at(index); }
#endif

	UCHAR m_HH_Id;											//!< The traditional Hand Held Products Symbology ID
	UCHAR m_AIM_Id;										//!< The AIM symbology ID
	UCHAR m_AIM_Modifier;								//!< The Modifier for the AIM symbology ID
	int m_PayloadStart;									//!< Offset to the payload data (-1 if invalid)
	int m_PayloadLength;									//!< Length of the payload data (0 if invlaid)
	int m_BufferSize;

protected:
	void SetImage(void);
	void SetTextFromMsgGet(void);
	void SetTextFromMsg00X(void);
//	void SetByteMessage(void);
	void SetRawMessage(void);

protected:
	PayloadType_t m_type;								//!< Type of received message
	bool m_valid;											//!< Shows whether the other content is valid
	int m_ImageFormat;									//!< Format ID for the images, used by the IMGSHP command
	int m_PixelDepth;										//!< Number of bits per pixel (1 or 8)
	UCHAR m_AckByte;
};

}	// namespace HsmDeviceComm
