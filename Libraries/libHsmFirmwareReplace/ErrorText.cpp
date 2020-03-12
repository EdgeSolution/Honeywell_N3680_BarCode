
#include "stdafx.h"
#include "HsmErrorDefs.h"

namespace HsmDeviceComm {

#undef _T
#define _T(x)	x

LIB_API bool HsmGetErrorText(unsigned int error, const char **pText)
{
	bool RetVal=true;
	switch(error)
	{
		case ERROR_SUCCESS:
			*pText = _T("Success");
			break;
		case ERROR_COMMUNICATION:
		case HEDC_CONNECTION_LOST:
			*pText = _T("There is a communication problem with the device");
			break;
		case ERROR_PORT_NOT_OPEN:
		case HEDC_NO_DEVICE_FOUND:
		case HEDC_PORT_NOT_AVAILABLE:
			*pText = _T("Port/device open failed (is it in use by someone else?)");
			break;
		case ERROR_CANCELED_BY_REMOTE:
			*pText = _T("The device aborted the receive of the firmware");
			break;
		case ERROR_NO_SYNC:
			*pText = _T("Device did not respond with XModem requests");
			break;
		case ERROR_XMIT_ERROR:
			*pText = _T("A transmit error occurred");
			break;
		case ERROR_NO_FW_FILE:
		case ERROR_PARSE_FILE:
			*pText = _T("This is not a valid firmware file");
			break;
		case ERROR_FILE_TO_BIG:
			*pText = _T("This firmware is too big for this device");
			break;
		case ERROR_FLASH_FAILED:
			*pText = _T("Storing the firmware into the flash rom failed");
			break;
		case ERROR_FLASH_UNSURE:
			*pText = _T("Flashing seemed to work, but device does not respond");
			break;
		case ERROR_FLASH_NO_RESPOND:
			*pText = _T("We got no ACK after the storing, but could communicate with the device");
			break;
		case ERROR_FILE_NOT_FOUND:
			*pText = _T("File not found");
			break;
		case ERROR_READ_FAULT:
			*pText = _T("File could not be read");
			break;
		case ERROR_NOT_ENOUGH_MEMORY:
			*pText = _T("Not enough memory");
			break;
		case ERROR_INVALID_PARAMETER:
			*pText = _T("Parameter is not valid");
			break;
		case ERROR_NOT_SUPPORTED:
		case HEDC_INVALID_COMMAND:
			*pText = _T("Function not implemented");
			break;
		case ERROR_AUTOSELECT_FAILED:
			*pText = _T("Propably more than one device is connected, so autoselect needs help.");
			break;
		case ERROR_CMD_UNKNOWN:
			*pText = _T("Command not implemented in device");
			break;
		case ERROR_CMD_FAILED:
		case HEDC_NOT_SUPPORTED:
			*pText = _T("Command returned error (parameter bad?)");
			break;
		case ERROR_FRAMEWORK_FAILED:
		case HEDC_DLL_NOT_INITIALIZED:
			*pText = _T("Fatal Error: Framework (MFC/QT) initialization failed");
			break;
		case ERROR_GETMODUL_FAILED:
			*pText = _T("Fatal Error: GetModuleHandle failed");
			break;

		case ERROR_INVALID_INPUT_BUFFER:
		case HEDC_INVALID_INPUT_BUFFER:
			*pText = _T("Invalid Input buffer");
			break;
		case ERROR_OUTPUT_BUFFER_TOO_SMALL:
		case HEDC_OUTPUT_BUFFER_TOO_SMALL:
			*pText = _T("Invalid Output buffer");
			break;
		case ERROR_CANCELED_BY_USER:
		case HEDC_OPERATION_CANCELLED:
			*pText = _T("Canceled");
			break;
		case ERROR_REGISTRY:
		case HEDC_REGISTRY_ERROR:
			*pText = _T("Fatal Error: Registry error");
			break;
		case ERROR_INTERNAL:
		case HEDC_INTERNAL_ERROR:
			*pText = _T("Fatal Error: LIB internal error");
			break;

		default:
			*pText = _T("Unknown error");
			RetVal=false;
			break;
	}
	return RetVal;
}

}	// namespace HsmDeviceComm

