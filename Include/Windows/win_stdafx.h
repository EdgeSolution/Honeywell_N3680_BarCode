
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

// MFC support for MBCS deprecated in Visual Studio 2013
#define NO_WARN_MBCS_MFC_DEPRECATION 1

// MSVC 2005 adds some security features, 
// this setting prevents us from changing it all over the place.
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT  1 
#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES   0 
#define _CRT_SECURE_NO_WARNINGS 1

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
#include <afx.h>

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <string>

#if _MSC_VER >= 1400
#define URESULT LRESULT
#define _stscanf_sx _stscanf_s
#define _stscanf_s1 _stscanf_s
#define sscanf_sx sscanf_s
#else
#define URESULT UINT
#define genericException generic
#define _tcscpy_s(a,b,c)	_tcscpy(a,c)
#define strcpy_s(a,b,c)		strcpy(a,c)
#define _tcsncpy_s(a,b,c,d) _tcsncpy(a,c,d)
#define _stscanf_sx _stscanf
#define _stscanf_s1(a,b,c,size) _stscanf(a,b,c)
#define sscanf_sx sscanf

inline void _tsplitpath_s(const TCHAR *path, TCHAR *drive, TCHAR *dir, TCHAR *fname, TCHAR *ext )
{
	 _tsplitpath(path, drive,dir,fname,ext);
};

typedef int errno_t;
inline errno_t _tfopen_s(FILE **stream, const TCHAR* name, const TCHAR* mode)
{
	*stream = _tfopen(name, mode);
	return (stream == NULL);
};

#endif // _MSC_VER >= 1400

#define LIB_API
