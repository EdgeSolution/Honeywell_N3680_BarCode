
#pragma once

enum phases_t
{
	EV_DownloadStarted,					//!< Send as a first action from the download thread
	EV_Downloading,						//!< Send to update the progress bar while downloading

	EV_FlashStarted,						//!< Send when starting to flashing
	EV_Flashing,							//!< Send whyle flashing
	EV_FlashFinished,						//!< Send after flashing is finshed (see LPARAM for error code)

	EV_NothingToDo,						//!< Send if version of FW in device is ok
// Keep the order of the above to ensure compatibility with old clients

	EV_DownloadFinished,					//!< Send after download is finished to update the GUI
	EV_FlashStillRunning,					//!< Send if flashing takes longer that average

	EV_BringDeviceIntoBootMode,			//!< Send to ask user for power cycle of device
	EV_BootMode,							//!< Send after device is probaly in bootmode (see LPARAM for error code)
	EV_DeviceChangedInterface,			//!< Send after device is probaly in bootmode (see LPARAM for error code)
	EV_Percent,							//!< Send to show progress not releated to download or flashing
	EV_Unknown,							//!< Sanity

	EV_ConfigStarted,						//!< Send when starting to configure device
	EV_Configering,						//!< Send while configuring device
	EV_ConfigFinished,						//!< Send after configuring device (see LPARAM for error code)

	EV_Aborted,							//!< aborted by caller
	EV_ConnectionStatus,					//!< whether we are succesfully connected to device
	EV_Validating,							//!< validating firmware and device

	// backwards compatibility for clients
	DownloadStarted= EV_DownloadStarted,
	Downloading= EV_Downloading,
	FlashStarted= EV_FlashStarted,
	Flashing= EV_Flashing,
	FlashFinished= EV_FlashFinished,
	NothingToDo= EV_NothingToDo,
};

typedef bool(* CallbackProgress_t)(phases_t ChangePhase, int Percent);




