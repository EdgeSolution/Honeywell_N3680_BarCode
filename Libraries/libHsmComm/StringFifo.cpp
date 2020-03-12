/*=================================================================================
  StringFifo.cpp
//=================================================================================
   $Id: StringFifo.cpp 156 2018-12-06 17:14:36Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#include "stdafx.h"
#include "StringFifo.h"
#include "MessageDetails.h"

namespace HsmDeviceComm {

///////////////////////////////////////////////////////////////////////////////
//! simple d-tor
/*! Clean up everything
*/
CStringFifoA::~CStringFifoA(void)
{
	// remove all strings in the fifo
	CStringA sString;
	do
	{
		sString = Read();
    } while (!sString.IsEmpty());
}

///////////////////////////////////////////////////////////////////////////////
//! simple d-tor
/*! Clean up everything
*/
CStringFifoW::~CStringFifoW(void)
{
	// remove all strings in the fifo
	CStringW sString;
	do
	{
		sString = Read();
    } while (!sString.IsEmpty());
}

///////////////////////////////////////////////////////////////////////////////
//! simple d-tor
/*! Clean up everything
*/
CMessageFifoA::~CMessageFifoA(void)
{
	// remove all strings in the fifo
	CMessageDetails *pString=NULL;
	do
	{
		pString = Read();
		delete pString;
	}while(pString!=NULL);
}

}	// namespace HsmDeviceComm
