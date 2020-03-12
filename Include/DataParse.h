/*=================================================================================
  DataParse.h
//=================================================================================
   $Id: DataParse.h 156 2018-12-06 17:14:36Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#pragma once

/** A collection of parse helpers
 * \ingroup Helpers
 *
 *
 * \par requirements
 * MFC\n
 *
 */
class CDataParse
{
public:
//	CDataParse(void);
//	~CDataParse(void);
	static BOOL GetBinaryString(CStringA sInput, char *pDestination, size_t &size);
#ifdef UNICODE
	static CStringW Utf8ToUtf16(CStringA sInput);
#endif
};
