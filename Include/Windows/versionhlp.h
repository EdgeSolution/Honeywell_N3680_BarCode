// version.h : interface of the version helpers
//

#ifndef _VERSION_HELPERS_H_1CD34367_0B9F_11D4_BC0D_006008CCD137__INCLUDED_
#define _VERSION_HELPERS_H_1CD34367_0B9F_11D4_BC0D_006008CCD137__INCLUDED_

CString GetVersionResourceString(const TCHAR *key);

CString GetVersionString(void);
CString GetProductNameString(void);
CString GetManufacturerString(void);
CString GetCopyrightString(void);
CString GetInternalNameString(void);

CString GetProductVersionString(void);


#ifdef _DEBUG
CString GetVersionDebugString(void);
CString GetProductNameDebugString(void);
#else
#define GetVersionDebugString				GetVersionString
#define GetProductNameDebugString		GetProductNameString
#endif

#endif

