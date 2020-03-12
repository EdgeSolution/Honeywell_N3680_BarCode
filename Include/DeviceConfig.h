#pragma once

namespace HsmDeviceComm {

class CDeviceConfig
{
public:
	CDeviceConfig(void);
	virtual ~CDeviceConfig(void);

	int Configure(const char *pFileName, CallbackProgress_t pProgress, const char * pDeviceName, int baudrate);

protected:
	int ParseFile(const std::string Filename);
	int ParseLine(const std::string Line);
	int ParseMenuCommand(const std::string Line);
	int ParseWait(const std::string Line);
	std::string GetPayload(const std::string Line);

	virtual bool ExecuteMenuCommand(const std::string sCmd);
	virtual bool WaitForDeviceChange(int Delay);
	virtual bool CallbackProgress(phases_t Phase, int Percent);
protected:
	class CTxFirmware *m_pComm;
	int m_Percent;
};

}
