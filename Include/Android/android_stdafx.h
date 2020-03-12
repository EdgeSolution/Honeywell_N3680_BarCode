// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

///////////////////////////////////////////////////////////////////////////////////
#pragma once

///////////////////////////////////////////////////////////////////////////////////
// Linux

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stddef.h>
#include <cstddef>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <iostream>
#include <cstdlib>
#include <limits.h>
#include <string.h>

#include "GetTickCount.h"

using namespace std;

#define SS_MBCS
#define _T(x)	x

#define UCHAR unsigned char
#define USHORT unsigned short
#define UINT unsigned int
#define DWORD unsigned int
#define ULONG unsigned long
#define BOOL bool
#define sscanf_s sscanf

#define TRUE true
#define FALSE false
#define ASSERT assert

#define HBITMAP unsigned int

typedef char TCHAR;

// the stuff below is required to make CStdString compile in Linux
typedef const char*		PCSTR;
typedef char*				PSTR;
typedef const wchar_t*	PCWSTR;
typedef wchar_t*			PWSTR;

inline int	ssnprintf(PSTR pA, size_t nCount, PCSTR pFmtA, va_list vl)
{
	return vsnprintf(pA, nCount, pFmtA, vl);
}
inline int	ssnprintf(PWSTR pW, size_t nCount, PCWSTR pFmtW, va_list vl)
{
	return vswprintf(pW, nCount, pFmtW, vl);
}

#define UNUSED(expr) do { (void)(expr); } while (0)

inline int Framework_Init() { return 0; }
#define LIB_API extern "C" __attribute__ ((visibility("default")))

static const char* kTAG = "honeywell-hedc";
#include "logs.h"
#include "syncobjects.h"

#define HWND int*
#define WPARAM UINT
#define LPARAM UINT

#include "StdString.h"
#define CStringA CStdStringA
#define CStringW CStdStringW
#define CString CStdString
#define HAS_CSTRING 1

#define AppendChar push_back
#define CBR_115200 115200


