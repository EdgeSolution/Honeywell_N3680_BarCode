/*=================================================================================
  AsyncParser.cpp
//=================================================================================
   $Id: AsyncParser.cpp 156 2018-12-06 17:14:36Z e411776 $

Example source code provided as is. No warranties of any kind.
Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#include "stdafx.h"
#include "AsyncParser.h"
#include "DataParse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define TRACE_STATES 1
#ifdef TRACE_STATES
#define ATRACE TRACE
#else
#define ATRACE(...)
#endif

namespace HsmDeviceComm {

CAsyncParser::CAsyncParser(void)
: m_pCollector(NULL)
{
	InitializeCriticalSection(&m_lock);
	PurgeReceiver();
}

CAsyncParser::~CAsyncParser(void)
{
	PurgeReceiver();
}

///////////////////////////////////////////////////////////////////////////////
//! Empties all internal buffers
/*!
*/
void CAsyncParser::PurgeReceiver(void)
{
	::EnterCriticalSection(&m_lock);
	TRACE("CAsyncParser::PurgeReceiver start\r\n");
	delete m_pCollector;
	m_pCollector = NULL;
	m_sRawMessage.Empty();
	m_sLength.Empty();
	m_RxState= Idle;
	m_LastPercent = 0;
//	m_HModemMode=false;
	TRACE("CAsyncParser::PurgeReceiver end\r\n");
	::LeaveCriticalSection(&m_lock);
}

///////////////////////////////////////////////////////////////////////////////
//! Empties all internal buffers
/*!
*/
void CAsyncParser::PurgeCommandResponses(void)
{
	::EnterCriticalSection(&m_lock);
	TRACE("CAsyncParser::PurgeCommandResponses start\r\n");
	CMessageDetails *pKill;
	CStringA Response;
	do
	{
		pKill = ReadNextCommandResponse();
		if(pKill != NULL)
		{
			Response = pKill->GetRawPayloadData();
			int t=0;
		}
		delete pKill;
	}
	while (pKill != NULL);
	TRACE("CAsyncParser::PurgeCommandResponses end\r\n");
	::LeaveCriticalSection(&m_lock);
}

