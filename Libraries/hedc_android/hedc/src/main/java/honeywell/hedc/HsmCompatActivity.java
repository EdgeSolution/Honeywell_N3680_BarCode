package honeywell.hedc;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.content.Context;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

public class HsmCompatActivity extends AppCompatActivity implements EngineCommunication.OnHedc {

    private static final String TAG = "honeywell"; // Debugging purposes
    protected EngineCommunication m_engine = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        m_engine = new EngineCommunication(this,this);
        m_engine.OnCreate();
    }

    @Override
    public void onStop(){
        super.onStop();
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onRestart() {
        super.onRestart();
    }

    @Override
    protected void onDestroy() {
        m_engine.OnDestroy();
        super.onDestroy();
    }

    public void showSoftKeyboard(View view) {
        if (view.requestFocus()) {
            InputMethodManager imm = (InputMethodManager)
                    getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.showSoftInput(view, InputMethodManager.SHOW_IMPLICIT);
        }
    }

    public void OnConnectionStateEvent(EngineCommunication.ConnectionState state) {
        Log.d(TAG, "No handler for OnConnectionStateEvent");
    }

    public void OnBarcodeData(String str) {
        Log.d(TAG, "No handler for OnBarcodeData");
    }

    public void OnBarcodeDataRaw(final byte[] data) {
        Log.d(TAG, "No handler for OnBarcodeDataRaw");
    }

    public void OnImageData(final Bitmap bitmap) {
        Log.d(TAG, "No handler for OnImageData");
    }


    public void OnProgress(int wparam, int lparam) {
        Log.d(TAG, "No handler for OnProgress");
    }

    // FIXME need cleanup
    public String MakeHumanReadable(byte raw[]) {
        String s = "";
        for (int i = 0; i < raw.length; i++) {
            int rec = raw[i];
            if ((rec < 0x20)&&(rec >= 0)) {
//              s += String.format("<%s>",AsciiTab[rec]);
                s += String.format("<0x%02X>",rec);
            }
            else if (rec >= 0x7F) {
                s += String.format("<0x%02X>",rec);
            }
            else if (rec<0) {
                s += String.format("<0x%02X>",-rec);
            }
            else {
                s += String.format("%c", rec);
            }
        }
        return s;
    }

    public String MakeHumanReadableCmdResponse(byte raw[]) {
        String s, s1;
        s = "";
        for (int i = 0; i < raw.length; i++) {
            if (raw[i] == 0x06) {
                s1 = String.format("[ACK]");
            } else if (raw[i] == 0x05) {
                s1 = String.format("[NACK]");
            } else if (raw[i] == 0x15) {
                s1 = String.format("[ENQ]");
            } else {
                s1 = String.format("%c", raw[i]);
            }
            s += s1;
        }
        return s;
    }

    // Public: Creates a string from portions of byte array
    public String ComposeString(String prefix, final byte[] bytes, int start, int end) {
        String s=prefix;
        int len = end-start;
        if(len>0) {
            s += new String(bytes, start, len - 1);
        }
        return s;
    }

    String GetErrorText(int error)
    {
        String RetVal = getString(R.string.ERROR_UNKNOWN);
        switch(error)
        {
            case Constants.ERROR_SUCCESS:
                RetVal = getString(R.string.ERROR_SUCCESS);
                break;
            case Constants.ERROR_COMMUNICATION:
                RetVal = getString(R.string.ERROR_COMMUNICATION);
                break;
            case Constants.ERROR_PORT_NOT_OPEN:
                RetVal = getString(R.string.ERROR_PORT_NOT_OPEN);
                break;
            case Constants.ERROR_CANCELED_BY_REMOTE:
                RetVal = getString(R.string.ERROR_CANCELED_BY_REMOTE);
                break;
            case Constants.ERROR_NO_SYNC:
                RetVal = getString(R.string.ERROR_NO_SYNC);
                break;
            case Constants.ERROR_XMIT_ERROR:
                RetVal = getString(R.string.ERROR_XMIT_ERROR);
                break;
            case Constants.ERROR_NO_FW_FILE:
                RetVal = getString(R.string.ERROR_NO_FW_FILE);
                break;
            case Constants.ERROR_PARSE_FILE:
                RetVal = getString(R.string.ERROR_PARSE_FILE);
                break;
            case Constants.ERROR_FILE_TO_BIG:
                RetVal = getString(R.string.ERROR_FILE_TO_BIG);
                break;
            case Constants.ERROR_FLASH_FAILED:
                RetVal = getString(R.string.ERROR_FLASH_FAILED);
                break;
            case Constants.ERROR_FLASH_UNSURE:
                RetVal = getString(R.string.ERROR_FLASH_UNSURE);
                break;
            case Constants.ERROR_FLASH_NO_RESPOND:
                RetVal = getString(R.string.ERROR_FLASH_NO_RESPOND);
                break;
            case Constants.ERROR_FILE_NOT_FOUND:
                RetVal = getString(R.string.ERROR_FILE_NOT_FOUND);
                break;
            case Constants.ERROR_READ_FAULT:
                RetVal = getString(R.string.ERROR_READ_FAULT);
                break;
            case Constants.ERROR_NOT_ENOUGH_MEMORY:
                RetVal = getString(R.string.ERROR_NOT_ENOUGH_MEMORY);
                break;
            case Constants.ERROR_INVALID_PARAMETER:
                RetVal = getString(R.string.ERROR_INVALID_PARAMETER);
                break;
            case Constants.ERROR_NOT_SUPPORTED:
                RetVal = getString(R.string.ERROR_NOT_SUPPORTED);
                break;
            case Constants.ERROR_AUTOSELECT_FAILED:
                RetVal = getString(R.string.ERROR_AUTOSELECT_FAILED);
                break;
            case Constants.ERROR_CMD_UNKNOWN:
                RetVal = getString(R.string.ERROR_CMD_UNKNOWN);
                break;
            case Constants.ERROR_CMD_FAILED:
                RetVal = getString(R.string.ERROR_CMD_FAILED);
                break;
            case Constants.ERROR_FRAMEWORK_FAILED:
                RetVal = getString(R.string.ERROR_FRAMEWORK_FAILED);
                break;
            case Constants.ERROR_GETMODUL_FAILED:
                RetVal = getString(R.string.ERROR_GETMODUL_FAILED);
                break;
            case Constants.ERROR_CANCELED_BY_USER:
                RetVal = getString(R.string.ERROR_CANCELED_BY_USER);
                break;
            case Constants.ERROR_INVALID_INPUT_BUFFER:
            case Constants.INVALID_INPUT_BUFFER:
			    RetVal ="Invalid Input buffer";
                break;
            case Constants.ERROR_OUTPUT_BUFFER_TOO_SMALL:
            case Constants.OUTPUT_BUFFER_TOO_SMALL:
			    RetVal ="Invalid Output buffer";
                break;
            case Constants.OPERATION_CANCELLED:
			    RetVal ="Canceled";
                break;
            case Constants.ERROR_REGISTRY:
            case Constants.REGISTRY_ERROR:
			    RetVal ="Fatal Error: Registry error";
                break;
            case Constants.ERROR_INTERNAL:
            case Constants.INTERNAL_ERROR:
			    RetVal ="Fatal Error: LIB internal error";
                break;
        }
        return RetVal;
    }
}
