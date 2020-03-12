/*=================================================================================
  GraphicsMagick.h
//! \file

// This file help to include all required files for Graphicsmagick
// It does not belong to the original distribution of Graphicsmagick
*/
#pragma once

#ifndef HSM_NO_AUTOLINK
// automatically add the libraries to the link list
#pragma comment(lib,"CORE_bzlib.lib")
#pragma comment(lib,"CORE_coders.lib")
#pragma comment(lib,"CORE_filters.lib")
#pragma comment(lib,"CORE_jbig.lib")
#pragma comment(lib,"CORE_jp2.lib")
#pragma comment(lib,"CORE_jpeg.lib")
#pragma comment(lib,"CORE_lcms.lib")
#pragma comment(lib,"CORE_magick.lib")
#pragma comment(lib,"CORE_Magick++.lib")
#pragma comment(lib,"CORE_png.lib")
#pragma comment(lib,"CORE_tiff.lib")
#pragma comment(lib,"CORE_ttf.lib")
#pragma comment(lib,"CORE_wand.lib")
#pragma comment(lib,"CORE_wmf.lib")
#pragma comment(lib,"CORE_xlib.lib")
#pragma comment(lib,"CORE_zlib.lib")
#endif

#define MSWINDOWS 
#include "Magick++.h" // image lib

extern "C" void *ImageToHBITMAP(const MagickLib::Image* image);

