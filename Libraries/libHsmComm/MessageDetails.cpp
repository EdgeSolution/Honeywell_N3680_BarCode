/*=================================================================================
  MessageDetails.cpp
//=================================================================================
   $Id: MessageDetails.cpp 163 2018-12-11 13:39:55Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#include "stdafx.h"
#include "MessageDetails.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace HsmDeviceComm {

///////////////////////////////////////////////////////////////////////////////
//! The default constructor
/*! 
*/
CMessageDetails::CMessageDetails(void)
: CStringA()
{
	Reset();
}

///////////////////////////////////////////////////////////////////////////////
//! A copy constructor for CStringA
/*! 
 @param sString A CStringA used for initializing
*/
CMessageDetails::CMessageDetails(CStringA sString)
: CStringA(sString)
{
	Reset();
}

///////////////////////////////////////////////////////////////////////////////
//! A copy constructor for pointer and size
/*! 
 @param *pText Pointer to data for initializing
 @param Length Amount of bytes used for initializing
*/
CMessageDetails::CMessageDetails(const char *pText, size_t Length)
: CStringA(pText, (int)Length)
{
	Reset();
}

CMessageDetails::~CMessageDetails(void)
{
}

///////////////////////////////////////////////////////////////////////////////
//! Sets all contents to invalid states
/*! 
*/
void CMessageDetails::Reset(void)
{
	SetInvalid();
	m_HH_Id='?';
	m_AIM_Id='?';
	m_AIM_Modifier=0;
	m_PayloadStart=-1;
	m_PayloadLength=0;
	m_BufferSize = 0;
	m_type = Unknown;
	m_AckByte = ' ';
}

///////////////////////////////////////////////////////////////////////////////
//! Extract all details out of an HTagged message and treat it as text
/*! 
*/
void CMessageDetails::SetTextFromMsgGet(void)
{
	const int MSGGET_LENGTH = 14;
	m_PayloadStart = MSGGET_LENGTH;
	CStringA sLength = Mid(6, 4);	// get the length string
	m_PayloadLength = atoi(sLength);
	m_HH_Id=GetAt(10);
	m_AIM_Id=GetAt(11);
	m_AIM_Modifier=GetAt(12);
	m_type = Text;
	SetValid();
}

#ifdef _MFC_VER
///////////////////////////////////////////////////////////////////////////////
//! Extract all details out of an HTagged message and treat it as text
/*!
*/
void CMessageDetails::SetTextFromMsg00X(void)
{
	const char GS = 0x1D;
	const int START = 6;
	char codepage = GetAt(5);

	int EndOfHeader = Find(GS, START);
	if (EndOfHeader > 0)
	{
		m_PayloadStart = EndOfHeader + 1;
		m_PayloadLength = GetLength() - m_PayloadStart;
		ASSERT(m_PayloadLength > 0);

		CStringA sDescriptor;
		sDescriptor = Mid(START, EndOfHeader - START);

		int curPos = 0;
		while (curPos >= 0)
		{
			CStringA sParameter = sDescriptor.Tokenize("\"", curPos);
			if (curPos < 0)
				break;
			CStringA sTag = sDescriptor.Tokenize("\"", curPos);
			if (curPos < 0)
				break;

			if (sTag == "ID")
			{
				m_HH_Id = sParameter.GetAt(0);
				m_AIM_Id = sParameter.GetAt(1);
				m_AIM_Modifier = sParameter.GetAt(2);
			}
			else // if(sTag=="ADLER")	// future expansions
			{
			}
		}
	}

	if (m_HH_Id == '6')
	{
		m_type = CmdResponse;
	}
	else
	{
		m_type = Text;
	}

	SetValid();
}

#else	//_MFC_VER

