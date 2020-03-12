//! \file
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1998-2006 by Dieter Fauth. All rights reserved
///////////////////////////////////////////////////////////////////////////////
// Please see the licence.cpp for details about the licence to use this code.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Taken from dfUsbLib
// With friendly permission from the original author Dieter Fauth.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef WIN32
#define EV_RXCHAR           0x0001  // Any Character received
//#define EV_RXFLAG           0x0002  // Received certain character
#define EV_TXEMPTY          0x0004  // Transmitt Queue Empty
#define EV_CTS              0x0008  // CTS changed state
#define EV_DSR              0x0010  // DSR changed state
//#define EV_RLSD             0x0020  // RLSD changed state
#define EV_BREAK            0x0040  // BREAK received
#define EV_ERR              0x0080  // Line status error occurred
#define EV_RING             0x0100  // Ring signal detected
#endif

enum
{
	EV_STATUS		= 0x01000000,	//!< A change of the device status happened
	EV_PNP_CHANGE	= 0x02000000,	//!< A device arrived or was removed (not sent for our device*)
	EV_PNP_NEWDEV	= 0x04000000,	//!< A device arrived (not sent for our device*)
	EV_PNP_REMDEV	= 0x08000000,	//!< A device was removed (not sent for our device*)
	EV_PNP_ARRIVE	= 0x10000000,	//!< Our device arrived (a USB device has been plugged in)
	EV_PNP_REMOVE	= 0x20000000,	//!< Our device was removed (USB ...)
	EV_USER1		= 0x40000000,	//!< For derived classes
	EV_USER2		= 0x80000000,	//!< For derived classes
	EV_PORTMAN_DEFAULT = EV_PNP_ARRIVE|EV_PNP_REMOVE|EV_STATUS|EV_CTS|EV_DSR|EV_RING|EV_BREAK
};

/*
	* Please be aware that composite devices still send the EV_PNP_CHANGE,EV_PNP_NEWDEV,EV_PNP_REMDEV
	  for the interface that is not ours.
*/
