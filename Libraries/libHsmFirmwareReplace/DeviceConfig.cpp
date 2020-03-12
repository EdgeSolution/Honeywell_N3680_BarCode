#include "stdafx.h"
#include <fstream>
#include <sstream>
#include <string>

#include "HsmErrorDefs.h"
#include "HsmProgress.h"
#include "TransmitFirmware.h"
#include "DeviceConfig.h"

namespace HsmDeviceComm {

extern std::string sLastGoodConnection;

CDeviceConfig::CDeviceConfig(void)
: m_pComm(NULL)
, m_Percent(1)
{
}

CDeviceConfig::~CDeviceConfig(void)
{
}

bool CDeviceConfig::ExecuteMenuCommand(const std::string sCmd)
{
	ASSERT(m_pComm!=NULL);
	return m_pComm->ExecuteMenuCommand(sCmd);
}

bool CDeviceConfig::WaitForDeviceChange(int Delay)
{
	ASSERT(m_pComm != NULL);
	bool bNewdevice = false;
	const int steps = 100;
	for (int i = 0; i < Delay; i += steps)
	{
		CallbackProgress(EV_Configering, m_Percent++);
		Sleep(steps);
#ifdef WIN32
		if (m_pComm->CheckForNewDevice())
		{
			// Looks like the COMx name hase been changed. Happens if the new FW has a different VID/PID.
			CallbackProgress(EV_DeviceChangedInterface, 0);
			bNewdevice = true;
			break;
		}
#endif
	}
#ifdef WIN32
	if (!bNewdevice)
	{
		const bool bForce = true;
		if (m_pComm->CheckForNewDevice(bForce))
		{
			// Looks like the COMx name hase been changed. Happens if the new FW has a different VID/PID.
			CallbackProgress(EV_DeviceChangedInterface, 0);
		}
	}
#endif
	return true;
}

bool CDeviceConfig::CallbackProgress(phases_t Phase, int Percent)
{
	ASSERT(m_pComm != NULL);
	return m_pComm->CallbackProgress(Phase, Percent);
}

const char cComment = '#';
const char *szComments = "#";
const char *szWhitespaces (" \t\f\v\n\r");

int CDeviceConfig::ParseFile(const std::string Filename)
{
	int RetVal=ERROR_SUCCESS;
	std::string Line;
	std::ifstream infile(Filename.c_str(), std::ios::in);
	if (infile.is_open())
	{
		while (!infile.eof())
		{
			getline(infile, Line);
			CallbackProgress(EV_Configering, m_Percent++);
			RetVal = ParseLine(Line);
			if(RetVal!=ERROR_SUCCESS)
				break;
		}
		infile.close();
	}
	else
	{
		RetVal = ERROR_FILE_NOT_FOUND;
	}
	return RetVal;
}

int CDeviceConfig::ParseLine(const std::string Line)
{
	int RetVal=ERROR_SUCCESS;
	if(Line.length() > 0)
	{
		char cmd = Line[0];
		switch(cmd)
		{
		case cComment:
			break;
		case 'm':
		case 'M':
		case ' ':
		case '\t':
			RetVal = ParseMenuCommand(Line);
			break;
		case 'w':
		case 'W':
			RetVal = ParseWait(Line);
			break;
		default:
			break;
		}
	}
	return RetVal;
}

std::string CDeviceConfig::GetPayload(const std::string Line)
{
	if(Line.length() <= 1)
		return "";

	size_t comment = Line.find_first_of (szComments);
	if((comment!=std::string::npos)&&(comment>0))
		comment--;

	// skip cmd selector
	const size_t skip=1;
	size_t first = Line.find_first_not_of(szWhitespaces, skip);

	if(first >= comment)
		return "";

	size_t trail = Line.find_last_not_of(szWhitespaces, comment);
  
	return Line.substr(first, trail);
}

int CDeviceConfig::ParseMenuCommand(const std::string Line)
{
	int RetVal=ERROR_SUCCESS;
	std::string MenuCmd = GetPayload(Line);
	size_t last = MenuCmd.length();

	if(MenuCmd.length() == 0)
		return ERROR_INVALID_PARAMETER;

	// automatically add the terminator
	char cTerminator = Line[last];
	if(!((cTerminator == '!') || (cTerminator == '.')))
		MenuCmd.append("!");
	
	if(!ExecuteMenuCommand(MenuCmd))
		RetVal = ERROR_CMD_FAILED;

	return RetVal;
}

int CDeviceConfig::ParseWait(const std::string Line)
{
	int RetVal = ERROR_SUCCESS;
	std::string WaitString = GetPayload(Line);
	int Delay = atoi(WaitString.c_str());

	if (Delay < 200)
		Delay=200;
	if (Delay>20000)
		Delay = 20000;

	if (!WaitForDeviceChange(Delay))
		RetVal = ERROR_COMMUNICATION;

	return RetVal;
}

int CDeviceConfig::Configure(const char *pFileName, CallbackProgress_t pProgress, const char * pDeviceName, int baudrate)
{
	int RetVal=ERROR_SUCCESS;
	m_pComm = CreateFlasher();

	std::string sDeviceName;
	if (sLastGoodConnection.length() > 0)
		sDeviceName = sLastGoodConnection;
	else if (pDeviceName != NULL)
		sDeviceName = pDeviceName;

	if(!m_pComm->Connect(sDeviceName.c_str(), baudrate))
		RetVal = ERROR_COMMUNICATION;

	if (RetVal == ERROR_SUCCESS)
		RetVal = m_pComm->PrepareConfig(pProgress);
	
	if (pFileName == NULL)
		RetVal = ERROR_INVALID_PARAMETER;

	if (RetVal == ERROR_SUCCESS)
	{
		CallbackProgress(EV_ConfigStarted, 0);
		RetVal = ParseFile(pFileName);
	}

	return RetVal;
}

LIB_API int HsmConfigureDevice2(const char *pFileName, CallbackProgress_t pProgress, const char * pDeviceName, int baudrate)
{
	int RetVal = Framework_Init();
	if (RetVal != ERROR_SUCCESS)
		return RetVal;

	CDeviceConfig cfg;
	return cfg.Configure(pFileName, pProgress, pDeviceName, baudrate);
}

LIB_API int HsmConfigureDevice(const char *pFileName, const char * pDeviceName, int baudrate)
{
	return HsmConfigureDevice2(pFileName, NULL, pDeviceName, baudrate);
}

}	// namespace HsmDeviceComm
