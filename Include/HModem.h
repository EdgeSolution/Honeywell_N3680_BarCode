/*=================================================================================
  HModem.h
//=================================================================================
   $Id: HModem.h 155 2018-12-06 11:20:57Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#pragma once

namespace HsmDeviceComm {

class CDeviceComm;

class CHModem
{
public:
	CHModem(CDeviceComm *pComm);
	~CHModem(void);

	bool Receive(CStringA *pCollector, size_t ExpectedBufferSize);
protected:
	void	SendAck(void);
	void	SendNak(void);
	void	PrepareReceive(void);
	int ReceiveStartByte(void);
	int ReceiveBlock(void);
protected:
	enum
	{
		Aborted, Ignore, GotStart, GotBlock, SentNak, GotFirstEOT, GotEOT
	};
	CDeviceComm *m_pComm;								//!< Accessor for the comm lib
	int m_current_blocksize;							//!< size of actual received HModem block
	UINT m_CurrentBlockType;							//!< type of the current block
	int m_LastSuccessBlockNumber;						//!< The last successfully transfered block
	bool m_FirstEOT;										//!< NAK the first EOT
	bool m_FirstRecovery;								//!< NAK the first Recovery byte
	enum
	{
		BLOCKSIZE=1024
	};
	UCHAR m_HModemProtocollBuffer[BLOCKSIZE+1];	//!< For internal HModem blocks
	UCHAR *m_pReceiveBuffer;							//!< Points to the receive buffer
	size_t m_ReceivedSoFar;								//!< Keep track of received bytes
};

}	// namespace HsmDeviceComm
