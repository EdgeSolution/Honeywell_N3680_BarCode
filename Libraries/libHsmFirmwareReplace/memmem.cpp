/*=================================================================================
  memmem.cpp
//=================================================================================
   $Id: memmem.cpp 202 2019-01-31 12:13:16Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// memmem.cpp : implementation file
//

#include "stdafx.h"
#include "memmem.h"

namespace HsmDeviceComm {

///////////////////////////////////////////////////////////////////////////////
//! This is a strstr() look-alike for non-text buffers.
/*! This overload uses const char* pointers.
 @param *Buf Binary data Buffer
 @param *Pattern Search pattern
 @param BufLen Size of buffer
 @param PatternLen Size of search pattern
 @return Address of found pattern or NULL if not found
*/
const char *memmem(const char *Buf, const char *Pattern, size_t BufLen, size_t PatternLen)
{
	int FirstByte = *Pattern;
	const char *pSearch = Buf;

	while (PatternLen <= (BufLen - (pSearch - Buf)))
	{
		// search for the first byte of pattern
		pSearch = (const char *)memchr(pSearch, FirstByte, BufLen - (pSearch - Buf));
		if (pSearch != NULL)
		{
			// we found the first byte, lets check for the complete pattern
			if (memcmp(pSearch, Pattern, PatternLen) == 0)
				return pSearch;	// found the pattern, return the found address
			else
				pSearch++;			// no match, proceed in searching
		}
		else
		{
			break;
		}
	}
	return NULL;
}

}	// namespace HsmDeviceComm
