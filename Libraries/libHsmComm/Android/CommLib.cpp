/*=================================================================================
  CommLib.cpp
//=================================================================================
   $Id: CommLib.cpp 158 2018-12-06 17:58:56Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#include "stdafx.h"
#include "CommLib.h"

CCommLib::CCommLib(HWND hNotifictionReceiver, DWORD MsgStatus)
: CommLibBase (hNotifictionReceiver, MsgStatus)
{
}

CCommLib::~CCommLib(void)
{
}
