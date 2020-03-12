/*=================================================================================
  StringFifo.h
//=================================================================================
   $Id: ImageFifo.h 158 2018-12-06 17:58:56Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
#pragma once

#include <queue>

class CImageHandler;

namespace HsmDeviceComm
{

/** A simple fifo for CxImage pointers
*
 */
    class CImageFifo : private std::queue<CImageHandler *>
    {
    public:
//		CImageFifo(void);
        ~CImageFifo(void);

    public:
        // Go to the end of the line
        void Write(CImageHandler *newImage)
        { push(newImage); }        // End of the queue

        // Get first element in line
		CImageHandler *Read()
        {
			CImageHandler *RetVal;
            if (empty())
            {
                RetVal = NULL;
            } else
            {
                RetVal = front();
                pop();
            }
            return RetVal;
        };
    };

} // namespace HsmDeviceComm

