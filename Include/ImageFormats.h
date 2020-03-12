/*=================================================================================
  CxImage.h
//=================================================================================
	$Id: CxImage.h 148 2015-04-29 10:25:21Z Fauth, Dieter $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2005
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file

// This file help to include all required files for CxImage
// It does not belong to the original distribution of CxImage

#pragma once

// File formats for the IMGSHP command and it's friends
const int HH_FORMAT_INVALID = -1;
const int HH_FORMAT_TIFF_BIN = 1;
const int HH_FORMAT_TIFF_BIN_COMP = 2;
const int HH_FORMAT_TIFF_GRAY = 3;
const int HH_FORMAT_RAW = 5;
const int HH_FORMAT_JPEG = 6;
const int HH_FORMAT_BMP = 8;
const int HH_FORMAT_TIFF_COLOR_COMP = 10;
const int HH_FORMAT_TIFF_COLOR_UNCOMP = 11;
const int HH_FORMAT_JPEG_COLOR = 12;
const int HH_FORMAT_BMP_COLOR = 14;
const int HH_FORMAT_BMP_COLOR_UNCOMP_RAW = 15;


const int HH_FORMAT_JPEG_GRAY = HH_FORMAT_JPEG;
const int HH_FORMAT_BMP_GRAY = HH_FORMAT_BMP;

const int HH_FORMAT_MIN = HH_FORMAT_TIFF_BIN;
const int HH_FORMAT_MAX = HH_FORMAT_BMP_COLOR_UNCOMP_RAW;

// Adapters to connect GraphicsMagick with Honeywell functions
class GmImageHH
{
public:
	static TCHAR *GetExtensionFromHsmFormat(int HH_Format);
	static bool isValidFormat(int HH_Format);
};
