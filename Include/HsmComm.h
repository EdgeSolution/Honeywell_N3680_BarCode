/*=================================================================================
  HsmComm.h
//=================================================================================
Copyright (C) Hand Held Products, Inc. 2005
//=================================================================================*/
//! \file
/*! This is the include file for the communication library.
	Include it at least once into a file of your project to trigger the automatic linking.
*/
#pragma once

#ifndef HSM_NO_AUTOLINK
//! Automatically add the libraries to the link list
#pragma comment(lib,"libHsmComm.lib")
#endif

#include "DeviceComm.h"
using namespace HsmDeviceComm;

