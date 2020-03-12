/*=================================================================================
  xmodem.cpp
//=================================================================================
   $Id: xmodem.cpp 203 2019-01-31 16:41:15Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// xmodem.cpp : implementation file
//

#include "stdafx.h"
#include "xmodem.h"
#include "crc16.h"
#include "HsmErrorDefs.h"

namespace HsmDeviceComm {

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

///////////////////////////////////////////////////////////////////////////////
//! The constructor does nothing fancy.
/*!
*/
CXModem::CXModem()
: m_UseCrc(false)
, m_TotalPacketSize(0)
{
}

///////////////////////////////////////////////////////////////////////////////
//! A default implementation of the purge function.
/*! Overwrite it in a derived class for a better solution.
*/
void CXModem::XM_PurgeReceive(void)
{
	while (XM_Read() >= 0);
	while (XM_Read() >= 0);	// extend the timeout
}

///////////////////////////////////////////////////////////////////////////////
//! A "hook" to get progress information.
/*! Overwrite it in a derived class for a better solution.

	This default implementation does nothing.
 @param BytesSend Number of bytes send so far
 @return true to abort, false to proceed
*/
bool CXModem::ShowWriteProgress(size_t /* BytesSend */)
{
	// just a no-op here
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//! A "hook" to get information on an abort by the user.
/*! Overwrite it in a derived class for a better solution.

	This default implementation does nothing.
 @return true to abort, false to proceed
*/
bool CXModem::IsAborted(void)
{
	// just a no-op here
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//! Tries to synconize with the receiver.
/*! Here we wait for a NAK or 'C' from the receiver.
 After some timout we abort.
 @return Error value as defined by Results_t
*/
long CXModem::StartSyncTX(void)
{
	long RetVal = ERROR_NO_SYNC;
	int c;
	const int Retries = 120;	// allow for some short text messages

	for(int retry = 0; retry < Retries; retry++)
	{
		c = XM_Read();
		switch (c)
		{
		case 'C':
			m_UseCrc = true;
			RetVal = ERROR_SUCCESS;
			break;
		case NAK:
			m_UseCrc = false;
			RetVal = ERROR_SUCCESS;
			break;
		case CAN:
			if ((c = XM_Read()) == CAN)
			{
				XM_Write(ACK);
				XM_PurgeReceive();
				RetVal = ERROR_CANCELED_BY_REMOTE;
			}
			break;
		default:
			if(IsAborted())
				RetVal = ERROR_CANCELED_BY_USER;
			break;
		}

		if(RetVal != ERROR_NO_SYNC)	// any other error or success?
			break;
	}

	if(RetVal == ERROR_NO_SYNC)
	{
		XM_Write(CAN);
		XM_Write(CAN);
		XM_Write(CAN);
		XM_PurgeReceive();
	}
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Wait for an ACK or other protocol byte.
/*! 
 @return Error value as defined by Results_t
*/
long CXModem::WaitForACK(void)
{
	long RetVal = ERROR_XMIT_ERROR;

	int c = XM_Read();
	if (c >= 0 )
	{
		switch (c)
		{
		case ACK:
			RetVal = ERROR_SUCCESS;
			break;
		case CAN:
			c = XM_Read();
			if (c == CAN)
			{
				XM_Write(ACK);
				TRACE(_T("Got a CAN\r\n"));
				XM_PurgeReceive();
				RetVal = ERROR_CANCELED_BY_REMOTE;
			}
			break;
		case NAK:
			TRACE(_T("Got a NAK\r\n"));
			if(IsAborted())
				RetVal = ERROR_CANCELED_BY_USER;
			break;
		default:
			TRACE(_T("Got an unknown char (%x)\r\n"), c);
			if(IsAborted())
				RetVal = ERROR_CANCELED_BY_USER;
			break;
		}
	}

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Send the precomposed packet and wait for the ACK.
/*! The packet must be ready to send in the buffer m_TxBuf.

 @return Error value as defined by Results_t
*/
long CXModem::SendPacket(void)
{
	long RetVal = ERROR_XMIT_ERROR;
	const int Retries = 16;

	for (int retry = 0; retry < Retries; retry++)
	{
		XM_PurgeReceive();	// remove all received garbage first
		XM_Write(m_TxBuf, m_TotalPacketSize);
		RetVal = WaitForACK();
		if(RetVal != ERROR_XMIT_ERROR)	// any other error or success?
		{
			break;
		}
	}

	if (RetVal == ERROR_XMIT_ERROR)
	{
		XM_Write(CAN);
		XM_Write(CAN);
		XM_Write(CAN);
		XM_PurgeReceive();
	}
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Calculate and add checksum or CRC.
/*! Depending on the "request to send" value of the receiver we decide which kind of checksum we use.

 @param HeaderSize Size of the Header (0 or 3)
 @param PacketSize XModem packet size of packet (1024 or 128)
*/
void CXModem::AddChecksum(int HeaderSize, int PacketSize)
{
	if (m_UseCrc)
	{
		unsigned short ccrc = crc16_ccitt(&m_TxBuf[HeaderSize], PacketSize);
		m_TxBuf[PacketSize+HeaderSize]	= (UCHAR)((ccrc>>8) & 0xFF);
		m_TxBuf[PacketSize+HeaderSize+1] = (UCHAR)(ccrc & 0xFF);
		m_TotalPacketSize = PacketSize+HeaderSize+sizeof(ccrc);
	}
	else
	{
		unsigned int ccks = 0;
		for (int i = HeaderSize; i < PacketSize+HeaderSize; ++i)
		{
			ccks += m_TxBuf[i];
		}
		m_TxBuf[PacketSize+HeaderSize] = (UCHAR)(ccks & 0xFF);
		m_TotalPacketSize = PacketSize+HeaderSize+1;
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Compose an XModem packet for transmitting.
/*! That means in detail to add the Header (STX, PacketNumbers), payload and the CRC or checksum.
 @param *pSource 		Point to payload
 @param Size 			Size of payload
 @param PacketSize 	XModem packet size of packet (1024 or 128)
 @param PacketNum 	XModem packet number
*/
void CXModem::BuildPacket(const UCHAR *pSource, int Size, int PacketSize, unsigned char PacketNum)
{
// Header
	const int HeaderSize = 3;
	if (PacketSize == 1024)
		m_TxBuf[0] = STX;	                  // 1024
	else
		m_TxBuf[0] = SOH;                   // 128

	m_TxBuf[1] = PacketNum;
	m_TxBuf[2] = ~PacketNum;

// Payload
	memcpy(&m_TxBuf[3], pSource, Size);
	// fill the rest with 0 (if there is room in the packet)
	if (Size < PacketSize)
		memset(&m_TxBuf[HeaderSize+Size], 0, PacketSize-Size);

// Trailer
	AddChecksum(HeaderSize, PacketSize);
}

///////////////////////////////////////////////////////////////////////////////
//! Add a tagged entry to the transmit buffer.
/*! This is a helper for the function BuildInfoBlock(size_t FW_Size).
	 All info in the InfoBlock is stored in a tagged format. It is build by:
	 - a two byte tag
	 - a two byte length info
	 - the value (here it is 4 bytes)

	 All entries are stored in little endian format.

	 We no not check for a size overflow of the transmit buffer.

 @param &index where to store it
 @param Tag to be stored
 @param Value to be stored
*/
void CXModem::AddTaggedEntry(int &index, USHORT Tag, size_t Value)
{
	m_TxBuf[index++] = (UCHAR)(Tag&0xff);			// Tag (little endian)
	m_TxBuf[index++] = (UCHAR)((Tag>>8)&0xff);   // 
	m_TxBuf[index++] = 4;								// Length of value is 4 bytes
	m_TxBuf[index++] = 0;								// 
	m_TxBuf[index++] = (UCHAR)(Value&0xff);		// Value in little endian
	m_TxBuf[index++] = (UCHAR)((Value>>8)&0xff); // 
	m_TxBuf[index++] = (UCHAR)((Value>>16)&0xff);// 
	m_TxBuf[index++] = (UCHAR)((Value>>24)&0xff);// 
}

///////////////////////////////////////////////////////////////////////////////
//! Build an optional info block to the device.
/*! This block contains sizes, compression mode and feature info.
	 (Some devices require the info block in order to work correctly).
	 
	 This code does not support compression at all and this is not reuired with current firmware files.
	 These are compressed internally, so the additional compression does not buy us much.

	 The info is stored in a tagged format, so it can be easily extended in the future.
 @param FW_Size Size of the Firmware
*/
void CXModem::BuildInfoBlock(size_t FW_Size)
{
// The tags here as constants
	const USHORT TransmitBufferSizeTag		= 100;
   const USHORT SizeOfUncompressedDataTag	= 101;
   const USHORT CompressionModeTag			= 200;
   const USHORT FeatureFlagTag				= 300;
// Feature flags
	enum
	{
		SUPPORT_OF_DOUBLE_EOT		= 1
	};

// Header
	const int HeaderSize = 0;
	const int PacketSize = 128;
	int index = 0;

// Payload	
	AddTaggedEntry(index, TransmitBufferSizeTag, FW_Size);
	AddTaggedEntry(index, SizeOfUncompressedDataTag, FW_Size);
	AddTaggedEntry(index, CompressionModeTag, 0);
	AddTaggedEntry(index, FeatureFlagTag, SUPPORT_OF_DOUBLE_EOT);

	// fill the rest with 0 (if there is room in the packet)
	memset(&m_TxBuf[index], 0, PacketSize-index);

// Trailer
	AddChecksum(HeaderSize, PacketSize);
}

///////////////////////////////////////////////////////////////////////////////
//! Send an optional info block to the device.
/*! This block contains sizes and other information.
	 Older devices do not support this block yet and will NAK the try. In this case we 
	 ignore the error and proceed.
	 (Some other devices require the info block).

	 This InfoBlock is not part of the XModem specification, a 
	 plain XModem device will reject ths block.

 @param FW_Size Size of the Firmware
 @return Error value as defined by Results_t
*/
long CXModem::TransmitInfoBlock(size_t FW_Size)
{
	const UCHAR INFO_BLOCK_128_BYTE = 0;
	XM_Write(INFO_BLOCK_128_BYTE);			// Try whether the device supports the info block
	long RetVal = WaitForACK();
	if(RetVal == ERROR_SUCCESS)
	{
		TRACE(_T("TransmitInfoBlock\n"));
		BuildInfoBlock(FW_Size);				// Build and send Info Block
		RetVal = SendPacket();
	}
	else
	{
		if(RetVal == ERROR_XMIT_ERROR)	// This means the device does not support the info block
			RetVal = ERROR_SUCCESS;			// ... so make it a success to proceed
	}
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Transmit with XModem protocol.
/*! This function handles the complete XModem transmit:
	- Wait for the "request to sent" (either NAK or 'C')
	- Build and send the packets
	- Check the response (ACK, ..)
	- Terminate after we are finished (EOT)

 @param *pSource 		Point to payload
 @param Size 			Size of payload
 @return Error value as defined by Results_t
*/
long CXModem::Transmit(const UCHAR *pSource, size_t Size)
{
// Perpare low level TX and RX
	XM_InitRxTx();

	long RetVal = ERROR_NO_SYNC;

// Wait for "request to send"
	TRACE(_T("Wait for request to send\n"));
	RetVal = StartSyncTX();
	if (RetVal != ERROR_SUCCESS)
		return RetVal;

// Send the optional Info block
	RetVal = TransmitInfoBlock(Size);
	if (RetVal != ERROR_SUCCESS)
		return RetVal;

	if(IsAborted())
		return ERROR_CANCELED_BY_USER;

// Build and send packets
	TRACE(_T("Send packets\n"));
	int PacketSize=1024;
	int RestSize=(int)Size;
	UCHAR PacketNum = 1;

	while(RestSize > 0)
	{
		int PayloadSize = RestSize;
		if(PayloadSize > PacketSize)
			PayloadSize = PacketSize;
		BuildPacket(pSource, PayloadSize, PacketSize, PacketNum);
		RetVal = SendPacket();

		if (RetVal == ERROR_SUCCESS)
		{
			pSource += PayloadSize;
			RestSize -= PayloadSize;
			PacketNum++;
			if (ShowWriteProgress(Size-RestSize))
			{
				RetVal = ERROR_CANCELED_BY_USER;
				break;
			}
		}
		else
		{
			break;
		}
	}

// Terminate
	if (RetVal == ERROR_SUCCESS)
	{
		// send EOT
		RetVal = ERROR_XMIT_ERROR;
		for (int retry = 0; retry < 10; retry++)
		{
			XM_Write(EOT);
			int c = XM_Read();
			if(c < 0)
				c = XM_Read();	// extend the timeout

			if (c == ACK)
			{
				RetVal = ERROR_SUCCESS;
				break;
			}
		}
//		XM_PurgeReceive();
	}
	return RetVal;
}


#ifdef ADD_RX_XMODEM
long CXModem::Check(const UCHAR *pBuf, int Size)
{
	if (m_UseCrc)
	{
		unsigned short crc = crc16_ccitt(pBuf, Size);
		unsigned short tcrc = (pBuf[Size]<<8)+pBuf[Size+1];
		if (crc == tcrc)
			return 1;
	}
	else
	{
		int i;
		unsigned char cks = 0;
		for (i = 0; i < Size; ++i)
		{
			cks += pBuf[i];
		}
		if (cks == pBuf[Size])
			return 1;
	}

	return 0;
}
#endif


/*! \page xmodem_spec The Xmodem Protocol Specification

\verbatim

MODEM PROTOCOL OVERVIEW

1/1/82 by Ward Christensen. I will maintain a master copy of
this. Please pass on changes or suggestions via CBBS/Chicago
at (312) 545-8086, or by voice at (312) 849-6279.

NOTE this does not include things which I am not familiar with,
such as the CRC option implemented by John Mahr.

Last Rev: (none)

At the request of Rick Mallinak on behalf of the guys at
Standard Oil with IBM P.C.s, as well as several previous
requests, I finally decided to put my modem protocol into
writing. It had been previously formally published only in the
AMRAD newsletter.

Table of Contents
1. DEFINITIONS
2. TRANSMISSION MEDIUM LEVEL PROTOCOL
3. MESSAGE BLOCK LEVEL PROTOCOL
4. FILE LEVEL PROTOCOL
5. DATA FLOW EXAMPLE INCLUDING ERROR RECOVERY
6. PROGRAMMING TIPS.

-------- 1. DEFINITIONS.
<soh> 01H
<eot> 04H
<ack> 05H
<nak> 15H
<can> 18H

-------- 2. TRANSMISSION MEDIUM LEVEL PROTOCOL
Asynchronous, 8 data bits, no parity, one stop bit.

The protocol imposes no restrictions on the contents of the
data being transmitted. No control characters are looked for
in the 128-byte data messages. Absolutely any kind of data may
be sent - binary, ASCII, etc. The protocol has not formally
been adopted to a 7-bit environment for the transmission of
ASCII-only (or unpacked-hex) data , although it could be simply
by having both ends agree to AND the protocol-dependent data
with 7F hex before validating it. I specifically am referring
to the checksum, and the block numbers and their ones-
complement.
Those wishing to maintain compatibility of the CP/M file
structure, i.e. to allow modemming ASCII files to or from CP/M
systems should follow this data format:
* ASCII tabs used (09H); tabs set every 8.
* Lines terminated by CR/LF (0DH 0AH)
* End-of-file indicated by ^Z, 1AH. (one or more)
* Data is variable length, i.e. should be considered a
continuous stream of data bytes, broken into 128-byte
chunks purely for the purpose of transmission.
* A CP/M "peculiarity": If the data ends exactly on a
128-byte boundary, i.e. CR in 127, and LF in 128, a
subsequent sector containing the ^Z EOF character(s)
is optional, but is preferred. Some utilities or
user programs still do not handle EOF without ^Zs.
* The last block sent is no different from others, i.e.
there is no "short block".

-------- 3. MESSAGE BLOCK LEVEL PROTOCOL
Each block of the transfer looks like:
<SOH><blk #><255-blk #><--128 data bytes--><cksum>
in which:
<SOH> = 01 hex
<blk #> = binary number, starts at 01 increments by 1, and
wraps 0FFH to 00H (not to 01)
<255-blk #> = blk # after going thru 8080 "CMA" instr, i.e.
each bit complemented in the 8-bit block number.
Formally, this is the "ones complement".
<cksum> = the sum of the data bytes only. Toss any carry.

-------- 4. FILE LEVEL PROTOCOL

---- 4A. COMMON TO BOTH SENDER AND RECEIVER:

All errors are retried 10 times. For versions running with
an operator (i.e. NOT with XMODEM), a message is typed after 10
errors asking the operator whether to "retry or quit".
Some versions of the protocol use <can>, ASCII ^X, to
cancel transmission. This was never adopted as a standard, as
having a single "abort" character makes the transmission
susceptible to false termination due to an <ack> <nak> or <soh>
being corrupted into a <can> and canceling transmission.
The protocol may be considered "receiver driven", that is,
the sender need not automatically re-transmit, although it does
in the current implementations.

---- 4B. RECEIVE PROGRAM CONSIDERATIONS:
The receiver has a 10-second timeout. It sends a <nak>
every time it times out. The receiver's first timeout, which
sends a <nak>, signals the transmitter to start. Optionally,
the receiver could send a <nak> immediately, in case the sender
was ready. This would save the initial 10 second timeout.
However, the receiver MUST continue to timeout every 10 seconds
in case the sender wasn't ready.
Once into a receiving a block, the receiver goes into a
one-second timeout for each character and the checksum. If the
receiver wishes to <nak> a block for any reason (invalid
header, timeout receiving data), it must wait for the line to
clear. See "programming tips" for ideas
Synchronizing: If a valid block number is received, it
will be: 1) the expected one, in which case everything is fine;
or 2) a repeat of the previously received block. This should
be considered OK, and only indicates that the receivers <ack>
got glitched, and the sender re-transmitted; 3) any other block
number indicates a fatal loss of synchronization, such as the
rare case of the sender getting a line-glitch that looked like
an <ack>. Abort the transmission, sending a <can>

---- 4C. SENDING PROGRAM CONSIDERATIONS.

While waiting for transmission to begin, the sender has
only a single very long timeout, say one minute. In the
current protocol, the sender has a 10 second timeout before
retrying. I suggest NOT doing this, and letting the protocol
be completely receiver-driven. This will be compatible with
existing programs.
When the sender has no more data, it sends an <eot>, and
awaits an <ack>, resending the <eot> if it doesn't get one.
Again, the protocol could be receiver-driven, with the sender
only having the high-level 1-minute timeout to abort.

-------- 5. DATA FLOW EXAMPLE INCLUDING ERROR RECOVERY

Here is a sample of the data flow, sending a 3-block message.
It includes the two most common line hits - a garbaged block,
and an <ack> reply getting garbaged. <xx> represents the
checksum byte.

SENDER RECEIVER
times out after 10 seconds,
                        <--- <nak>
<soh> 01 FE -data- <xx> --->
                        <--- <ack>
<soh> 02 FD -data- xx   ---> (data gets line hit)
                        <--- <nak>
<soh> 02 FD -data- xx   --->
                        <--- <ack>
<soh> 03 FC -data- xx   --->
    (ack gets garbaged) <--- <ack>
<soh> 03 FC -data- xx   ---> <ack>
<eot>                   --->
                        <--- <ack>

-------- 6. PROGRAMMING TIPS.

* The character-receive subroutine should be called with a
parameter specifying the number of seconds to wait. The
receiver should first call it with a time of 10, then <nak> and
try again, 10 times.
After receiving the <soh>, the receiver should call the
character receive subroutine with a 1-second timeout, for the
remainder of the message and the <cksum>. Since they are sent
as a continuous stream, timing out of this implies a serious
like glitch that caused, say, 127 characters to be seen instead
of 128.

* When the receiver wishes to <nak>, it should call a "PURGE"
subroutine, to wait for the line to clear. Recall the sender
tosses any characters in its UART buffer immediately upon
completing sending a block, to ensure no glitches were mis-
interpreted.
The most common technique is for "PURGE" to call the
character receive subroutine, specifying a 1-second timeout,
and looping back to PURGE until a timeout occurs. The <nak> is
then sent, ensuring the other end will see it.

* You may wish to add code recommended by Jonh Mahr to your
character receive routine - to set an error flag if the UART
shows framing error, or overrun. This will help catch a few
more glitches - the most common of which is a hit in the high
bits of the byte in two consecutive bytes. The <cksum> comes
out OK since counting in 1-byte produces the same result of
adding 80H + 80H as with adding 00H + 00H.
\endverbatim
*/

}	// namespace HsmDeviceComm

