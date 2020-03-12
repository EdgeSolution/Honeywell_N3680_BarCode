/*=================================================================================
  ImageFifo.cpp
//=================================================================================
   $RCSfile: Helpers/ImageFifo.cpp $
   $Author: Fauth, Dieter $
   $Date: 2015-04-29 11:52:52 +0200 (Mi, 29 Apr 2015) $
   $Revision: 147 $

Copyright (C) Hand Held Products, Inc. 2005
//=================================================================================*/
//! \file

#include "stdafx.h"
#include "HsmComm.h"
#include "ImageFifo.h"
#include "ImageHandler.h"


namespace HsmDeviceComm
{

//CImageFifo::CImageFifo(void)
//{
//}

///////////////////////////////////////////////////////////////////////////////
//! simple d-tor
/*! Clean up everything. We delete all the contents of the fifo, so you do not 
//  need to delete it elsewere.
*/
    CImageFifo::~CImageFifo(void)
    {
        // remove all strings in the fifo
		CImageHandler *pImage=NULL;
        do
        {
            pImage = Read();
            delete pImage;
        } while (pImage != NULL);
    }

}	// namespace HsmDeviceComm