package honeywell.hedc_usb_com;

import android.content.Context;
import android.os.Message;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import static java.lang.System.arraycopy;
import honeywell.hedc.Constants;
import honeywell.hedc.EngineCommunication;


/* This class provides a compatibility layer for HEDC V1 clients.
* */
public class HEDCUsbCom extends EngineCommunication {

    private static final String TAG = "honeywell";
    Context UsbContext = null;

    private OnConnectionStateListener m_OnUpdateGUI;
    private OnBarcodeListener m_OnBarcode;
    private OnImageListener m_OnImage;
    private OnDownloadListener m_OnDownload;

    // Parameterized class constructor
    public HEDCUsbCom(Context context, OnConnectionStateListener onConnectionState, OnBarcodeListener onBarcode, OnImageListener onImage, OnDownloadListener OnDownload){
        super(context);
        UsbContext = context;
        m_OnUpdateGUI = onConnectionState;
        m_OnBarcode =  onBarcode;
        m_OnImage =  onImage;
        m_OnDownload = OnDownload;
    }

    // legacy interfaces for backwards compatibility
    public interface OnConnectionStateListener {
        void OnConnectionStateEvent(ConnectionState state);
    }

    public interface OnBarcodeListener {
        void OnBarcodeData(final byte[] data, int length);
    }

    public interface OnImageListener {
        void OnImageData(final byte[] RawImage, int sizeImage);
    }

    public interface OnDownloadListener {
        void OnDownloadData(int progress, short error);
    }

    public void OnDestroy() {
        Log.i(TAG, "onDestroy");
        super.OnDestroy();
        m_OnUpdateGUI=null;
        m_OnBarcode=null;
        m_OnImage=null;
        m_OnDownload=null;
    }

    @Override
    protected void OnConnectionStateEvent() {
        if (m_OnUpdateGUI!=null) {
            m_OnUpdateGUI.OnConnectionStateEvent(m_ConStatus);
        }
    }

    @Override
    protected void OnBarcodeDataRaw(final byte [] rxBytes) {
        if (rxBytes!=null) {
            if (m_OnBarcode != null) {
                m_OnBarcode.OnBarcodeData(rxBytes, rxBytes.length);
            }
        }
    }

    @Override
    protected void OnImageDataLegacy(final byte[] RawImage) {
        if ((m_OnImage!=null) && (RawImage!=null)) {
            m_OnImage.OnImageData(RawImage, RawImage.length);
        }
    }

    @Override
    protected void OnProgress(int wparam, int lparam) {
        if (m_OnDownload!=null) {
            m_OnDownload.OnDownloadData(wparam, (short)lparam);
        }
    }

    // Called fromm CPP
    @Override
    protected void ReceiveText(int kind, String str) {
        // todo: handle kind
        byte [] rxBytes = str.getBytes();
        ReceiveRawFromDeviceAsync(kind, rxBytes);
    }

    // Called fromm CPP
    @Override
    protected void ReceiveImage(int kind, byte [] img) {
        // todo: handle kind
        Message msg = Message.obtain(handler, HandlerCommands.GOT_IMAGE_LEGACY.ordinal(), img);
        handler.sendMessage(msg);
    }

    public void DisplayUsbDevicesList() {
        String s = "";
        s = listUsbDevices(false);
        if (s == "")
            s = "No device found";
        AlertDialog.Builder AlertDlg = new AlertDialog.Builder(UsbContext);
        AlertDlg.setPositiveButton("OK", null);
        AlertDlg.setTitle("Honeywell: USB devices attached");
        AlertDlg.setMessage(s);
        AlertDlg.show();
    }

    public void DisplayTERMID130() {
        AlertDialog.Builder AlertDlg = new AlertDialog.Builder(UsbContext);
        AlertDlg.setTitle("TERMID130 interface");
        AlertDlg.setMessage("This is just there for compatibility reasons");
        AlertDlg.show();
    }

    protected int StringToArray(String str, byte[] pOutputBuffer, int nSizeOfOutputBuffer, int[] nBytesReturned) {
        if (str.length() > nSizeOfOutputBuffer)
            return Constants.ERROR_OUTPUT_BUFFER_TOO_SMALL;

        arraycopy(str.getBytes(), 0, pOutputBuffer, 0, str.length());
        nBytesReturned[0] = str.length();
        return Constants.ERROR_SUCCESS;
    }

    public int GetLibraryVersion(byte[] pOutputBuffer, int nSizeOfOutputBuffer, int[] nBytesReturned) {
        String version = GetLibraryVersion();
        return StringToArray(version, pOutputBuffer, nSizeOfOutputBuffer, nBytesReturned);
    }

    public int GetSerialNumber(byte[]pOutputBuffer,  int nSizeOfOutputBuffer,int[] nBytesReturned)
    {
        String serial = GetSerialNumber();
        return StringToArray(serial, pOutputBuffer, nSizeOfOutputBuffer, nBytesReturned);
    }

    public int GetModelName(byte[] pOutputBuffer, int nSizeOfOutputBuffer, int[] nBytesReturned) {
        String name = GetModelName();
        return StringToArray(name, pOutputBuffer, nSizeOfOutputBuffer, nBytesReturned);
    }

    public int GetFirmwareVersion(byte[] pOutputBuffer, int nSizeOfOutputBuffer, int[] nBytesReturned) {
        String version = GetFirmwareVersion();
        return StringToArray(version, pOutputBuffer, nSizeOfOutputBuffer, nBytesReturned);
    }


}
