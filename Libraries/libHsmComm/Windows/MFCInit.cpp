// MFCInit.cpp : Defines theinit call for the MFC application.
//

#include "stdafx.h"
#include "HsmErrorDefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object
// we cannot use this here, it breaks GUI tools.
// I think we do not need it anyway in CMD line tools.
//CWinApp theApp;

static HMODULE hModule = nullptr;

int Framework_Init()
{
	int RetVal = ERROR_SUCCESS;
	if (hModule == nullptr)	// avoid calling it twice
	{
		hModule = ::GetModuleHandle(nullptr);

		if (hModule != nullptr)
		{
			// initialize MFC
			if (!AfxWinInit(hModule, nullptr, _T(""), 0))
			{
				RetVal = ERROR_FRAMEWORK_FAILED;
			}
		}
		else
		{
			RetVal = ERROR_GETMODUL_FAILED;
		}
	}
	return RetVal;
}