///////////////////////////////////////////////////////////////////////////////
//! Call to parse any received data for a HTag structure
/*! Parse received bytes for HTag structures and store the payload into
	 a CStringA which then gets stored (a pointer to it) into a fifo.
	 We can receive and handle a unlimited (well, memory!) number of HTag structures.
	 Call this function from the receiver thread everytime it has received data.
 @param *pReceived data pointer
 @param Length Amount of received bytes
 @return Type of data
*/
int CAsyncParser::ParseAsyncMessage(const UCHAR *pReceived, size_t Length)
{
	TRACE("CAsyncParser::ParseAsyncMessage %zu\r\n" ,Length);
	const size_t HMODEM_HEADER_SIZE=6+2;
	int RetVal=NeedMore;
	::EnterCriticalSection(&m_lock);
	for(size_t Cursor=0; Cursor<Length; Cursor++)
	{
		UCHAR CurrentByte = pReceived[Cursor];
		switch (m_RxState)
		{
		case Idle:
			ATRACE("Idle\n");
			switch ((int)CurrentByte)
			{
			case 0x16:
				ATRACE("-> WaitFE\n");
				m_RxState = WaitFE;	// proceed to expect the 0xFE
				if(!m_sRawMessage.IsEmpty())	// do not loose any bytes arrived before the protocolled info
				{
					StoreRawToFifo(m_sRawMessage);
					m_sRawMessage.Empty();
				}
				break;
			case '.':
			case '!':
			case '\n':
				ATRACE("   Idle (raw)\n");
				RetVal |= ReceivedRawData|ReceivedRawMessage;
				m_sRawMessage += CurrentByte;
				StoreRawToFifo(m_sRawMessage);
				m_sRawMessage.Empty();
				break;
			default:
				ATRACE("   Idle (raw)\n");
				RetVal |= ReceivedRawData;
				m_sRawMessage += CurrentByte;
				break;
			}
			break;

		case WaitFE:
			ATRACE("WaitFE\n");
			if (CurrentByte == 0xFE)
			{
				ATRACE("-> WaitLength\n");
				m_RxState = WaitLength;
				m_sLength.Empty();
			}
			else
			{
				ATRACE("-> Idle (raw)\n");
				m_RxState = Idle;
				m_sRawMessage += CurrentByte;
				RetVal |= ReceivedRawData;
			}
			break;

		case WaitLength:
			ATRACE("WaitLength\n");
			m_sLength += CurrentByte;
			if (m_sLength.GetLength() == 4+1)	// also wait for the CR
			{
				// translate the length (only works on little endian machines!)
				const char* pcTemp= m_sLength;
				ULONG *pLong = (ULONG*) pcTemp;
				m_HTagSize = *pLong;

				if(m_sLength[4] == 0x0d)	// some additional integrity test
				{
					ATRACE("-> WaitEndofMessage HS=%X\n", m_HTagSize);
					m_RxState = WaitEndofMessage;
				}
				else
				{
					ATRACE("-> Idle (raw)\n");
					m_RxState = Idle;
					m_sRawMessage += m_sLength;
					RetVal |= ReceivedRawData;
				}
			}
			break;

		case WaitEndofMessage:
			{
			ATRACE("WaitEndofMessage\n");
			GetCollector();		// alloc a string if not already done

				// add any bytes to the end of the collected data
				int OldSize = m_pCollector->GetLength();
				int AddSize = Length-Cursor;
				int NewSize = OldSize + AddSize;
				if(NewSize > m_HTagSize)
				{
					AddSize -= NewSize-m_HTagSize;	// fix the sizes, so we don't touch the next message yet
					NewSize = m_HTagSize;
				}
				RetVal |= Collect(&pReceived[Cursor], AddSize);
				if(NewSize == HMODEM_HEADER_SIZE)	// did we receive a header that tells us to use HModem?
				{
					// we better double check
					if (0 == m_pCollector->Find("IMG"))
					{
						if(m_pCollector->GetAt(HMODEM_HEADER_SIZE-1)=='P')
						{
							char temp = m_pCollector->GetAt(HMODEM_HEADER_SIZE-1-1);
							if(temp=='3' || temp=='4')
							{
								m_RxState = Idle;
								RetVal |= UseHModemRx;
							}
						} // if(m_pCollector->GetAt(HMODEM_HEADER_SIZE)=='P')
					}
				}
				ATRACE("HS=%X, NS=%X, AS=%X\n", m_HTagSize, NewSize, AddSize);
				if(NewSize >= m_HTagSize)	// we received a full message, push it into the fifo
				{
					ATRACE("FinalizeCollector\n");
					RetVal |= FinalizeCollector();
				}
				Cursor += AddSize;
			}
			break;
		
		default:
			ASSERT(0);
			m_RxState = Idle;
			m_sRawMessage += CurrentByte;
			RetVal |= ReceivedRawData;
		break;
		};
	} // for(size_t Cursor=0; Cursor<Length; Cursor++)
	
//	if((RetVal & ReceivedRawData) && !m_sRawMessage.IsEmpty())
//	{		
//		StoreToFifo(m_sRawMessage);
//		m_sRawMessage.Empty();
//	}

	::LeaveCriticalSection(&m_lock); 
	return RetVal;
}

