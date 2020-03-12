/*=================================================================================
  xmodem.h
//=================================================================================
   $Id: xmodem.h 203 2019-01-31 16:41:15Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// xmodem.h : header file
//
#pragma once

namespace HsmDeviceComm {

///////////////////////////////////////////////////////////////////////////////
//!  This is the core class to send data with the XModem protocol.
/*! It is based on idea from several public domain sources, but has been cleaned up
 for better reading and clean programming. The new code has only been tested with
 the Adaptus devices.

 Use the Transmit() function to start an XModem transfer.
 Derive a class from it and fill the virtual functions with life.
 This class tries to be pretty portable.
 Receiving has not been implemented.

 \subpage xmodem_spec
*/
class CXModem
{
protected:
	CXModem();

	long Transmit(const UCHAR *pSource, size_t Size);

protected:
	virtual void XM_PurgeReceive();	      				// See the .cpp for doc
	virtual bool ShowWriteProgress(size_t BytesSend);	// See the .cpp for doc
	virtual bool IsAborted();							// See the .cpp for doc

	///////////////////////////////////////////////////////////////////////////////
	//! Initialize the low level RX and TX.
	/*! You must overwrite it in your derived class. */
	virtual void XM_InitRxTx()=0;

public:
	///////////////////////////////////////////////////////////////////////////////
	//! Read a byte from the serial connection.
	/*! You must overwrite it in your derived class.
		Ensure that the read timeout is about 500mSec.
	  @return -1 for timeout, else the received byte.
	*/
	virtual int	 XM_Read()=0;

	///////////////////////////////////////////////////////////////////////////////
	//! Write a byte to the serial connection.
	/*! You must overwrite it in your derived class.
	 @param byte Gets send serially.
	*/
	virtual void XM_Write(UCHAR byte)=0;
	///////////////////////////////////////////////////////////////////////////////
	//! Write a group of byte to the serial connection.
	/*! You must overwrite it in your derived class.
	 @param *pByte Point to the buffer to be sent.
	 @param Size Amount of bytes to be sent.
	*/
	virtual void XM_Write(const UCHAR *pByte, size_t Size)=0;
	virtual void XM_Write(const char *pByte, size_t Size) { XM_Write((const UCHAR *) pByte, Size);	}

private:
	long StartSyncTX(void);
	long WaitForACK(void);
	long SendPacket(void);
	void AddChecksum(int HeaderSize, int PacketSize);
	void BuildPacket(const UCHAR *pSource, int Size, int PacketSize, unsigned char PacketNum);

	void AddTaggedEntry(int &index, USHORT Tag, size_t Value);
	void BuildInfoBlock(size_t FW_Size);
	long TransmitInfoBlock(size_t FW_Size);

private:
	enum
	{
		BUFFERSIZE = 1024+3+2+1,					//!< 1024 for XModem 1k + 3 head chars + 2 crc + nul
	};
	bool m_UseCrc;									//!< Flag to show we must use a crc rather a simple checksum
	size_t m_TotalPacketSize;						//!< Size of the composed packet
	UCHAR m_TxBuf[BUFFERSIZE+4];					//!< Buffer to hold a composed packet
};

}
