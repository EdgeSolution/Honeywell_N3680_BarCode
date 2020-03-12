/*=================================================================================
  ErrorDefsFW.h
//=================================================================================
   $Id: HsmErrorDefsFW.h 158 2018-12-06 17:58:56Z e411776 $

Example source code provided as is. No warranties of any kind.
Copyright (C) Hand Held Products, Inc. 2006
Copyright Honeywell International Inc. 2015
//=================================================================================*/
//! \file
// ErrorDefsFW.h : header file
//

#pragma once

//! Defines the error return values for most functions.
/*! The numbers fit into the model used by Win32.
 	If you port to another enviroment, you might need to adjust the values.
 \verbatim
// Win32 errors:
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
 \endverbatim
*/

enum Results_t	// see above for Win32 errors explanation
{
#ifndef ERROR_SUCCESS
	ERROR_SUCCESS = 0,	                  		// declared in winerror.h
#endif

	rCustomCode	= 0x20000000,
	rInformational	= 0x60000000,
	rError			= 0xE0000000,
	rWarning		= 0xA0000000,

// results for information

	ERROR_CANCELED_BY_USER	= rError+1,		//!< User aborted the action
	ERROR_NO_FW_FILE		= rError+2,		//!< This is no firmware file
	ERROR_PARSE_FILE		= rError+3,		//!< Parsing the firmware file did fail
	ERROR_FILE_TO_BIG		= rError+4,		//!< Firmware file is too big for this device
	ERROR_WRONG_FW			= rError+5,		//!< Firmware is for a different device
#ifndef ERROR_FILE_NOT_FOUND
	ERROR_FILE_NOT_FOUND	= rError+6,		// declared in winerror.h
#endif
#ifndef ERROR_NOT_ENOUGH_MEMORY
	ERROR_NOT_ENOUGH_MEMORY	= rError+7,		// declared in winerror.h
#endif
#ifndef ERROR_READ_FAULT
	ERROR_READ_FAULT		= rError+8,		// declared in winerror.h
#endif
#ifndef ERROR_INVALID_PARAMETER
	ERROR_INVALID_PARAMETER	= rError+9,		// declared in winerror.h
#endif
#ifndef ERROR_NOT_SUPPORTED
	ERROR_NOT_SUPPORTED = rError + 10,		// declared in winerror.h
#endif

	ERROR_INVALID_INPUT_BUFFER	 = rError+81,	//!< Input buffer NULL?
	ERROR_OUTPUT_BUFFER_TOO_SMALL= rError+82,	//!< Output buffer NULL or too small.

	ERROR_FRAMEWORK_FAILED	= rError+90,	//!< MFC/QT etc could not be initialized
	ERROR_GETMODUL_FAILED	= rError+91,	//!< MFC/QT etc could not be initialized
	ERROR_GETMODUL_FAILED = rError + 91,	//!< MFC/QT etc could not be initialized

	ERROR_THREAD			= rError+100,	//!< Thread error
	ERROR_THREAD_RUNNING	= rError+101,	//!< Thread did already run as we tried to create it
	ERROR_THREAD_NOT_CREATED= rError+102,	//!< Thread could not be created

	ERROR_COMMUNICATION		= rError+200,	//!< A communication error occurred
	ERROR_PORT_NOT_OPEN		= rError+201,	//!< Port/device open failed (is it in use by someone else?)
	ERROR_CANCELED_BY_REMOTE= rError+202,	//!< Remote side did cancel the XModem transfer
	ERROR_NO_SYNC			= rError+203,	//!< We did not receive the XModem request to send signal
	ERROR_XMIT_ERROR		= rError+204,	//!< Transmitting failed
	ERROR_DEVICETYPE		= rError+205,	//!< Device reports an invalid device type
	ERROR_AUTOSELECT_FAILED	= rError+206,	//!< Proably more than one device is connected, so autoselect needs help.
	ERROR_CMD_UNKNOWN		= rError+207,	//!< Command not implemented
	ERROR_CMD_FAILED		= rError+208,	//!< Command returned error

	ERROR_BOOTMODE_FAILED	= rError+220,	//!< No Boot mode communication found

	ERROR_FLASH_FAILED		= rError+300,	//!< Storing the firmware into the flash ROM failed
	ERROR_FLASH_UNSURE		= rError+301,	//!< We got no ACK after the storing, but could communicate with the device
	ERROR_FLASH_NO_RESPOND	= rError+302,	//!< Flashing seamed to work, but device does not respond
	ERROR_FLASH_WRONG_FW	= rError+304,	//!< Device complained about the firmware

	MAX_ERR
};

// Map legacy error definitions
// Note: The old error numbers did cause conflicts with existing numbers in various OS.
#define HEDC_NO_ERROR                     ERROR_SUCCESS
#define HEDC_INVALID_INPUT_BUFFER         ERROR_INVALID_INPUT_BUFFER
#define HEDC_OUTPUT_BUFFER_TOO_SMALL      ERROR_OUTPUT_BUFFER_TOO_SMALL
#define HEDC_REGISTRY_ERROR               3
#define HEDC_DLL_NOT_INITIALIZED          ERROR_FRAMEWORK_FAILED
#define HEDC_NO_DEVICE_FOUND              ERROR_COMMUNICATION
#define HEDC_PORT_NOT_AVAILABLE           ERROR_PORT_NOT_OPEN
#define HEDC_CONNECTION_LOST              ERROR_COMMUNICATION
#define HEDC_OPERATION_CANCELLED          ERROR_CANCELED_BY_USER
#define HEDC_INTERNAL_ERROR               12
#define HEDC_NOT_SUPPORTED                ERROR_NOT_SUPPORTED
#define HEDC_INVALID_COMMAND			  ERROR_CMD_UNKNOWN
