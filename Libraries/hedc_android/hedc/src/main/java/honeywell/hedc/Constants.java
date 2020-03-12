package honeywell.hedc;

public final class Constants {

    public static final int EV_DownloadStarted = 0;					//!< Send as a first action from the download thread
    public static final int EV_Downloading = 1;						//!< Send to update the progress bar while downloading

    public static final int EV_FlashStarted = 2;						//!< Send when starting to flashing
    public static final int EV_Flashing = 3;							//!< Send whyle flashing
    public static final int EV_FlashFinished = 4;					//!< Send after flashing is finshed (see LPARAM for error code)

    public static final int EV_NothingToDo = 5;						//!< Send if version of FW in device is ok

    public static final int EV_DownloadFinished = 6;				//!< Send after download is finished to update the GUI
    public static final int EV_FlashStillRunning = 7;				//!< Send if flashing takes longer that average

    public static final int EV_BringDeviceIntoBootMode = 8;		//!< Send to ask user for power cycle of device
    public static final int EV_BootMode = 9;        					//!< Send after device is probaly in bootmode (see LPARAM for error code)
    public static final int EV_DeviceChangedInterface = 10;		//!< Send after device is probaly in bootmode (see LPARAM for error code)
    public static final int EV_Percent = 11;							//!< Send to show progress not releated to download or flashing
    public static final int EV_Unknown = 12;							//!< Sanity

    public static final int EV_ConfigStarted = 13;					//!< Send when starting to configure device
    public static final int EV_Configering = 14;						//!< Send while configuring device
    public static final int EV_ConfigFinished = 15;					//!< Send after configuring device (see LPARAM for error code)

    public static final int EV_Aborted = 16;							//!< aborted by caller
    public static final int EV_ConnectionStatus = 17;				//!< whether we are succesfully connected to device
    public static final int EV_Validating = 18;						//!< validating firmware and device


    public static final int rError = 0xE0000000;
    public static final int ERROR_SUCCESS = 0;

    public static final int ERROR_CANCELED_BY_USER = rError+1;		//!< User aborted the action
    public static final int ERROR_NO_FW_FILE        = rError+2;		//!< This is no firmware file
    public static final int ERROR_PARSE_FILE		= rError+3;		//!< Parsing the firmware file did fail
    public static final int ERROR_FILE_TO_BIG		= rError+4;		//!< Firmware file is too big for this device
    public static final int ERROR_WRONG_FW			= rError+5;		//!< Firmware is for a different device
    public static final int ERROR_FILE_NOT_FOUND	= rError+6;		// declared in winerror.h
    public static final int ERROR_NOT_ENOUGH_MEMORY	= rError+7;	    // declared in winerror.h
    public static final int ERROR_READ_FAULT		= rError+8;		// declared in winerror.h
    public static final int ERROR_INVALID_PARAMETER	= rError+9;	    // declared in winerror.h
    public static final int ERROR_NOT_SUPPORTED     = rError + 10;	// declared in winerror.h

    public static final int ERROR_INVALID_INPUT_BUFFER = rError + 81;		//!< Input buffer might be NULL
    public static final int ERROR_OUTPUT_BUFFER_TOO_SMALL = rError + 82;	//!< Output buffer either NULL or too small

    public static final int ERROR_FRAMEWORK_FAILED = rError + 90;	//!< MFC/QT etc could not be initialized
    public static final int ERROR_GETMODUL_FAILED  = rError + 91;	//!< MFC/QT etc could not be initialized
    public static final int ERROR_REGISTRY		    = rError + 93;	//!< Registry access failed
    public static final int ERROR_INTERNAL		    = rError + 99;	//!< Internal error (should never show up)

    public static final int ERROR_THREAD			    = rError+100;	//!< Thread error
    public static final int ERROR_THREAD_RUNNING	= rError+101;	//!< Thread did already run as we tried to create it
    public static final int ERROR_THREAD_NOT_CREATED= rError+102;	//!< Thread could not be created

    public static final int ERROR_COMMUNICATION	    = rError+200;	//!< A communication error occurred
    public static final int ERROR_PORT_NOT_OPEN	    = rError+201;	//!< Port/device open failed (is it in use by someone else?)
    public static final int ERROR_CANCELED_BY_REMOTE=rError+202;	//!< Remote side did cancel the XModem transfer
    public static final int ERROR_NO_SYNC			= rError+203;	//!< We did not receive the XModem request to send signal
    public static final int ERROR_XMIT_ERROR		= rError+204;	//!< Transmitting failed
    public static final int ERROR_DEVICETYPE		= rError+205;	//!< Device reports an invalid device type
    public static final int ERROR_AUTOSELECT_FAILED	= rError+206;	//!< Proably more than one device is connected; so autoselect needs help.
    public static final int ERROR_CMD_UNKNOWN		= rError+207;	//!< Command not implemented
    public static final int ERROR_CMD_FAILED		= rError+208;	//!< Command returned error

    public static final int ERROR_BOOTMODE_FAILED	= rError+220;	//!< No Boot mode communication found

    public static final int ERROR_FLASH_FAILED		= rError+300;	//!< Storing the firmware into the flash ROM failed
    public static final int ERROR_FLASH_UNSURE		= rError+301;	//!< We got no ACK after the storing; but could communicate with the device
    public static final int ERROR_FLASH_NO_RESPOND	= rError+302;	//!< Flashing seamed to work; but device does not respond
    public static final int ERROR_FLASH_WRONG_FW	= rError+304;	//!< Device complained about the firmware

    // Legacy error numbers
    // Note, we had to add some offset to avoid conflicts in different environments.
    private static final int LEGOFFSET=0x4000;
    public static final int NO_ERROR_ = 0;
    public static final int INVALID_INPUT_BUFFER = LEGOFFSET+1;
    public static final int OUTPUT_BUFFER_TOO_SMALL = LEGOFFSET+2;
    public static final int REGISTRY_ERROR = LEGOFFSET+3;
    public static final int DLL_NOT_INITIALIZED = LEGOFFSET+7;
    public static final int NO_DEVICE_FOUND = LEGOFFSET+8;
    public static final int PORT_NOT_AVAILABLE = LEGOFFSET+9;
    public static final int CONNECTION_LOST = LEGOFFSET+10;
    public static final int OPERATION_CANCELLED = LEGOFFSET+11;
    public static final int INTERNAL_ERROR = LEGOFFSET+12;
    public static final int NOT_SUPPORTED = LEGOFFSET+13;
    public static final int INVALID_COMMAND = LEGOFFSET+14;
    public static final int INVALID_FIRMWARE = LEGOFFSET+15;
    public static final int INCOMPATIBLE_FIRMWARE = LEGOFFSET+16;
    public static final int DOWNLOAD_COMPLETE = LEGOFFSET+17;
}
