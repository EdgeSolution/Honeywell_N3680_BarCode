package honeywell.hedc;


import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.hardware.usb.UsbDevice;
import android.os.Handler;
import android.os.Message;
//import android.os.SystemClock;

import java.io.FileInputStream;
import java.text.ParseException;
import java.util.HashMap;
import java.util.Iterator;
import android.content.BroadcastReceiver;
import android.content.Intent;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import android.util.Log;
import java.io.File;

public class EngineCommunication extends GuiHelpers {

    // BEGIN CLASS VARIABLES
    private static final String TAG = "honeywell";
    private static final String ACTION_USB_PERMISSION = "honeywell.usbconnection";

    protected enum HandlerCommands {
        UNKNOWN,
        BARCODE_MSG, GOT_DATA, GOT_IMAGE,
        BARCODE_MSG_RAW,
        PERMIT_USB, END_USB, RECONNECT_USB, LOSTPERMITTION_USB,
        NOTIFY_STATE,
        BARCODE_MSG_LEGACY, GOT_IMAGE_LEGACY,
        PROGRESS,
    }

    public enum ConnectionState {
        UNKNOWN,
        Connected,
        Disconnected,
        PAUSED, ASKPERMISSION, CONNECTING,
    }

    protected volatile ConnectionState m_ConStatus = ConnectionState.UNKNOWN;

    private String m_sProduct = "";
    private String m_sFirmware = "";
    private String m_sRevision = "";
    private String m_sSerial = "";
    private String m_sInterfaceName = "";

    volatile UsbInterface m_UsbInterface = null;
    volatile UsbEndpoint m_EP_In, m_EP_Out;
    volatile UsbDeviceConnection m_Connection = null;
    UsbDevice m_HoneywellDevice = null;
    PendingIntent m_PermissionIntent;
    UsbManager m_UsbManager;
    UsbBroadcastReceiver m_UsbReceiver;

    enum InterfaceType { UNKNOWN, CDC_ACM, HIDPOS }
    InterfaceType interface_type = InterfaceType.UNKNOWN;

    private final Lock m_ConnectionLock = new ReentrantLock(true);

    private boolean m_ManualDisconnect = false;
    private int m_InterfaceIndex = -1;
    private int m_UsbPid = 0;

    private static final int PidMask=0x1F;
    private static final int VID_WA=0x0536;
    private static final int VID_METRO=0x0c2e;

    // One interface for all callbacks
    public interface OnHedc {
        void OnBarcodeData(String str);
        void OnBarcodeDataRaw(final byte[] data);
        void OnConnectionStateEvent(ConnectionState state);
        void OnImageData(final Bitmap bitmap);
        void OnProgress(int wparam, int lparam);
    }

    private OnHedc mHedcListener;
    // END CLASS VARIABLES

    // Parameterized class constructor
    public EngineCommunication(Context context){
        super(context);
        this.m_UsbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        this.m_PermissionIntent = PendingIntent.getBroadcast(context, 0, new Intent(ACTION_USB_PERMISSION), 0);
    }

    public EngineCommunication(Context context, OnHedc Listener){
        super(context);
        mHedcListener = Listener;
        this.m_UsbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        this.m_PermissionIntent = PendingIntent.getBroadcast(context, 0, new Intent(ACTION_USB_PERMISSION), 0);
    }

    public void OnDestroy() {
        Log.i(TAG, "onDestroy");
        UnRegisterUsb();
        Disconnect();
        DestroyDeviceObject();
        mHedcListener=null;
        m_UsbManager=null;
        m_PermissionIntent=null;
    }

    public void OnCreate() {
        if (!isConnecting()) {
            Log. d(TAG, "OnCreate");
            RegisterUsb();
            if(!m_ManualDisconnect) {
                AskForPermission();
            }
        } else {
            Log.d(TAG, "OnCreate ignored");
        }
    }


    public void OnExit(){
        Disconnect();
        ((Activity) this.context).finish();
    }

    protected void InitCpp() {
        ConnectEngine(getEngineFileDescriptor(), getOutputEndpointAddr(), getInputEndpointAddr(), getInterfaceType(), getOutputEndpointSize(), getInputEndpointSize());
    }

    protected int getOutputEndpointSize() {
        return m_EP_Out.getMaxPacketSize();
    }

