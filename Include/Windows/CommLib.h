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

#include "PortManGui.h"
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

#define CommLibBase CPortManGui
class CCommLib : public CommLibBase
{
public:
	CCommLib(HWND hNotifictionReceiver=NULL, DWORD MsgStatus=0);
	virtual ~CCommLib(void);

	bool Connect()
	{
		return OpenConnection();
	}

	bool Disconnect()
	{
		return CloseConnection();
	}

	virtual void PrepareDisconnect();
	virtual bool IsUSBConnected();
	virtual void InitRxTx();
	virtual bool IsSecureConnection();

	using CommLibBase::Write;

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
};

