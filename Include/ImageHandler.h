// ============================================================================
// $Id: $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Brief.
	Details.
	\file */
//=============================================================================

#pragma once

namespace HsmDeviceComm
{
	class CImageHandler
	{
	public:
		CImageHandler();
		~CImageHandler();

		// Assignment operator
		const CImageHandler& operator= (CImageHandler &rhs);

		bool ReadImage(CDeviceComm *pDevice);
		HBITMAP GetBitmap();
		bool GetImageDetails(int &Format, size_t &Length, int &x, int &y);
		bool isValid();
		const TCHAR *GetExtension();
		bool Write(CString Filename);
		bool write(CString Filename) { return Write(Filename); }

	protected:
		Magick::Image *m_pImage;
		HBITMAP m_hBitmap;
		int m_HsmFormat;
		size_t m_Length;
	private:
		static bool s_NeedsInit;
	};

} // namespace HsmDeviceComm