    protected int getInputEndpointSize(){
        return m_EP_In.getMaxPacketSize();
    }

    protected int getOutputEndpointAddr() {
        return m_EP_Out.getAddress();
    }

    protected int getInputEndpointAddr(){
        return m_EP_In.getAddress();
    }

    protected int getInterfaceType(){
        if (interface_type == InterfaceType.CDC_ACM) return 1;
        else if (interface_type == InterfaceType.HIDPOS) return 2;
        else return 0;
    }

    // ScanEngine class variable: UsbDeviceConnection connection
    // returns -1 if device not opened
    protected int getEngineFileDescriptor(){
        return m_Connection.getFileDescriptor();
    }

    // Functions for onClickToggleConnect(View view)
    // Function was meant to disconnect if connected, connect if disconnected
    // when connected, manual disconnect available, if button pressed send END_USB
    // when NOT connected, manual disconnect NOT available, if button pressed send PERMIT_USB

    // FIXME:  need better name. Return value so caller knows the state?
    protected void AllowConnDisconn(){
        if((m_ConStatus == ConnectionState.Connected)){
            Log. d(TAG, "Button off");
            m_ManualDisconnect = true;
            sendHandlerMessage(HandlerCommands.END_USB);
        }else{
            Log. d(TAG, "Button on");
            m_ManualDisconnect = false;
            sendHandlerMessage(HandlerCommands.PERMIT_USB);
        }
    }

    public void AllowDisconnect(){
        if((m_ConStatus == ConnectionState.Connected)){
            Log. d(TAG, "Button off");
            m_ManualDisconnect = true;
            sendHandlerMessage(HandlerCommands.END_USB);
        }
    }

    public void AllowConnect(){
        if((m_ConStatus != ConnectionState.Connected)){
            Log. d(TAG, "Button on");
            m_ManualDisconnect = false;
            sendHandlerMessage(HandlerCommands.PERMIT_USB);
        }
    }

    // Return String functions, and UsbPid function for UpdateGUICtrl()
    public String GetInterfaceName(){
        return m_sInterfaceName;
    }

    public int GetUsbPid(){
        return m_UsbPid;
    }

    public boolean isConnected(){
        return (m_ConStatus == ConnectionState.Connected);
    }

    public boolean isConnecting() {
        return (m_ConStatus == ConnectionState.ASKPERMISSION)||(m_ConStatus == ConnectionState.CONNECTING);
    }

    protected void RegisterUsb() {
        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        this.m_UsbReceiver = new UsbBroadcastReceiver();
        context.registerReceiver(this.m_UsbReceiver, filter);
        filter = null;
        if(this.m_HoneywellDevice != null){
            this.m_UsbManager.requestPermission(this.m_HoneywellDevice, this.m_PermissionIntent);
        }
    }

    protected void UnRegisterUsb() {
        context.unregisterReceiver(m_UsbReceiver);
    }

    protected void AskForPermission() {
        Log. d(TAG, "AskForPermission");
        if (m_Connection == null) {

            LocateHsmDevice(); // inits Honeywelldevice

            if (m_HoneywellDevice != null) {
                if (!m_UsbManager.hasPermission(m_HoneywellDevice)) {
                    Log. d(TAG, "AskForPermission ask");
                    m_ConStatus = ConnectionState.ASKPERMISSION;
                    m_UsbManager.requestPermission(m_HoneywellDevice, m_PermissionIntent);
                    // see usbReceiver for more actions. It gets called by the OS.
                } else {
                    Log. d(TAG, "AskForPermission already have permission, calling ReConnect now");
                    m_ConStatus = ConnectionState.CONNECTING;
                    sendHandlerMessage(HandlerCommands.RECONNECT_USB);
                }
            } else {
                m_ConStatus = ConnectionState.Disconnected;
                OnConnectionStateEvent();
                Log.d(TAG,"No Engine found");
            }
        } else if (m_ConStatus != ConnectionState.Connected) {
            Log. d(TAG, "Skipped AskForPermission, calling ReConnect now");
            sendHandlerMessage(HandlerCommands.RECONNECT_USB);
        } else {
            Log. d(TAG, "Skipped AskForPermission, nothing to do");
        }
    }