///////////////////////////////////////////////////////////////////////////////
//! Extract all details out of an HTagged message and treat it as text
/*! 
*/
void CMessageDetails::SetTextFromMsg00X(void)
{
	const char *delimiter="\"";
    const char GS=0x1D;
	const int START=6;
	char codepage = GetAt(5);

	int EndOfHeader = Find(GS, START);
	if(EndOfHeader > 0)
	{
		m_PayloadStart = EndOfHeader+1;
		m_PayloadLength = GetLength()-m_PayloadStart;
		ASSERT(m_PayloadLength > 0);

		CStringA sDescriptor;
		sDescriptor = Mid(START, EndOfHeader-START);

		int curPos= 1; // skip first "
		while (curPos!=std::string::npos)
		{
            curPos = sDescriptor.find(delimiter, curPos);
            int StartTag = curPos+1;
		    CStringA sParameter = sDescriptor.substr(1, curPos-1);
			if(curPos==std::string::npos)
				break;
            curPos = sDescriptor.find(delimiter, StartTag);
			CStringA sTag = sDescriptor.substr(StartTag, curPos);
			if(sTag=="ID")
			{
				m_HH_Id=sParameter.GetAt(0);
				m_AIM_Id=sParameter.GetAt(1);
				m_AIM_Modifier=sParameter.GetAt(2);
			}
			else // if(sTag=="ADLER")	// future expansions
			{
			}
		}
	}

	if(m_HH_Id == '6')
	{
		m_type=CmdResponse;
	}
	else
	{
		m_type = Text;
	}

	SetValid();
}

#endif	//_MFC_VER

///////////////////////////////////////////////////////////////////////////////
//! Extract all details out of a raw message and check for a single byte response
/*! 
*/
//void CMessageDetails::SetByteMessage(void)
//{
//	m_PayloadStart = 0;
//	m_PayloadLength = GetLength();
//	if (m_PayloadLength == 1)
//	{
//		m_type = ByteResponse;
////////////////////	}
//	else
//	{
//		m_type = Unknown;
//////	}
//	SetValid();
//}

///////////////////////////////////////////////////////////////////////////////
//! Extract all details out of a raw message and treat it as unknown
/*! 
*/
void CMessageDetails::SetRawMessage(void)
{
	m_PayloadStart = 0;
	m_PayloadLength = GetLength();
	m_type = Unknown;
	SetValid();
}


