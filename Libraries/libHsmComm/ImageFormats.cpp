/*=================================================================================
  Images.cpp
//=================================================================================
   $Id: CxImage.cpp 249 2017-09-21 16:39:34Z Fauth, Dieter $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2019
//=================================================================================*/
//! \file

#include "stdafx.h"
#include "ImageFormats.h"

struct CIdTranslator
{
	int m_HH_Format;
	TCHAR *m_szExtension;
};

static CIdTranslator FormatTable[] =
{
	HH_FORMAT_TIFF_BIN,				_T(".tif"),
	HH_FORMAT_TIFF_BIN_COMP,		_T(".tif"),
	HH_FORMAT_TIFF_GRAY,			_T(".tif"),
	HH_FORMAT_JPEG,					_T(".jpg"),
	HH_FORMAT_BMP,					_T(".bmp"),
	HH_FORMAT_RAW,					_T(".raw"),
	HH_FORMAT_TIFF_COLOR_COMP,		_T(".tif"),
	HH_FORMAT_TIFF_COLOR_UNCOMP,	_T(".tif"),
	HH_FORMAT_JPEG_COLOR,			_T(".jpg"),
	HH_FORMAT_BMP_COLOR,			_T(".bmp"),
	-1, _T(""),	// EOT
};

TCHAR *GmImageHH::GetExtensionFromHsmFormat(int HH_Format)
{
	CIdTranslator *pTable = FormatTable;
	while(pTable->m_HH_Format >= 0)
	{
		if(pTable->m_HH_Format == HH_Format)
			break;
		pTable++;
	}
	return pTable->m_szExtension;
}

bool GmImageHH::isValidFormat(int HH_Format)
{
	return ((HH_Format >= HH_FORMAT_MIN) && (HH_Format<= HH_FORMAT_MAX));
}
