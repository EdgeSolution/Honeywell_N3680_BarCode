/*=================================================================================
  DataParse.cpp
//=================================================================================
   $Id: DataParse.cpp 156 2018-12-06 17:14:36Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#include "stdafx.h"
#include "DataParse.h"

//CDataParse::CDataParse(void)
//{
//}
//
//CDataParse::~CDataParse(void)
//{
//}


/**
 *	\brief Translates a string containing C-Esc sequences into it's binary form.
 * Example: "Hello\nagain\rand\x06"
 * @param sInput contains the input string with optional C-Esc sequences
 * @param *pDestination will receive the binary form
 * @param &size passes the size of the buffer and returns the used size
 * @return 
 */
BOOL CDataParse::GetBinaryString(CStringA sInput, char *pDestination, size_t &size)
{
   char *pHere=pDestination;
	const char *pSrc1 = sInput;
	const char *pSrc2 = sInput;
   size_t len=sInput.GetLength();
	int num;
   int numlen;
	size_t ReadSoFar=0;
	size_t ReadThisLoop;

	if(size < len)
	{
		size = len;
		return FALSE;
	}

	size = 0;
   while (NULL!=(pSrc2=strchr(pSrc1,'\\')))
   {
      ReadThisLoop = pSrc2-pSrc1;
		ReadSoFar += ReadThisLoop;
		memcpy(pHere, pSrc1, ReadThisLoop );
      pHere += ReadThisLoop;
      size += ReadThisLoop+1;

      numlen=2;
      switch (pSrc2[1])
      {
      case '\\':
			break;

      case 'r':
         *pHere = '\r';
         break;

      case 'n':
         *pHere = '\n';
         break;

      case 't':
         *pHere = '\t';
         break;

      case 'v':
         *pHere = '\v';
         break;

      case 'a':
         *pHere = '\a';
         break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
         if(ReadSoFar < len-3)
			{
				if(1 == sscanf_s(&pSrc2[1],"%3o",&num))
				{
					*pHere = (char)num;
					numlen += 2;
				}
			}
         break;

      case 'x':
         if(ReadSoFar < len-3)
			{
				if (1 == sscanf_s(&pSrc2[2],"%2x",&num))
				{
					*pHere = (char) num;
					numlen += 2;
				}
			}
         break;
		default:
         *pHere = '\\';
			break;
      }
      pHere++;
		pSrc1 = pSrc2 + numlen;
   }
	size += strlen(pSrc1);
	strcpy(pHere, pSrc1);	// MSVC2005 flags this as deprecated (warning). Will be fixed later.
   return TRUE;
}

#ifdef UNICODE

///////////////////////////////////////////////////////////////////////////////
//! Translate UTF8 to UTF16
/*! 
 @param sInput A CStringA containing UTF8 characters
 @return A CString containing UTF16LE characters
*/
CStringW CDataParse::Utf8ToUtf16(CStringA sInput)
{
	CStringW sDestination;
	int DestBufferLength = sInput.GetLength()* sizeof(wchar_t);
	wchar_t *pDest = sDestination.GetBufferSetLength(DestBufferLength);

	int size = MultiByteToWideChar(	CP_UTF8,
									MB_PRECOMPOSED,
									sInput,
									sInput.GetLength(),
									pDest,
									DestBufferLength);
	if (size == 0)
	{
		int test = GetLastError();
		TRACE("Translation to Unicode failed");
	}
	sDestination.ReleaseBufferSetLength(size);
	
	return sDestination;
}

#endif