///////////////////////////////////////////////////////////////////////////////
//! Extract all details out of an HTagged message and treat it as an image
/*! 
*/
void CMessageDetails::SetImage(void)
{
	const char GS=0x1D;
	const int START=6;
	CStringA sNumber;
	int i;
	for(i=START; i<GetLength() && GetAt(i)!=GS; i++)
	{
		char temp = GetAt(i);
		if(isdigit(temp))
		{
			sNumber += temp;
		}
		else
		{
			if (!sNumber.IsEmpty())
			{
				int Number = atoi(sNumber);
				sNumber.Empty();
				switch(temp)
				{
				case 'D':
					m_PixelDepth = Number;
					break;
				case 'F':
					m_ImageFormat = Number;
					break;
				default:	// ignore the rest (for now)
					break;
				}
			}
		}
	} // for(i=START; i<sHTag.GetLength() && GetAt(i)!=GS; i++)
	m_PayloadStart = i+1;
	m_PayloadLength = GetLength()-m_PayloadStart;
	if(m_PayloadLength > 0)
	{
		m_type = Image;
		SetValid();
	}
	else
	{
		m_type = BadImage;
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Store that we received a broken image (HMODEM abort?)
/*! 
*/
void CMessageDetails::SetBadImage(void)
{
	m_type = BadImage;
}

///////////////////////////////////////////////////////////////////////////////
//! Parse the incomming message for details
/*! 
 @return 
*/
CMessageDetails::PayloadType_t CMessageDetails::ParseMessage(void)
{
	if(IsValid())	// already parsed the first time
		return m_type;
	
	if (IsCmdResponse())
		ParseCmdMessage();
	else
		ParseStructuredMessage();
	return m_type;
}

///////////////////////////////////////////////////////////////////////////////
//! Parse the contents descriptor of incomming message for details.
/*! 
 @return 
*/
int CMessageDetails::ParseStructuredMessage(void)
{
	Reset();
	switch(GetAt(0))
	{
	case 'M':
		if(Find("MSG00") == 0)
		{
			SetTextFromMsg00X();
		}
		else if(Find("MSGGET") == 0)
		{
			SetTextFromMsgGet();
		}
		break;
	case 'I':
		if(Find("IMGSHP") == 0)
		{
			SetImage();
		}
		break;
	default:
		SetRawMessage();
		break;
	} // switch(GetAt(0))

	return m_type;
}

int CMessageDetails::ParseCmdMessage(void)
{
	m_PayloadStart = 0;
	m_PayloadLength = GetLength();
	int AckPosition = m_PayloadLength-2;
	if(AckPosition<0)
		AckPosition=0;

	UCHAR AckByte = GetAt(AckPosition);
	switch(GetAt(AckPosition))
	{
	case 0x05:
	case 0x06:
	case 0x15:
		m_AckByte = AckByte;
		break;
	default:
		break;
	}

	SetValid();
	return m_type;
}

///////////////////////////////////////////////////////////////////////////////
//! 
/*! 
 @param sFileName 
 @return 
*/
int CMessageDetails::WritePayloadToFile(CString sFileName)
{
	int Written=0;
#if 0   // fixme
	if(IsValid())
	{
		CFile destFile;
		CFileException ex;

		BOOL RetVal = destFile.Open(sFileName, CFile::modeCreate|CFile::modeReadWrite, &ex);
		if(RetVal==TRUE)
		{
			char *pData;
			size_t Length;
			GetRawPayloadBuffer(pData, Length);
			destFile.Write(pData, (int)Length);
			Written = m_PayloadLength;
		}
		else
		{
		#ifdef _DEBUG
		#ifndef _WIN32_WCE
			afxDump << _T("File could not be opened ") << ex.m_cause << _T("\n");
		#endif
		#endif
		}
		destFile.Close();
	} // if(IsValid())
#endif
	return Written;
}

///////////////////////////////////////////////////////////////////////////////
//! Retrieve the current payload data as raw bytes.
/*! 
 Be carefull with this function. Payload data is copied into a CStringA. 
 This can cause performance problems if you use it for images etc.
 It is probably ok to use for text messages.
 @return CStringA with pure payload data
*/
CStringA CMessageDetails::GetRawPayloadData(void)
{
	if(IsValid())
	{
		ASSERT(m_PayloadStart >= 0);	
		ASSERT(m_PayloadLength >= 0);	
		ASSERT(m_PayloadStart <= GetLength());	
		ASSERT(m_PayloadLength <= GetLength());	
		return Mid(m_PayloadStart, m_PayloadLength);
	}
	return "";
}


///////////////////////////////////////////////////////////////////////////////
//! 
/*! I would like to use const for the pData, but the CXImage functions need a non-const pointer.
*/
bool CMessageDetails::GetRawPayloadBuffer(char *&pData, size_t &Length)
{
	if(IsValid())
	{
		pData = GetBuffer(GetLength());
		pData += m_PayloadStart;
		Length = m_PayloadLength;
	}
	return IsValid();
}

void CMessageDetails::Collect(const char *pReceived, int AddSize)
{
	ASSERT(m_BufferSize != 0);
	ASSERT(AddSize <= m_BufferSize);
	int OldSize = GetLength();
	ASSERT(OldSize+AddSize <= m_BufferSize);
	if (OldSize + AddSize > m_BufferSize)
		m_BufferSize = OldSize + AddSize;	// ensure there is no crash
	char *pDestination = GetBuffer(m_BufferSize);
	memcpy(&pDestination[OldSize], pReceived, AddSize);
	int NewSize = OldSize + AddSize;
	ReleaseBuffer(NewSize);
}

}	// namespace HsmDeviceComm