    protected void ReConnect() {
        Log.d(TAG, "ReConnect");
        setupConnection();
        if (m_Connection == null) {
            Log.d(TAG, "ReConnect creating connection");
            m_Connection = m_UsbManager.openDevice(m_HoneywellDevice);
        }
        if ( (m_Connection != null) && isDeviceValid() ) {
            m_Connection.claimInterface(m_UsbInterface, true);
            Log.d(TAG, "ReConnect2");
            m_ConStatus = ConnectionState.Connected;
            InitCpp();
            if (!isFirmwareReplace()) {
                StartEngine();
            }
        } else {
            Log.d(TAG, "ReConnect failed");
        }
    }

    // Disconnect engine, but keep other info intact
    protected void PauseConnection() {
        Log.d(TAG, "PauseConnection");
        if ( m_Connection != null) {
            m_ConStatus = ConnectionState.PAUSED;
            StopEngine();
            m_ConnectionLock.lock();    // wait for communication has been ended
            if(m_UsbInterface!=null)
                m_Connection.releaseInterface(m_UsbInterface);
            m_Connection.close();
            m_Connection = null;
            m_ConnectionLock.unlock();
        }
    }

    // Totally disconnect. All connection info deleted
    public void Disconnect() {
        Disconnect(false);
    }

    // Totally disconnect. All connection info deleted
    protected void Disconnect(boolean unplug) {
        if ( m_Connection != null) {
            Log. d(TAG, "Disconnect1");
            PauseConnection();
            interface_type = InterfaceType.UNKNOWN;
            m_HoneywellDevice=null;
            m_sProduct = "";
            m_sSerial = "";
            m_sFirmware = "";
            m_sRevision = "";

            m_InterfaceIndex=-1;
            DisconnectEngine(unplug);

            Log.d(TAG,"Engine disconnected");
        } else {
            Log. d(TAG, "Skipped Disconnect, already disconnected");
        }
        m_ConStatus = ConnectionState.Disconnected;
    }

