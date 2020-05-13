package com.adv.scanbarcode;

import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.adv.localmqtt.MQTTWrapper;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.reflect.Method;

import honeywell.hedc.EngineCommunication;
import honeywell.hedc_usb_com.HEDCUsbCom;

public class MainActivity extends Activity implements HEDCUsbCom.OnConnectionStateListener, HEDCUsbCom.OnBarcodeListener, HEDCUsbCom.OnImageListener, HEDCUsbCom.OnDownloadListener {
    private static final String TAG = "ScanBarCode";
    private TextView textView;
    private MQTTWrapper mqttWrapper = null;
    private String mqttClientId = "com.adv.scanbarcode";
    private MqttMessageReceiver mqttMessageReceiver;

    HEDCUsbCom m_engine;

    private String[] AsciiTab = {
            "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",	"BS",  "HT",  "LF",  "VT",
            "FF",  "CR",  "SO",  "SI",	"DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
            "CAN", "EM",  "SUB", "ESC", "FS",  "GS",  "RS",  "US",	"SP",  "DEL",
    };

    final Context mContext = this;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        textView = findViewById(R.id.textview);
        textView.setMovementMethod(ScrollingMovementMethod.getInstance());
        Log.d(TAG,"onCreate");

        m_engine = new HEDCUsbCom(this,this,this,this, this);
        m_engine.OnCreate();

        textView.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View view) {
                textView.setText("");
                Toast.makeText(mContext,"Clear textview",Toast.LENGTH_LONG).show();
                return true;
            }
        });

        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    try {
                        //String command = "ps |grep mosquitto";
                        String command = "ps";
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                            String mosquittoIsRunning = getSystemStringProperties(MainActivity.this,"adv.mosquittoIsRunning","false");
                            if (mosquittoIsRunning != null && !mosquittoIsRunning.isEmpty() && mosquittoIsRunning.equals("true")) {
                                break;
                            }
                            Log.d(TAG, "isServiceRunning ...");
                        }else {
                            if (commandIsRuning(command)) {
                                break;
                            }
                            Log.d(TAG, "ps command ...");
                        }
                        Thread.sleep(3000);
                    } catch (InterruptedException | IOException e) {
                        Log.d(TAG, "Exception Error ...");
                        e.printStackTrace();
                        return;
                    }
                }
                Log.d(TAG, "connectMqttBroker ...");
                connectMqttBroker();
            }
        }).start();

    }

    @Override
    protected void onDestroy() {
        if(m_engine!=null){
            m_engine.OnDestroy();
        }
        if(mqttWrapper != null) {
            mqttWrapper.destroy();
        }
        Log.d(TAG,"onDestroy");
        super.onDestroy();
    }

    @Override
    public void OnConnectionStateEvent(EngineCommunication.ConnectionState state) {
        if (state == HEDCUsbCom.ConnectionState.Connected) {
            Log.i(TAG,"Device connected");
            Toast.makeText(mContext,"Device connected",Toast.LENGTH_LONG).show();
        }else{
            Log.i(TAG,"Device not connected");
            Toast.makeText(mContext,"Device not connected",Toast.LENGTH_LONG).show();
        }
    }

    @Override
    public void OnBarcodeData(byte[] data, int length) {
        String barcodeData = ConvertToString(data, length);
        Log.d(TAG, "barcodeData: " + barcodeData);
        //textView.append(barcodeData + "\n");
        refreshTextView(barcodeData+"\n");
        JSONObject jsonObj = new JSONObject();
        try {
            jsonObj.put("barcodeData", barcodeData);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        if(mqttMessageReceiver != null) {
            mqttMessageReceiver.autoRepBuilder(mqttClientId, "report_data", jsonObj.toString());
        }else{
            Toast.makeText(mContext,"Mqtt is not connected therefore does not report data to the web!",Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void OnImageData(byte[] RawImage, int sizeImage) {

    }

    @Override
    public void OnDownloadData(int progress, short error) {

    }

    public String ConvertToString(byte[] data, int length)
    {
        String s;
        StringBuilder s_final = new StringBuilder();
        for (int i = 0; i < length; i++) {
            if ((data[i]>=0)&&(data[i] < 0x20 ))
            {
                s = String.format("<%s>",AsciiTab[data[i]]);
            }
            else if (data[i] >= 0x7F)
            {
                s = String.format("<0x%02X>",data[i]);
            }
            else {
                s = String.format("%c", data[i]&0xFF);
            }
            s_final.append(s);
        }

        return s_final.toString();
    }

    private void connectMqttBroker() {
        Log.d(TAG, "--> onCreate");
        mqttWrapper = new MQTTWrapper(mqttClientId);
        mqttMessageReceiver = new MqttMessageReceiver(mqttWrapper,mqttClientId);
        boolean ret = mqttWrapper.connect(mqttMessageReceiver);
        if (ret) {
            Log.d(TAG, "--> mqtt connected");
        } else {
            Log.e(TAG, "--> mqtt no connect, return");
        }
    }

    private boolean commandIsRuning(String command) throws IOException {
        Runtime runtime = Runtime.getRuntime();
        Process proc = runtime.exec(command);
        InputStream inputstream = proc.getInputStream();
        InputStreamReader inputstreamreader = new InputStreamReader(inputstream);
        BufferedReader bufferedreader = new BufferedReader(inputstreamreader);
        String line = "";
        StringBuilder sb = new StringBuilder(line);
        while ((line = bufferedreader.readLine()) != null) {
            sb.append(line);
            sb.append('\n');
        }
        try {
            if (proc.waitFor() != 0) {
                Log.e(TAG,"Command exit value = " + proc.exitValue());
                return false;
            }
            return sb.toString().contains("mosquitto");
        }
        catch (InterruptedException e) {
            e.printStackTrace();
            return false;
        }
    }

    public static String getSystemStringProperties(Context context, String key, String def) throws IllegalArgumentException {
        String ret = def;
        try {
            ClassLoader cl = context.getClassLoader();
            @SuppressWarnings("rawtypes")
            Class SystemProperties = cl.loadClass("android.os.SystemProperties");
            @SuppressWarnings("rawtypes")
            Class[] paramTypes = new Class[2];
            paramTypes[0] = String.class;
            paramTypes[1] = String.class;
            Method get = SystemProperties.getMethod("get", paramTypes);
            Object[] params = new Object[2];
            params[0] = new String(key);
            params[1] = new String(def);
            ret = (String) get.invoke(SystemProperties, params);
        } catch (IllegalArgumentException iAE) {
            throw iAE;
        } catch (Exception e) {
            ret = def;
        }
        return ret;
    }

    void refreshTextView(String msg){
        textView.append(msg);
        int offset = textView.getLineCount()*textView.getLineHeight();
        if(offset>textView.getHeight()){
            textView.scrollTo(0,offset-textView.getHeight());
        }
    }
}
