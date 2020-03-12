/*=================================================================================
  ddxfile.h
//=================================================================================
   $Id: ddxfile.h 238 2017-03-07 18:19:45Z Fauth, Dieter $

//=================================================================================*/
//! \file
// FirmwareReplaceDlg.h : header file
//

/*
	Modified by Dieter Fauth to use a CComboBox rather an CEdit
*/

// original source:
/*
Module : DDXFILE.H
Purpose: Defines the interface to a for a MFC DDX_ routine to get a filename
         Using the file open/save as common dialogs
Created: PJN / DDXGETFILE/1 / 19-03-1997
History: None

Copyright (c) 1997 - 1998 by PJ Naughter.  
All rights reserved.

*/


////////////////////////////////// Macros ///////////////////////////

#ifndef _DDXFILE_H__
#define _DDXFILE_H__

#define GETFILENAME_EDIT_ID 100

////////////////////////////////// Consts /////////////////////////////////////

//flags used to control how the DDX_GetFilenameControl routine workds

const DWORD GF_OVERWRITEPROMPT  = 0x0001;   //User will be prompted about overwriting existing file 
                                            //prior to allowing selection
const DWORD GF_FILEMUSTEXIST    = 0x0002;   //File must exist to be selected
const DWORD GF_OLD_STYLE_DIALOG = 0x0004;   //Use the old style file open dialog instead of the
                                            //style as used in Windows Explorer





////////////////////// foward declaration ///////////////////////////
class CGetFilenameControl;


/////////////////////////// Classes /////////////////////////////////
class CModifyButton : public CButton
{
public:
  CModifyButton();
  void SetBuddy(CGetFilenameControl* pBuddy);

protected:
  //{{AFX_VIRTUAL(CModifyButton)
  public:
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

  //{{AFX_MSG(CModifyButton)
  afx_msg void OnClicked();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  CGetFilenameControl* m_pBuddy;
  BOOL m_bFirstCall;
  CToolTipCtrl m_ToolTip;
};


#if(0)
class CGetFileNameDialog : public CFileDialog
{
	DECLARE_DYNAMIC(CGetFileNameDialog)

public:
	CGetFileNameDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,	LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL);

protected:
  virtual void OnInitDone();
  virtual BOOL OnFileNameOK();

	//{{AFX_MSG(CGetFileNameDialog)
	//}}AFX_MSG
  virtual BOOL OnInitDialog();
  virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};
#endif

class CStringHistory;

//#define baseCGetFilenameControl CEdit
#define baseCGetFilenameControl CComboBox
class CGetFilenameControl : public baseCGetFilenameControl
{
public:
//Constructors / Destructors
  CGetFilenameControl();

//Methods
  BOOL SubclassEdit(HWND hEdit);

//Accessors/Mutators
  //Set/Get the flags
  void SetFlags(DWORD dwFlags) { m_dwFlags = dwFlags; };
  DWORD GetFlags() const { return m_dwFlags; };

  //Set/Get the dialog title
  void SetDialogTitle(const CString& sDialogTitle) { m_sDialogTitle = sDialogTitle; };
  CString GetDialogTitle() const { return m_sDialogTitle; };

  //Set/Get the extension filter string
  void SetExtensionFilter(const CString& sExtFilter) { m_sExtFilter = sExtFilter; };
  CString GetExtensionFilter() const { return m_sExtFilter; };

  void SetFileOpen(BOOL bOpenFileDialog) { m_bOpenFileDialog = bOpenFileDialog; };

  //Bring up the file picker dialog
  virtual void Edit(void);

	void Attach(CStringHistory *pHistory)	{ m_pNameHistory = pHistory;	};
	void AddToHistory(CString sFile);
protected:
  //{{AFX_VIRTUAL(CGetFilenameControl)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

  //{{AFX_MSG(CGetFilenameControl)
	afx_msg void OnDropDown();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  BOOL AddEditButton();

  CModifyButton m_Edit;
	BOOL			m_bOpenFileDialog;
  DWORD         m_dwFlags;
  CString       m_sDialogTitle;
  CString       m_sExtFilter;
	CStringHistory *m_pNameHistory;
};





/////////////// MFC Data exchange routines //////////////////////////

void DDX_FilenameControl(CDataExchange* pDX, int nIDC, CGetFilenameControl& rCGetFilenameControl, DWORD dwFlags, const CString& sDialogTitle, const CString& sExtFilter);
void DDX_FilenameControl(CDataExchange* pDX, int nIDC, CGetFilenameControl& rCGetFilenameControl, const CString& sExtFilter);
void DDX_FilenameValue(CDataExchange* pDX, int nIDC, CGetFilenameControl& rCGetFilenameControl, CString& sFile, BOOL bOpenFileDialog);
void DDV_FilenameControl(CDataExchange* pDX, CGetFilenameControl& rCGetFilenameControl);

void DDX_FilenameControl(CDataExchange* pDX, int nIDC, CGetFilenameControl& rCGetFilenameControl, CString& sFile, BOOL bOpenFileDialog, const CString& sExtFilter);

#endif //_DDXFILE_H__