    // New BroadcastReceiver implementation
    class UsbBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            //Log.d(TAG, "onReceiveBroadcast");
            String action = intent.getAction();
            if ( m_Connection == null) {
                if (ACTION_USB_PERMISSION.equals(action)) {
                    // broadcast is like an interrupt and works asynchronously with the class, it must be synced just in case
                    synchronized (this) {
                        if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                            // this broadcast comes several times in short distance
                            if(m_ConStatus == ConnectionState.ASKPERMISSION) {
                                Log.d(TAG, "onReceiveBroadcast connecting");
                                m_ConStatus = ConnectionState.CONNECTING;
                                sendHandlerMessage(HandlerCommands.RECONNECT_USB);
                            } else {
                                Log.d(TAG, "onReceiveBroadcast skipped");
                            }
                        } else {
                            Log.d(TAG, "onReceiveBroadcast lost permission");
                            sendHandlerMessage(HandlerCommands.LOSTPERMITTION_USB);
                        }
                    }
                }
            } else {
                Log.d(TAG, "onReceiveBroadcast already connected");
            }

            if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
                Log.d(TAG, "ACTION_USB_DEVICE_ATTACHED");
                UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (IsHsmDevice(device.getVendorId())) {
                    sendHandlerMessage(HandlerCommands.PERMIT_USB);
                }
            }
            else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
                Log.d(TAG, "ACTION_USB_DEVICE_DETACHED");
                UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                // Is it our device?
                if (IsSameDevice(device, m_HoneywellDevice)) {
                    sendHandlerMessage(HandlerCommands.END_USB);
                }
            }
        }
    }


    protected boolean isDeviceValid() {
        return ((m_EP_In != null)&&(m_EP_Out!=null)&&(m_UsbInterface!=null));
    }

    public boolean isConnectionValid() {
        return (m_InterfaceIndex >= 0) && (m_Connection != null);
    }

    protected void setupConnection() {
        Log. d(TAG, "setupConnection");
        m_EP_In = null;
        m_UsbInterface  = null;
        m_EP_Out = null;

        // find the right interface
        for(int i = 0; i < m_HoneywellDevice.getInterfaceCount(); i++) {
            UsbInterface Interface = m_HoneywellDevice.getInterface(i);
            // Log. d(TAG, Interface.getName());
            if(Interface.getInterfaceClass() == UsbConstants.USB_CLASS_CDC_DATA) {
                // communications device class (CDC) type device
                m_UsbInterface = Interface;
                m_InterfaceIndex=i;
                FindEndpoints(UsbConstants.USB_ENDPOINT_XFER_BULK);
                break;
            } else if(Interface.getInterfaceClass() == UsbConstants.USB_CLASS_HID) {
                // HidPos type device
                if (Interface.getName().equals("HID POS")) {
                    m_UsbInterface = Interface;
                    m_InterfaceIndex=i;
                    FindEndpoints(UsbConstants.USB_ENDPOINT_XFER_INT);
                    break;
                }
                if (false && (Interface.getName().equals("REM"))) {   // not for now
                    m_UsbInterface = Interface;
                    m_InterfaceIndex=i;
                    FindEndpoints(UsbConstants.USB_ENDPOINT_XFER_INT);
                    break;
                }
            }
        }
        if ((m_EP_In == null)||(m_EP_Out==null)||(m_UsbInterface==null)) {
            ShowAlert("setupConnection error!!!");
        }
    }

    protected void FindEndpoints(int EpType) {
        // find the endpoints
        for(int j = 0; j < m_UsbInterface.getEndpointCount(); j++) {
            UsbEndpoint Ep = m_UsbInterface.getEndpoint(j);
            if(Ep.getDirection() == UsbConstants.USB_DIR_OUT && Ep.getType() == EpType) {
                // from android to device
                m_EP_Out = Ep;
            }

            if(Ep.getDirection() == UsbConstants.USB_DIR_IN && Ep.getType() == EpType) {
                // from device to android
                m_EP_In = Ep;
            }
        }
    }

    protected static boolean IsSameDevice(UsbDevice dev1, UsbDevice dev2) {
        if ((dev1 == null) || (dev2 == null))
            return false;

        boolean vid = (dev1.getVendorId() == dev2.getVendorId());
        boolean pid = (dev1.getProductId() == dev2.getProductId());
        boolean ser = false;
        if (vid && pid) {
            String ser1 = dev1.getSerialNumber();
            String ser2 = dev2.getSerialNumber();
            if (ser1 != null && ser2 != null) {
                ser = (dev1.getSerialNumber().equals(dev2.getSerialNumber()));
            }
        }
        return (vid && pid && ser);
    }

    protected static boolean IsHsmDevice(int vid) {
        return (vid==VID_METRO)||(vid==VID_WA);
    }

    protected static boolean IsCdcAcmDevice(int pid){ // for now using the pid
        int PidOffset=pid&PidMask;
        return (PidOffset==0xA)||(PidOffset==0x14);
    }

    // For now using the pid.
    // Better would be to use the HID  usages, but Android seems to not support that.
    protected static boolean IsHidPosDevice(int pid) {
        // Our keyboard interface can be a composite with keyboard and HidPos.
        int PidOffset=pid&PidMask;
        boolean Keyboard = (PidOffset==0x1)||(PidOffset==0xF);
        boolean HidPos = (PidOffset==0x7)||(PidOffset==0x13);
        return Keyboard||HidPos;
    }

    protected static boolean IsSupportedDevice(UsbDevice device) {
        return (IsHsmDevice(device.getVendorId())
                && (IsCdcAcmDevice(device.getProductId()) || IsHidPosDevice(device.getProductId())));
    }


    public String listUsbDevices(boolean all){
        Log. d(TAG, "listUsbDevices");
        HashMap<String, UsbDevice> deviceList = m_UsbManager.getDeviceList();
        m_HoneywellDevice = null;
        if(deviceList.size() == 0) {
            Log. d(TAG, "no usb devices found");
            return "no usb devices found";
        }
        Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
        String returnValue = "";
        // Unnecessary variables????
        int index, epi;
        UsbInterface usbInterface;
        interface_type = InterfaceType.UNKNOWN;
        while(deviceIterator.hasNext()) {
            UsbDevice device = deviceIterator.next();
            if (IsSupportedDevice(device) || all) {
                returnValue += GetDevicedetails(device);
            }
        }
        return returnValue;
    }

    public void LocateHsmDevice() {
        Log. d(TAG, "LocateHsmDevice");
        HashMap<String, UsbDevice> deviceList = m_UsbManager.getDeviceList();
        m_HoneywellDevice = null;
        if(deviceList.size() == 0) {
            Log. d(TAG, "no usb devices found");
            m_UsbPid=0;
            return;
        }
        Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
        String returnValue = "";
        int index;
        int epi;
        UsbInterface usbInterface;
        interface_type = InterfaceType.UNKNOWN;
        while(deviceIterator.hasNext() && (interface_type == InterfaceType.UNKNOWN)) {
            UsbDevice device = deviceIterator.next();
            if (IsHsmDevice(device.getVendorId())) {
                m_HoneywellDevice = device;
                m_UsbPid=device.getProductId();
                //returnValue += GetDevicedetails(device);
                if (IsCdcAcmDevice(m_UsbPid)) {  /* Com Port emulation Interface */
                    Log.d(TAG, "Found CDC-ACM device");
                    interface_type = InterfaceType.CDC_ACM; /* CDC_ACM is Com Port emulation */
                } else if (IsHidPosDevice(m_UsbPid)) {
                    Log.d(TAG, "Found HidPos device");
                    interface_type = InterfaceType.HIDPOS; /* Either pure or as composite with Keyboard */
                }
            }
        }
    }

    public String GetDevicedetails(UsbDevice device) {
        String returnValue = "";
        int index;
        int epi;

        returnValue += "Model: " + device.getDeviceName()+"\r\n";
        returnValue += "ID: " + device.getDeviceId()+"\r\n";
        returnValue += "Class: " + device.getDeviceClass()+"\r\n";
        returnValue += "Protocol: " + device.getDeviceProtocol()+"\r\n";
        returnValue += "Vendor ID " + device.getVendorId()+"\r\n";
        returnValue += "Product ID: " + device.getProductId()+"\r\n";
        returnValue += "S/N: " + device.getSerialNumber() + "\r\n";
        returnValue += "Interface count: " + device.getInterfaceCount()+"\r\n";
        returnValue += "---------------------------------------"+"\r\n";

        for (index = 0; index < device.getInterfaceCount(); index++) {
            UsbInterface mUsbInterface = device.getInterface(index);
            returnValue += "  *****     *****\r\n";
            returnValue += "  Interface index: " + index+"\r\n";
            returnValue += "  Interface ID: " + mUsbInterface.getId()+"\r\n";
            returnValue += "  Inteface class: " + mUsbInterface.getInterfaceClass()+"\r\n";
            returnValue += "  Interface protocol: " + mUsbInterface.getInterfaceProtocol()+"\r\n";
            returnValue += "  Endpoint count: " + mUsbInterface.getEndpointCount()+"\r\n";
            // Get endpoint details
            for (epi = 0; epi < mUsbInterface.getEndpointCount(); epi++) {
                UsbEndpoint mEndpoint = mUsbInterface.getEndpoint(epi);
                returnValue += "    ++++   ++++   ++++\r\n";
                returnValue += "    Endpoint index: " + epi+"\r\n";
                returnValue += "    Attributes: " + mEndpoint.getAttributes()+"\r\n";
                returnValue += "    Direction: " + mEndpoint.getDirection()+"\r\n";
                returnValue += "    Number: " + mEndpoint.getEndpointNumber()+"\r\n";
                returnValue += "    Interval: " + mEndpoint.getInterval()+"\r\n";
                returnValue += "    Packet size: " + mEndpoint.getMaxPacketSize()+"\r\n";
                returnValue += "    Type: " + mEndpoint.getType()+"\r\n";
            }
        }
        returnValue += "\n";
        return returnValue;
    }

    public int SetManualTrigger() {
        return SetTriggerMode(0);
    }

    public int SetPresentationMode() {
        return SetTriggerMode(3);
    }

    public int SetTriggerMode(int mode) {
        Log. d(TAG, "SetTriggerMode");
        String cmd = String.format("TRGMOD%d!", mode);
        return ExecuteMenuCommand(cmd) ? Constants.ERROR_SUCCESS : Constants.ERROR_CMD_FAILED;
    }

    public void GetProductInfo() {
        m_sSerial = m_HoneywellDevice.getSerialNumber();
        // next do cache the results into class members
        GetFirmwareVersion();
        GetFirmwareRevision();
        GetModelName();

        if (interface_type == InterfaceType.HIDPOS) {
            m_sInterfaceName = "HidPos ";
        }
        if (interface_type == InterfaceType.CDC_ACM) {
            m_sInterfaceName = "CDC-ACM ";
        }
    }

    public int GetIntSetting(String cmd) {
        int val = -1;
        if (ExecuteMenuCommand(cmd +"?!")) {
            String sResponse = GetCommandResponse().replace(cmd,"");
            try{
                if(!sResponse.isEmpty())
                    val = Integer.parseInt(sResponse);
            } catch(Exception e){
                Log. d(TAG, "GetIntSetting cannot parse: " + sResponse);
            }
        } else{
            Log. d(TAG, "GetIntSetting bad command: " + cmd);
        }
        return val;
    }

    public boolean SetIntSetting(String cmd) {
        return ExecuteMenuCommand(cmd +"!");
    }

    public String GetStringSetting(String cmd) {
        String sVal = "";
        if (ExecuteMenuCommand(cmd +"?!")) {
            sVal = GetCommandResponse().replace(cmd,"");
        }
        return sVal;
    }

    public String GetModelName() {
        if (m_sProduct.length() == 0) {
            m_sProduct = GetStringSetting("P_NAME");
        }
        return m_sProduct;
    }

    public String GetFirmwareVersion() {
        if (m_sFirmware.length() == 0) {
            if (ExecuteMenuCommand("REV_WA?.")) {
                m_sFirmware = GetCommandResponse().replace("REV_WA: ", "");
            }
        }
        return m_sFirmware;
    }

    public String GetFirmwareRevision() {
        if (m_sRevision.length() == 0) {
            String RawRev = GetStringSetting("REV_TD?.");
            // clean and make it shorter
            String Rev1 = RawRev.replace(": $ProjectRevision: ", "r");
            m_sRevision = Rev1.replace(" Local Modifications Exist", "M");
        }
        return m_sRevision;
    }

    public String GetSerialNumber()
    {
        return m_sSerial;
    }

    public native static String GetLibraryVersion();

    public int ReplaceFirmwareFile(String sFilepath)
    {
        return ExecuteFirmwareFlashFile(sFilepath);
    }

    private void sendHandlerMessage(HandlerCommands cmd) {
        handler.sendEmptyMessage(cmd.ordinal());
    }

    private void sendHandlerMessage(Message msg) {
        handler.sendMessage(msg);
    }

    protected void OnConnectionStateEvent() {
        if (mHedcListener!=null) {
            mHedcListener.OnConnectionStateEvent(m_ConStatus);
        }
    }

    protected void OnBarcodeData(String str) {
        if ((mHedcListener!=null) && (str!=null)) {
            mHedcListener.OnBarcodeData(str);
        }
    }

    protected void OnBarcodeDataRaw(final byte [] rxBytes) {
        if (rxBytes!=null) {
            if (mHedcListener != null) {
                mHedcListener.OnBarcodeDataRaw(rxBytes);
            }
        }
    }

    protected void OnImageData(final Bitmap bitmap) {
        if ((mHedcListener!=null) && (bitmap!=null)) {
            mHedcListener.OnImageData(bitmap);
        }
    }

    // Just a dummy to be overwritten in derived compatibility layer
    protected void OnImageDataLegacy(final byte[] RawImage) {
    }

    protected void OnProgress(int wparam, int lparam) {
        if (mHedcListener!=null) {
            mHedcListener.OnProgress(wparam, lparam);
        }
    }

    public Handler handler = new Handler() {
        public void handleMessage(Message msg) {
            //Log.d(TAG, "handler");
            HandlerCommands cmd = HandlerCommands.values()[msg.what];
            switch (cmd) {
                case PROGRESS:
                    if (msg.arg1 == Constants.EV_FlashFinished) {
                        StartEngine();
                    }
                    OnProgress(msg.arg1, msg.arg2);
                    break;
                case PERMIT_USB:
                    AskForPermission();
                    break;
                case RECONNECT_USB:
                    ReConnect();
                    break;
                case LOSTPERMITTION_USB:
                    ShowAlert("Permission denied for USB device");
                    Disconnect(false);
                    OnConnectionStateEvent();
                    break;
                case END_USB:
                    Disconnect(true);
                    OnConnectionStateEvent();
                    break;
                case NOTIFY_STATE:
                    OnConnectionStateEvent();
                    break;
                case BARCODE_MSG:
                    // Define this in main
                    OnBarcodeData((String)msg.obj);
                    break;
                case GOT_IMAGE:
                    // Define this in main
                    OnImageData((Bitmap)msg.obj);
                    break;
                case BARCODE_MSG_RAW:
                    // Define this in main
                    OnBarcodeDataRaw((byte []) msg.obj);
                    break;
                case GOT_IMAGE_LEGACY:
                    // Define this in main
                    OnImageDataLegacy((byte [])msg.obj);
                    break;
            }
        }
    };


    public void GetImage() {
        ExecuteMenuCommand("IMGSNP;IMGSHP6F100J!");
    }

    protected void ReceiveRawFromDeviceAsync(int kind, byte [] rxBytes) {
        Message msg = Message.obtain(handler, HandlerCommands.BARCODE_MSG_RAW.ordinal(), rxBytes);
        handler.sendMessage(msg);
    }

    // Called fromm CPP
    protected void ReceiveText(int kind, String str) {
        // todo: handle kind
        Message msg = Message.obtain(handler, HandlerCommands.BARCODE_MSG.ordinal(), str);
        handler.sendMessage(msg);
    }

    // Called fromm CPP
    protected void ReceiveImage(int kind, byte [] img) {
        // todo: handle kind
        Bitmap bitmap = BitmapFactory.decodeByteArray(img, 0, img.length);
        Message msg = Message.obtain(handler, HandlerCommands.GOT_IMAGE.ordinal(), bitmap);
        handler.sendMessage(msg);
    }

    // Called fromm CPP
    public void ReceiveStatus(int wparam, int lparam) {
        Message msg = Message.obtain(handler, HandlerCommands.PROGRESS.ordinal(), wparam, lparam);
        handler.sendMessage(msg);
    }

    private void StartEngine() {
        GetProductInfo();
        sendHandlerMessage(HandlerCommands.NOTIFY_STATE);
    }

    private void StopEngine() {
        sendHandlerMessage(HandlerCommands.NOTIFY_STATE);
    }

    // Initialize details struct
    private native void ConnectEngine(int fd, int outEP, int inEP, int interfaceType, int inSize, int outSize);
    private native void DisconnectEngine(boolean unplug);
    private native void DestroyDeviceObject();
    private native int SendToDevice(final byte [] cmd);
    private native int SendToDeviceString(final String str);

    public native boolean ExecuteMenuCommand(String str);
    public native boolean ExecuteProdMenuCommand(String str);
    public native int ExecuteFirmwareFlashFile(String str);
    public native int ExecuteFirmwareFlashArray(byte[] firmware);
    public native void AbortFirmwareReplace();
    public native boolean isFirmwareReplace();
    public native String GetCommandResponse();


    private byte[] fullyReadFileToBytes (File file)
    {
        FileInputStream input = null;
        if (file.exists()) try
        {
            input = new FileInputStream(file);
            int len = (int) file.length();
            byte[] data = new byte[len];
            int count, total = 0;
            while ((count = input.read (data, total, len - total)) > 0) total += count;
            return data;
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
        finally
        {
            if (input != null) try
            {
                input.close();
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }
        return null;
    }


    public int UpgradeFirmware(File Firmwarefile)
    {
        return ExecuteFirmwareFlashArray(fullyReadFileToBytes(Firmwarefile));
    }

//  public native static short WritePortSetup(byte port, int baudRate, byte byteSize, byte parity, byte stopBit);
//  public native static short InitializeEx( String pszFolderPath, byte[]pStatus, boolean log);
//  public native static short Deinitialize();
//  public native static short Connect();
    public native int SendMenuCommand(byte[] pInputBuffer, int nBytesInInputBuffer, byte[]pOutputBuffer, int nSizeOfOutputBuffer,int [] nBytesReturned, int TimeOutMsec);
    public native int StartReadingSession();
    public native int StopReadingSession();
    public native int BeepSuccess();
    public native int BeepError();
    //public native static short GetImage(byte[]pOutputBuffer, long nSizeOfOutputBuffer,long[] nBytesReturned);
    //public native static short DeviceIOControl( long ioControlCode,byte[]pInputBuffer,  long nBytesInInputBuffer,
    //                                            byte[]pOutputBuffer, long nSizeOfOutputBuffer,long[] nBytesReturned);
//    public native static short Disconnect();

//    public native static short RegisterCallback(HEDC_App theApp);


    // Used to load the 'hedc' library on application startup.
    static {
        System.loadLibrary("hedc");
    }

} // EOF ScanEgnine class
