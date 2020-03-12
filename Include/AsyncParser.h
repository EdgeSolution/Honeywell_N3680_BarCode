/*=================================================================================
  AsyncParser.h
//=================================================================================
   $Id: AsyncParser.h 155 2018-12-06 11:20:57Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#pragma once

#include "StringFifo.h"
#include "MessageDetails.h"

namespace HsmDeviceComm {

/** Parses received data for a HTag block from the device.
 This class collects and maintains the received data packets. 
 It also contains the fifo for the received packets.
 
 
 * \ingroup Helpers
 *
 *
 * \par requirements
 * MFC\n
 *
 */
class CAsyncParser
{
public:
	CAsyncParser(void);
	~CAsyncParser(void);
	
	///////////////////////////////////////////////////////////////////////////////
	//! Reads a received device message out of the fifo.
	/*! 
	 @return CMessageDetails * It is the callers responsibility to delete the object.
	*/
	CMessageDetails *ReadRxFifo(void)	{ return m_RxFifo.Read();	}

	///////////////////////////////////////////////////////////////////////////////
	//! Reads a received command response out of the fifo.
	/*! 
	 @return CMessageDetails * It is the callers responsibility to delete the object.
	*/
	CMessageDetails *ReadNextCommandResponse(void)	{ return m_CmdRspFifo.Read();	}
	
	//! Empties all internal buffers
	void PurgeReceiver(void);
	void PurgeCommandResponses(void);
	
	//! Call to parse any received data for a HTag structure
	int ParseAsyncMessage(const UCHAR *pReceived, size_t Length);
#ifdef UNICODE
	int ParseAsyncUTF8Message(const UCHAR *pReceived, size_t Length);
#endif
	bool StoreRawToFifo(const UCHAR *pReceived, size_t Length);
	bool StoreRawToFifo(const CStringA sReceived);
	bool StoreRawToFifo(CMessageDetails *pReceivedMessage);

	//! used by ParseHTag to return usefull information
	enum ReturnStates_t
	{
		NeedMore=0x1, ReceivedHTag=0x2, UseHModemRx=0x4,
		ReceivedRawData=0x8, ReceivedRawMessage=0x10,
		ReceivedCommandResponse = 0x20,
		ReceivedImagePortion = 0x40,
		HModemError=0x200
	};

	//! Used by the HModem receiver
	size_t GetExpectedBufferSize(void);
	//! Used by the HModem receiver
	CStringA *GetCollector();
	//! Used by the HModem receiver
	int FinalizeCollector(void);
	int Collect(const unsigned char *pReceived, int AddSize);
	int SetBadImage(void);

	bool GetCommandResponse(CStringA &sResponse);

protected:
	enum RxStates_t	//!< States used by the reeiving state machine
	{
		Idle, WaitFE, WaitC3, WaitBE, WaitLength, WaitHModemPacket, WaitEndofMessage
	};
	RxStates_t m_RxState;					//!< State for state machine
	CStringA m_sLength;						//!< Collects the received length until we can decode it
	int	m_HTagSize;						//!< Size of the currently received device message
	int m_LastPercent;

	CMessageDetails m_sRawMessage;		//!< Collects the received message if we do not find any tag
	CMessageDetails *m_pCollector;		//!< Collects the received message
	CMessageFifoA m_CmdRspFifo;						//!< A fifo for all received command responses
	CMessageFifoA m_RxFifo;								//!< A fifo for all received device messages
	CRITICAL_SECTION m_lock;							//!< Protect access to members
};

}	// namespace HsmDeviceComm
