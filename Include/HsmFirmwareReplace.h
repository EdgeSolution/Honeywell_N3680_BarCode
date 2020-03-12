/*=================================================================================
  HsmFirmwareReplace.h
//=================================================================================
Copyright (C) Hand Held Products, Inc. 2005
//=================================================================================*/
//! \file
/*! This is the include file for the firmware replace portion of the communication library.
	Include it at least once into a file of your project to trigger the automatic linking.
*/
#pragma once

#ifndef HSM_NO_AUTOLINK
//! Automatically add the libraries to the link list
#pragma comment(lib,"libHsmFirmwareReplace.lib")
#endif

#include "HsmComm.h"
#include "ThreadedDeviceCommands.h"
#include "FirmwareReplace.h"
using namespace HsmDeviceComm;

