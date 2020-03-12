/*=================================================================================
  CommLib.h
//=================================================================================
   $Id: CommLib.h 203 2019-01-31 16:41:15Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

#pragma once

//#include "PortManGui.h"
#include "AndroidUsbManager.h"
#include "PortFunctor.h"

///////////////////////////////////////////////////////////////////////////////
/** Interface to the communication library.
 * This helps to keep the code more portable.
 * \ingroup Helpers
 *
 * \par requirements
 * MFC\n
 *
 *
 */

#define CommLibBase CUsbManager
class CCommLib : public CommLibBase
{
public:
	CCommLib(HWND hNotifictionReceiver=NULL, DWORD MsgStatus=0);
	virtual ~CCommLib(void);

	using CommLibBase::Connect;
    using CommLibBase::Disconnect;
    using CommLibBase::Write;
	using CommLibBase::Read;

    ///////////////////////////////////////////////////////////////////////////////
    //! Send a string to device.
    /*!
     @param sText String to be send to device
     @return true for success
    */
    bool Write(CStringA str)
    {
        //TRACE(":; %s\r",(const char*)sText);
        return Write((const char*) str, str.GetLength());
    }

// dummies
    bool OnOptions()     { return false;  }
    bool OnPorts()       { return false;  }
    bool DoBorrowPort() { return false; }

};

inline bool PostMessage(HWND, DWORD, WPARAM, LPARAM)  { return false; };
