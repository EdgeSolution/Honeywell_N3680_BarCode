// ============================================================================
// $Id: $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Brief.
	Details.
	\file */
//=============================================================================

#include "stdafx.h"
#include "HsmComm.h"
#include "ImageHandler.h"
#include "GraphicsMagick.h"
#include "ImageFormats.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace HsmDeviceComm
{

bool CImageHandler::s_NeedsInit = true;

CImageHandler::CImageHandler()
: m_pImage(NULL)
, m_hBitmap(NULL)
, m_HsmFormat(HH_FORMAT_INVALID)
, m_Length(0)
{
	if (s_NeedsInit)
	{
		Magick::InitializeMagick((char *)NULL);
		s_NeedsInit = false;
	}
}

CImageHandler::~CImageHandler()
{
	if (m_hBitmap)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	delete m_pImage;
	m_pImage = NULL;
}

bool CImageHandler::GetImageDetails(int &Format, size_t &Length, int &x, int &y)
{
	if (isValid())
	{
		Format = m_HsmFormat;
		Length = m_Length;
		x = m_pImage->columns();
		y = m_pImage->rows();
		return true;
	}
	return false;
}

bool CImageHandler::isValid()
{
	bool RetVal = false;
	if (m_Length && m_hBitmap && m_pImage && m_pImage->isValid() && GmImageHH::isValidFormat(m_HsmFormat))
	{
		RetVal = true;
	}
	return RetVal;
}

const TCHAR *CImageHandler::GetExtension()
{
	return GmImageHH::GetExtensionFromHsmFormat(m_HsmFormat);
}

bool CImageHandler::Write(CString sFileName)
{
	bool RetVal = true;
	CStringA sA(sFileName);
	try
	{
		m_pImage->write(std::string(sA));
	}
	catch (Magick::Exception &e)
	{
		UNUSED(e);
		RetVal = false;
	}
	return RetVal;
}

bool CImageHandler::ReadImage(CDeviceComm *pDevice)
{
	bool RetVal = false;
	UCHAR *pData=NULL;
	if(pDevice->GetRawPayloadBuffer(pData, m_Length))
	{
		m_HsmFormat = pDevice->GetImageFormat();
		if(GmImageHH::isValidFormat(m_HsmFormat))
		{
			if (m_pImage != NULL)
			{
				delete m_pImage;
				m_pImage = NULL;
			}
			try
			{
				// Blob creates a copy! 
				// FIXME: Try to find a way without the extra copy. see Blob::updateNoCopy.
				// Challenges are:
				//		Blob maintains the life cycle
				//		Image is inside a bigger buffer (HTAG header etc), 
				//			so we cannot simply pass the life cycle managment to Blob.
				// Can I use a class derived from Blob?
				Magick::Blob blob(pData, m_Length);
				m_pImage = new Magick::Image(blob);
			}
			catch (Magick::Exception &e)
			{
				UNUSED(e);
				RetVal = false;
			}

			if(m_pImage && m_pImage->isValid())
			{
				RetVal = true;
			}
		}
	}
	return RetVal;
}

HBITMAP CImageHandler::GetBitmap()
{
	if (m_hBitmap != NULL)	// remove old bitmap
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	if (m_pImage != NULL)
		m_hBitmap = (HBITMAP)ImageToHBITMAP(m_pImage->constImage());
	return m_hBitmap;
}

//----------------------------------------------------------------------------------
const CImageHandler& CImageHandler::operator= (CImageHandler &rhs)
{
	if (this != &rhs)
	{
		if (m_hBitmap != NULL)	// remove old bitmap
		{
			DeleteObject(m_hBitmap);
			m_hBitmap = NULL;
		}
		delete m_pImage;
		m_pImage = rhs.m_pImage;	// copies the image
		m_HsmFormat = rhs.m_HsmFormat;
		m_Length = rhs.m_Length;
	}
	return *this;
}

} // namespace HsmDeviceComm
