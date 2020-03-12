/*=================================================================================
  ValidateFirmware.cpp
//=================================================================================
   $Id: ValidateFirmware.cpp 202 2019-01-31 12:13:16Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// ValidateFirmware.cpp : implementation file
//

#include "stdafx.h"
#include "ValidateFirmware.h"
#include "memmem.h"
#include "HsmErrorDefs.h"

namespace HsmDeviceComm {

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CValidateFirmware::CValidateFirmware(void)
: m_pFileBuffer(NULL)
, m_pBuffer(NULL)
, m_FirmwareSize(0)
, m_NumMaskFlags(0)
, m_pMaskFlags(NULL)
{
};

CValidateFirmware::~CValidateFirmware(void)
{
	CleanBuffers();
};

///////////////////////////////////////////////////////////////////////////////
//! Cleanup our buffers.
/*!
*/
void CValidateFirmware::CleanBuffers()
{
	delete m_pFileBuffer;
	m_pFileBuffer = NULL;	// sanity
	m_pBuffer = NULL;		// sanity
	m_FirmwareSize = 0;
}

///////////////////////////////////////////////////////////////////////////////
//! Create a fresh buffer for the firmware file.
/*!
 @param [in] Size	Size of required new buffer
 @return long
*/
long CValidateFirmware::CreateFreshBuffer(size_t Size)
{
	CleanBuffers();
	m_pFileBuffer = new char[Size+16];	// make the buffer a little bigger than required.
	m_pBuffer = m_pFileBuffer;
	ASSERT(m_pBuffer!=NULL);
	return (m_pBuffer!=NULL) ? ERROR_SUCCESS : ERROR_NOT_ENOUGH_MEMORY;
}

#ifdef _MFC_VER
// MFC version
///////////////////////////////////////////////////////////////////////////////
//! Load firmware file and parse it.
/*!
 @param [in] Filename
 @return ERROR_SUCCES on success
*/
long CValidateFirmware::LoadFile(CString Filename)
{
	long RetVal=ERROR_FILE_NOT_FOUND;
	CFile theFile;

	CFileStatus Status;
	if( CFile::GetStatus( Filename, Status ) )
	{
		// Open the file
		CFileException FileException;

		if (!theFile.Open( Filename, CFile::modeRead, &FileException ))
		{
			TRACE(_T("Can't open file %s, error = %u\n"), (const TCHAR*)Filename, FileException.m_cause );
			RetVal=FileException.m_lOsError;
		}
		else
		{
			size_t FileSize = (size_t)Status.m_size;
			RetVal = CreateFreshBuffer(FileSize);
			if(RetVal==ERROR_SUCCESS)
			{
				m_FirmwareSize = theFile.Read( m_pFileBuffer, (UINT)FileSize );
				theFile.Close();
				if(FileSize != m_FirmwareSize)
				{
					RetVal=ERROR_READ_FAULT;
				}
				else
				{
					RetVal = ParseLoadedFile();
				}
			}
		}
	}
	return RetVal;
}
#else // none MFC version
///////////////////////////////////////////////////////////////////////////////
//! Load firmware file and parse it.
/*!
 @param [in] Filename
 @return ERROR_SUCCES on success
*/
long CValidateFirmware::LoadFile(const CString Filename)
{
	long RetVal=ERROR_FILE_NOT_FOUND;
	FILE *file = fopen(Filename, _T("rb"));
	if( file != NULL )
	{
		// find the size of the file
		if (0 == fseek(file, 0, SEEK_END))
		{
			size_t FileSize = ftell(file);
			if (0 == fseek(file, 0, SEEK_SET))	// seek back to the beginning
			{
				RetVal = CreateFreshBuffer(FileSize);
				if(RetVal==ERROR_SUCCESS)
				{
					m_FirmwareSize = fread( m_pFileBuffer, sizeof(char), (size_t)FileSize, file );
					fclose(file);

					if(FileSize != m_FirmwareSize)
					{
						RetVal=ERROR_READ_FAULT;
					}
					else
					{
						RetVal = ParseLoadedFile();
					}
				}
			}
		}
	}
	else
	{
		//		RetVal = errno;
	}
	return RetVal;
}
#endif

///////////////////////////////////////////////////////////////////////////////
//! Use this to load a firmware file that is already in memory.
/*!
 @param [in] pBuffer	firmware in buffer
 @param [in] Size		size of firmware
 @return ERROR_SUCCES on success
*/
long CValidateFirmware::LoadBuffer(const UCHAR *pBuffer, size_t Size)
{
	ASSERT(pBuffer!=NULL);
	ASSERT(Size>0);
	if((pBuffer==NULL)||(Size==0))
		return ERROR_INVALID_PARAMETER;

	CleanBuffers();
	m_pBuffer = (const char*)pBuffer;
	m_FirmwareSize = Size;
	return ParseLoadedFile();
}

///////////////////////////////////////////////////////////////////////////////
//! Parse the firmware file in buffer.
/*!
 @return ERROR_SUCCES on success
*/
long CValidateFirmware::ParseLoadedFile(void)
{
	long RetVal=ParseLoadedFileGen6();
	if(RetVal!=ERROR_SUCCESS)
	{
		RetVal=ParseLoadedFileGen5();
	}
	return RetVal;
}


const char *pSearchPattern = "MATRIXAPP\000\000\000DEVICETYPE\000\000";
const size_t PatternSize = 24;

///////////////////////////////////////////////////////////////////////////////
//! Parse the GEN5 firmware file in buffer.
/*!
 @return ERROR_SUCCES on success
*/
long CValidateFirmware::ParseLoadedFileGen5(void)
{
	long RetVal=ERROR_NO_FW_FILE;
	const char *pHTag = memmem (m_pBuffer, pSearchPattern, m_FirmwareSize, PatternSize);
	if(pHTag != NULL)
	{
		RetVal=ERROR_PARSE_FILE;
		if(m_FirmwareSize > (size_t)((pHTag - m_pBuffer) + 4))
		{
			const UCHAR *pRead = (const UCHAR*)pHTag + PatternSize;
			m_NumMaskFlags  = (*pRead++ & 0xff);
			m_NumMaskFlags |= (*pRead++ & 0xff) << 8;
			m_NumMaskFlags |= (*pRead++ & 0xff) << 16;
			m_NumMaskFlags |= (*pRead++ & 0xff) << 24;
			if(m_FirmwareSize > ((pRead - (const UCHAR*)m_pBuffer)+ m_NumMaskFlags))
			{
				m_pMaskFlags = pRead;
				RetVal = ERROR_SUCCESS;
			}
		}

	}

	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! Parse the GEN5 firmware file in buffer.
/*! This is a very simple check right now.
Gen6 firmware has MOCF as the very first 4 bytes in the file.
We only check for these first 4 bytes.
The device itself does a full test anyway.
 @return ERROR_SUCCES on success
*/
long CValidateFirmware::ParseLoadedFileGen6(void)
{
	long RetVal=ERROR_NO_FW_FILE;
	const char *pSearchPattern = "MOCF";
	const size_t PatternSize = 4;
	const char *pFound = memmem (m_pBuffer, pSearchPattern, PatternSize, PatternSize);
	if(pFound == m_pBuffer)
	{
		RetVal = ERROR_SUCCESS;
	}

	return RetVal;
}

}	// namespace HsmDeviceComm
