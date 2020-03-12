/*=================================================================================
  HModem.cpp
//=================================================================================
   $Id: HModem.cpp 156 2018-12-06 17:14:36Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

// not tested and not working yet!!!
#include "stdafx.h"
#include "HModem.h"
#include "DeviceComm.h"

namespace HsmDeviceComm {

#define     HEADER_BLOCK_128_BYTE                   0
#define     DATA_BLOCK_128_BYTE                     1
#define     DATA_BLOCK_1024_BYTE                    2
#define     RECOVERY_HEADER_BLOCK_BYTE              3
#define		EOT												 4

#define BIG_ENDIAN  0   /* Intel processors are little endian    */

#if BIG_ENDIAN
  #define CRCLOW    *crcp       /* IBM-PC is big endian         */
  #define CRCHIGH   *(crcp+1)
#else
  #define CRCLOW    *(crcp+1)   /* Motorola is little endian    */
  #define CRCHIGH   *crcp
#endif



unsigned short calc_crc(unsigned char *ptr, int count)
{
int	crc, i;

	crc = 0;
	while(--count >= 0)	{
		crc = crc ^ (int)*ptr++ << 8;
		for(i = 0; i < 8; ++i)
			if(crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			else
				crc = crc << 1;
	}

	return (((unsigned short) (crc & 0xFFFF)));
}   /*  calc_crc    */



CHModem::CHModem(CDeviceComm *pComm)
{
	m_current_blocksize=0;
	m_LastSuccessBlockNumber=0;
	m_pComm = pComm;
}

CHModem::~CHModem(void)
{
}

void	CHModem::SendAck(void)
{
	m_pComm->Write(0x06);
}

void	CHModem::SendNak(void)
{
	do
	{	
		m_pComm->PurgeReceive();
		Sleep(100);
	}while (-1 != m_pComm->Read());

	m_pComm->Write(0x15);
}

void	CHModem::PrepareReceive(void)
{
	m_pComm->Write('C');	// tell we are ready
	m_FirstEOT=true;
	m_FirstRecovery=true;
	m_current_blocksize=0;
	m_LastSuccessBlockNumber=0;
}

bool CHModem::Receive(CStringA *pCollector, size_t ExpectedBufferSize)
{
	int RetVal=Aborted;
	for(int i=0; i<10; i++)
	{
		if(RetVal == Aborted)
			PrepareReceive();

		RetVal = ReceiveStartByte();
		if (RetVal==GotStart)
			break;

		if(RetVal!=Aborted)
			m_pComm->ShowProgress();
	}

	size_t InitialLength = pCollector->GetLength();
	m_ReceivedSoFar = InitialLength;
	m_pReceiveBuffer = (UCHAR*)pCollector->GetBuffer((int)ExpectedBufferSize+1024);
	do
	{
		if(RetVal != GotStart)
			RetVal = ReceiveStartByte();

		if((RetVal!=GotFirstEOT)&&(RetVal!=GotEOT)&&(RetVal!=Aborted))
			RetVal = ReceiveBlock();

		if(RetVal!=Aborted)
			m_pComm->ShowProgress();

	}while((RetVal!=GotEOT)&&(RetVal!=Aborted));
	pCollector->ReleaseBuffer((int)m_ReceivedSoFar);
	return (RetVal==GotEOT);
}

int CHModem::ReceiveStartByte(void)
{
	int ret=GotStart;

	// receive start byte
	m_CurrentBlockType = m_pComm->Read();
	switch(m_CurrentBlockType)
	{
	case HEADER_BLOCK_128_BYTE:
//		m_current_blocksize=128;
//		SendAck();	// we must send the ACK immediately after the header!
		SendNak();						// not supported yet, so we NAK
		ret = SentNak;
		break;
	case DATA_BLOCK_128_BYTE:
		m_current_blocksize=128;
		break;
	case DATA_BLOCK_1024_BYTE:
		m_current_blocksize=1024;
		break;
	case RECOVERY_HEADER_BLOCK_BYTE:
		if(m_FirstRecovery)
		{
			SendNak();
			m_FirstRecovery=false;
			ret = SentNak;
		}
		else
		{
			SendAck();
			m_FirstRecovery=true;
			ret = GotEOT;
		}
		break;
	case EOT:
		if(m_FirstEOT)
		{
			SendNak();
			m_FirstEOT=false;
			ret = GotFirstEOT;
		} // if(m_FirstEOT)
		else
		{
			SendAck();
			ret = GotEOT;
		}
		break;
	case 0xffffffff:	// timeout
		ret = Aborted;
		break;
	default:
		SendNak();
		ret = SentNak;
		break;
	} // switch(CurrentByte)

	return ret;
}

int CHModem::ReceiveBlock()
{
	UINT CurrentByte;
	UINT BlockNumber;
	UCHAR *pReceiveBuffer=NULL;
	pReceiveBuffer = m_pReceiveBuffer+m_ReceivedSoFar;


	// receive Block number
	BlockNumber = m_pComm->Read();
	CurrentByte = m_pComm->Read();
	// check for timeout
	if((BlockNumber == 0xffff) || (CurrentByte == 0xffff))
	{
		SendNak();
		return SentNak;
	}
	// check for valid numbers
	if((BlockNumber&0xff) != (~CurrentByte&0xff))
	{
		SendNak();
		return SentNak;
	}
	// ensure sequence of packets
	if(((m_LastSuccessBlockNumber+1)&0xff) != (BlockNumber&0xff))
	{
		// allow same block again, we will ignore it later
		if((m_LastSuccessBlockNumber&0xff) != (BlockNumber&0xff))
		{
			SendNak();
			return SentNak;
		}
	}
	
	// receive payload
	int Received=0;
	// todo: timeout !!!
	do
	{
		Received = m_pComm->Read(pReceiveBuffer+Received, m_current_blocksize-Received);
	}while (Received<m_current_blocksize);

	unsigned short crc =	calc_crc(pReceiveBuffer, m_current_blocksize); 
	bool CrcOk = (m_pComm->Read() == (crc>>8));
	CrcOk		 &= (m_pComm->Read() == (crc&0xff));
	if(!CrcOk)
	{
		SendNak();
		return SentNak;
	} // if(!CrcOk)

	if(m_CurrentBlockType == HEADER_BLOCK_128_BYTE)
	{
		SendNak();	// later we use this info and parse it, now we say NAK
		return SentNak;
	}

	SendAck();
	// ignore an already successfull received packet if it appears a second time
	if((m_LastSuccessBlockNumber&0xff) == (BlockNumber&0xff))
	{
		return Ignore;
	}
	m_LastSuccessBlockNumber = BlockNumber;
	if((m_CurrentBlockType == DATA_BLOCK_128_BYTE) || (m_CurrentBlockType == DATA_BLOCK_1024_BYTE))
	{
		m_ReceivedSoFar += m_current_blocksize;
	}

	return GotBlock;
}

}	// namespace HsmDeviceComm
