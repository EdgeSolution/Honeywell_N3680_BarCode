/*=================================================================================
  BitmapCtrl.h
//=================================================================================
   $Id: BitmapCtrl.h 232 2017-03-05 13:15:55Z Fauth, Dieter $

//=================================================================================*/
//! \file
// BitmapCtrl.h : header file
//
// found at http://www.codeguru.com/Cpp/G-M/bitmap/viewers/article.php/c1763/
// Now heavily changed, not much of the original code is there anymore.

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CBitmapCtrl window

class CBitmapCtrl : public CStatic
{
public:
	void SetBitmap(CBitmap &bmp);
	void SetBitmap(HBITMAP hBmp);
	void DrawBitmap();


// Overrides
	public:
#ifndef _WIN32_WCE
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd);
	virtual LRESULT WindowProc(UINT mess, WPARAM wParam, LPARAM lParam);
	void LoadFile(CString filename);
#endif

// Implementation
public:
	CBitmapCtrl();
	virtual ~CBitmapCtrl();
	afx_msg void OnPaint();

protected:
	BOOL RegisterWindowClass();
	virtual void PreSubclassWindow();
#ifdef _WIN32_WCE
	CDC m_ExtraMemDC;
	CBitmap* m_pBitmap2;
	CBitmap* m_pOldBitmap2;
#endif
	void Resize();
	void Recalc();

	// Generated message map functions
protected:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

	CRect m_rcClient;
	CSize m_scrollbar_size;
	CSize m_image_size;
	CSize m_offset;
	bool m_bResizing;
	HBITMAP m_hBitmap;
	CDC m_memDC;
	CBitmap* m_pBitmap;
	CBitmap* m_pOldBitmap;
};

/////////////////////////////////////////////////////////////////////////////
