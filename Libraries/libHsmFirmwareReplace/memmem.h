/*=================================================================================
  memem.h
//=================================================================================
   $Id: memmem.h 202 2019-01-31 12:13:16Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// memem.h : header file
//

#pragma once

namespace HsmDeviceComm {

const char *memmem(const char *Buf, const char *Pattern, size_t BufLen, size_t PatternLen);


///////////////////////////////////////////////////////////////////////////////
//! This is a strstr() look-alike for non-text buffers.
/*! This overload uses const void* pointers.
 @param *Buf Binary data Buffer
 @param *Pattern Search pattern
 @param BufLen Size of buffer
 @param PatternLen Size of search pattern
 @return Address of found pattern or NULL if not found
*/
inline const void *memmem(const void *Buf, const void *Pattern, size_t BufLen, size_t PatternLen)
{
	return memmem((const char *)Buf, (const char *)Pattern, BufLen, PatternLen);
}

}