#ifdef UNICODE
///////////////////////////////////////////////////////////////////////////////
//! Call to parse any received data for a HTag structure
/*! Parse received bytes for HTag structures and store the payload into 
	 a CStringA which then gets stored (a pointer to it) into a fifo.
	 We can receive and handle a unlimited (well, memory!) number of HTag structures.
	 Call this function from the receiver thread everytime it has received data.
 @param *pReceived data pointer
 @param Length Amount of received bytes
 @return Type of data
*/
int CAsyncParser::ParseAsyncUTF8Message(const UCHAR *pReceived, size_t Length)
{
	const size_t HMODEM_HEADER_SIZE=6+2;
	int RetVal=NeedMore;
	::EnterCriticalSection(&m_lock); 
	for(size_t Cursor=0; Cursor<Length; Cursor++)
	{
		UCHAR CurrentByte = pReceived[Cursor];
		switch (m_RxState)
		{
		case Idle:
			switch (CurrentByte)
			{
			case 0x16:
				m_RxState = WaitC3;	// proceed to expect the 0xFE
				break;
			case '.':
			case '!':
				RetVal |= ReceivedRawData|ReceivedRawMessage;
				m_sRawMessage += CurrentByte;
				StoreRawToFifo(m_sRawMessage);
				m_sRawMessage.Empty();
				break;
			default:
				RetVal |= ReceivedRawData;
				m_sRawMessage += CurrentByte;
				break;
			}
			break;
		
		case WaitC3:	// the 0xFE is two bytes in UTF8: 0xC3,0xBE
			if (CurrentByte == 0xC3)
			{
				m_RxState = WaitBE;
			}
			else
			{
				m_RxState = Idle;
				m_sRawMessage += CurrentByte;
				RetVal |= ReceivedRawData;
			}
			break;
		
		case WaitBE:
			if (CurrentByte == 0xBE)
			{
				m_RxState = WaitLength;
				m_sLength.Empty();
			}
			else
			{
				m_RxState = Idle;
				m_sRawMessage += CurrentByte;
				RetVal |= ReceivedRawData;
			}
			break;

		case WaitLength:
			m_sLength += CurrentByte;
			if(CurrentByte == 0x0d)
			{
				CString sLength = CDataParse::Utf8ToUtf16(m_sLength);
				if (sLength.GetLength() == 4+1)	// also wait for the CR
				{
					// translate the length
					m_HTagSize = sLength[0] + (sLength[1]<<8) + (sLength[2]<<16) + (sLength[3]<<24);
					if(sLength[4] == 0x0d)	// some additional integrity test
					{
						m_RxState = WaitEndofMessage;
					}
					else
					{
						m_RxState = Idle;
						m_sRawMessage += m_sLength;
						RetVal |= ReceivedRawData;
					}
				}
			}
			break;
		
		case WaitEndofMessage:
			{
				GetCollector();		// alloc a string if not already done

				// add any bytes to the end of the collected data
				int OldSize = m_pCollector->GetLength();
				int AddSize = Length-Cursor;
				int NewSize = OldSize + AddSize;
				if(NewSize > m_HTagSize)
				{
					AddSize -= NewSize-m_HTagSize;	// fix the sizes, so we don't touch the next message yet
					NewSize = m_HTagSize;
				}
				RetVal |= Collect(&pReceived[Cursor], AddSize);
				if(NewSize == HMODEM_HEADER_SIZE)	// did we receive a header that tells us to use HModem?
				{
					// we better double check
					if (0 == m_pCollector->Find("IMG"))
					{
						if(m_pCollector->GetAt(HMODEM_HEADER_SIZE-1)=='P')
						{
							char temp = m_pCollector->GetAt(HMODEM_HEADER_SIZE-1-1);
							if(temp=='3' || temp=='4')
							{
								m_RxState = Idle;
								RetVal |= UseHModemRx;
							}
						} // if(m_pCollector->GetAt(HMODEM_HEADER_SIZE)=='P')
					}
				}
				if(NewSize >= m_HTagSize)	// we received a full message, push it into the fifo
				{
					FinalizeCollector();
					RetVal |= ReceivedHTag;	// flag that we've got a valid data message
				}
				Cursor += AddSize;
			}
			break;
		
		default:
			ASSERT(0);
			m_RxState = Idle;
			m_sRawMessage += CurrentByte;
			RetVal |= ReceivedRawData;
		break;
		};
	} // for(size_t Cursor=0; Cursor<Length; Cursor++)
	
//	if((RetVal & ReceivedRawData) && !m_sRawMessage.IsEmpty())
//	{		
//		StoreToFifo(m_sRawMessage);
//		m_sRawMessage.Empty();
//	}

	::LeaveCriticalSection(&m_lock); 
	return RetVal;
}
#endif

