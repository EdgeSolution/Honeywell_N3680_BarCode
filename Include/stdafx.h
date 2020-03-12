// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

///////////////////////////////////////////////////////////////////////////////////
#pragma once

#if defined WIN32
#include "win_stdafx.h"
#elif defined LINUX
#include "linux_stdafx.h"
#elif defined ANDROID
#include "android_stdafx.h"
#elif
#error "must define one of WIN32,LINUX,ANDROID"
#endif

#include <string>


#define _CRT_SECURE_NO_WARNINGS 1

using namespace std;
#undef UNUSED
#define UNUSED(expr) do { (void)(expr); } while (0)

int Framework_Init();

#include "wchar_overloads.h"

// Safe typing by declaring some classes here
namespace HsmDeviceComm {
	class CTxFirmware *CreateFlasher();
	class CDeviceComm;
	class CThreadedDeviceCommands;
	class CImageHandler;
}
namespace Magick
{
	class Image;
}

#define CBR_115200 115200

// dfUsbLib shall be a static lib
#define STATIC_PORTMAN 1

