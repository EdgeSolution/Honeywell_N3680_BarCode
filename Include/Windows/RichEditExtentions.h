/*=============================================================================
 *
 *=============================================================================
 * $Id: RichEditExtentions.h 147 2015-04-29 09:52:52Z Fauth, Dieter $
 * $Revision: 147 $
 * $Date: 2015-04-29 11:52:52 +0200 (Mi, 29 Apr 2015) $
 * $Author: Fauth, Dieter $
 *============================================================================*/
//! \file
///////////////////////////////////////////////////////////////////////////////
// Public domain 1999-2006 by Dieter Fauth.
// Do whatever you like except to claim ownership of this code.
///////////////////////////////////////////////////////////////////////////////

// RichEditExtentions.h : header file
// This files contains a few nice helper functions to make the handling of text a little easier.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CRichEditCtrlExtended dialog
/** Extends the CRichEditCtrl with some features that makes it easier to use.
 */
class CRichEditCtrlExtended : public CRichEditCtrl
{
public:
	CRichEditCtrlExtended(void);
	void StreamIn(CStringW sText, int Format=SF_TEXT|SF_UNICODE|SFF_SELECTION);
	void StreamIn(CStringA sText,int Format=SF_TEXT|SFF_SELECTION);

	// black text
	void Display(CStringA sText, BOOL bold = FALSE);
	void Display(CStringW sText, BOOL bold = FALSE);

	// Simple colored text
	void DisplayWithColor(CStringA sText, COLORREF color, BOOL bold = FALSE);
	void DisplayWithColor(CStringW sText, COLORREF color, BOOL bold = FALSE);
	void NewLine(void);

	// Shows control characters in HEX notation
	void TranslateTextWithColor(const CStringA  sText, COLORREF color, COLORREF hexcolor, BOOL bold = FALSE, BOOL AutoNewLine=FALSE);
	void TranslateTextWithColor(const UCHAR   *pBuf, size_t size, COLORREF color, COLORREF hexcolor, BOOL bold = FALSE, BOOL AutoNewLine=FALSE);
	void TranslateTextWithColor(const char    *pBuf, size_t size, COLORREF color, COLORREF hexcolor, BOOL bold = FALSE, BOOL AutoNewLine=FALSE)
	{
		TranslateTextWithColor((const UCHAR*)pBuf, size, color, hexcolor, bold, AutoNewLine);
	}

	void TranslateTextWithColor(const CStringW sText, COLORREF color, COLORREF hexcolor, BOOL bold = FALSE, BOOL AutoNewLine=FALSE);
	void TranslateTextWithColor(const wchar_t *pBuf, size_t size, COLORREF color, COLORREF hexcolor, BOOL bold = FALSE, BOOL AutoNewLine=FALSE);

	// Shows all in HEX notation
	void TranslateToHex(const CStringA sText, COLORREF hexcolor, BOOL bold);
	void TranslateToHex(const CStringW sText, COLORREF hexcolor, BOOL bold);
	void TranslateToHex(const wchar_t *pBuf, size_t size, COLORREF hexcolor, BOOL bold);
	void TranslateToHex(const UCHAR   *pBuf, size_t size, COLORREF hexcolor, BOOL bold);
	void TranslateToHex(const char    *pBuf, size_t size, COLORREF hexcolor, BOOL bold)
	{
		TranslateToHex((const UCHAR*)pBuf, size, hexcolor, bold);
	}

	void ClearScreen(void);

// Implementation
protected:
	static DWORD CALLBACK StaticStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
	DWORD StreamInCallbackW(LPBYTE pBuff, long NumBytesRequested, long *pSentBytes);
	DWORD StreamInCallbackA(LPBYTE pBuff, long NumBytesRequested, long *pSentBytes);
	CStringW sStreamInW;
	CStringA sStreamInA;
	LONG RestSizeIn;
};