///////////////////////////////////////////////////////////////////////////////
//! Push the collected data into the fifo
/*! 
 @return int Type of packet
*/
int CAsyncParser::FinalizeCollector(void)
{
	TRACE("CAsyncParser::FinalizeCollector\r\n");
	int RetVal=0;

	m_pCollector->ParseMessage();
	if(m_pCollector->IsCmdResponse())
	{
		TRACE("CAsyncParser::FinalizeCollector Cmd\r\n");
		m_CmdRspFifo.Write(m_pCollector);
		RetVal = ReceivedCommandResponse;
	}
	else
	{
		TRACE("CAsyncParser::FinalizeCollector Msg\r\n");
		m_RxFifo.Write(m_pCollector);
		RetVal = ReceivedHTag;	// flag that we've got a valid data message
	}
	m_pCollector=NULL;
	m_RxState = Idle;
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Push the bad image into the fifo
/*! 
 @return int Type of packet
*/
int CAsyncParser::SetBadImage(void)
{
//	TRACE("CAsyncParser::SetBadImage\r\n");
	m_pCollector->ParseMessage();
	m_pCollector->SetBadImage();
	m_RxFifo.Write(m_pCollector);
	m_pCollector=NULL;
	m_RxState = Idle;
	return HModemError;
}

///////////////////////////////////////////////////////////////////////////////
//! Retrieve a command response from the parsed data.
/*! 
 @param [out] sResponse 
 @return bool true if there is a response available
*/
bool CAsyncParser::GetCommandResponse(CStringA &sResponse)
{
	TRACE("CAsyncParser::GetCommandResponse\r\n");
	bool RetVal=false;
	CMessageDetails *pCommandResponse = ReadNextCommandResponse();
	if((pCommandResponse!=NULL) && pCommandResponse->IsCmdResponse())
	{
		sResponse = pCommandResponse->GetRawPayloadData();
		if(sResponse.IsEmpty())
		{
			int x=0;
		}

		delete pCommandResponse;
		RetVal = true;
	}
	else
	{
		int y=0;
	}

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Prepare a valid collector
/*! 
 @return A pointer to a CStringA object that receives the data
*/
CStringA *CAsyncParser::GetCollector()
{
	if (m_pCollector == NULL)	// alloc a string if not already done
	{
		m_pCollector = new CMessageDetails;
		m_pCollector->SetBufferSize(m_HTagSize);
	}

	return m_pCollector;
}

int CAsyncParser::Collect(const unsigned char *pReceived, int AddSize)
{
	int RetVal = 0;
	ASSERT(m_HTagSize != 0);
	ASSERT(AddSize <= m_HTagSize);
	m_pCollector->Collect((const char *) pReceived, AddSize);

	// calc some progress
	int CurrentSize = m_pCollector->GetLength();
	int Percent = (CurrentSize * 100) / m_HTagSize;
	const int Steps = 4;
	if (Percent - m_LastPercent > Steps)
	{
		RetVal = ReceivedImagePortion;
		m_LastPercent += Steps;
	}

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Return the size of the HTag that will be received via the HModem protocoll
/*! 
 @return Amount of data to be expected
*/
size_t CAsyncParser::GetExpectedBufferSize(void)
{
	return m_HTagSize;
}

///////////////////////////////////////////////////////////////////////////////
//! Stores the received bytes into a CStringA and the pointer of it to the fifo.
/*! 
 @param *pReceived Pointer the received bytes
 @param Length Amount of the received bytes
 @return always true
*/
bool CAsyncParser::StoreRawToFifo(const UCHAR *pReceived, size_t Length)
{
	CMessageDetails *pReceivedMessage = new CMessageDetails((const char*)pReceived, (int)Length);
	return StoreRawToFifo(pReceivedMessage);
}

///////////////////////////////////////////////////////////////////////////////
//! Stores the received bytes into a CStringA and the pointer of it to the fifo.
/*! 
 @param sReceived A CStringA containing the received bytes
 @return always true
*/
bool CAsyncParser::StoreRawToFifo(const CStringA sReceived)
{
	CMessageDetails *pReceivedMessage = new CMessageDetails(sReceived);
	return StoreRawToFifo(pReceivedMessage);
}

///////////////////////////////////////////////////////////////////////////////
//! Stores the received bytes into a CStringA and the pointer of it to the fifo.
/*! 
 @param pReceivedMessage A CMessageDetails containing the received bytes
 @return always true
*/
bool CAsyncParser::StoreRawToFifo(CMessageDetails *pReceivedMessage)
{
	pReceivedMessage->SetCmdResponse();
	m_RxFifo.Write(pReceivedMessage);
	return true;
}

}	// namespace HsmDeviceComm
