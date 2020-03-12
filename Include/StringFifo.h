/*=================================================================================
  StringFifo.h
//=================================================================================
   $Id: StringFifo.h 156 2018-12-06 17:14:36Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
#pragma once

#include <queue>

namespace HsmDeviceComm {

class CMessageDetails;

/** A simple fifo for CString pointers
 */
class CStringFifoA : private std::queue<CStringA>
{
public:
	//	CMessageFifoA(void);
	~CStringFifoA(void);
public:
	// Go to the end of the line
	void Write(CStringA newString)
	{
		push(newString);
	}        // End of the queue

// Get first element in line
	CStringA Read()
	{
		CStringA RetVal;
		if (empty())
		{
			RetVal = "";
		}
		else
		{
			RetVal = front();
			pop();
		}
		return RetVal;
	}
};

class CStringFifoW : private std::queue<CStringW>
{
public:
	//	CMessageFifWA(void);
	~CStringFifoW(void);
public:
	// Go to the end of the line
	void Write(CStringW newString)
	{
		push(newString);
	}        // End of the queue

// Get first element in line
	CStringW Read()
	{
		CStringW RetVal;
		if (empty())
		{
			RetVal = L"";
		}
		else
		{
			RetVal = front();
			pop();
		}
		return RetVal;
	}
};

/** A simple fifo for CMessageDetails pointers
 * \ingroup Helpers
 *
 */
class CMessageFifoA : private std::queue<CMessageDetails*>
{
public:
//	CMessageFifoA(void);
	~CMessageFifoA(void);
public:
    // Go to the end of the line
    void Write( CMessageDetails* newString )
        { push( newString ); }        // End of the queue

    // Get first element in line
    CMessageDetails* Read()
    {
        CMessageDetails *RetVal;
        if (empty())
        {
            RetVal = NULL;
        } else
        {
            RetVal = front();
            pop();
        }
        return RetVal;
    }
};

}  // namespace HsmDeviceComm

