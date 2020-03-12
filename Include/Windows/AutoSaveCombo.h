/*=================================================================================
  AutoSaveCombo.h
//=================================================================================
   $Id: AutoSaveCombo.h 148 2015-04-29 10:25:21Z Fauth, Dieter $

//=================================================================================*/
//! \file
//

/////////////////////////////////////////////////////////////////////////////
// CAutoSaveCombo window

///////////////////////////////////////////////////////////////////////////////
/// A ComboBox that saves and restores its contents to the registry.
/*!
	
 \par requirements
 MFC\n
*/
class CAutoSaveCombo : public CComboBox
{
// Construction
public:
	CAutoSaveCombo();

// Attributes
public:

// Operations
public:
	void 	GetIni(const TCHAR *szSec, int &Current);
	void 	WriteIni(const TCHAR *szSec);

	int AddString(CString sValue);
	CString GetCurrentString(void);
	bool GetCurrent(int &Value);

// Implementation
public:
	virtual ~CAutoSaveCombo();

// Generated message map functions
protected:

	DECLARE_MESSAGE_MAP()
};

void AFXAPI DDX_CBInt(CDataExchange* pDX, int nIDC, int& value);

/////////////////////////////////////////////////////////////////////////////
