package com.adv.scanbarcode;

import android.util.Log;

import com.adv.localmqtt.MQTTWrapper;
import com.adv.localmqtt.MessageID;
import com.adv.localmqtt.MqttV3MessageReceiver;
import com.adv.localmqtt.Payload;



public class MqttMessageReceiver extends MqttV3MessageReceiver {
    private final String TAG = "MqttMessageReceiver";

    MqttMessageReceiver(MQTTWrapper mqttWrapper, String mqttClientId) {
        super(mqttWrapper, mqttClientId);
    }

    @Override
    public void handleMessage(String topic, String message) {
        Log.d(TAG,"topic:" + topic + " message:"+message);
    }

    public void autoRepBuilder(String appName, String funcId, String content) {
        Log.d(TAG,"appName:" + appName + " funcId:"+funcId + " content:"  +content);
        String autoRepTopic = genAutoRepTopic();
        Payload response = new Payload(MessageID.get(), appName, funcId, 1, 2, content);
        String pubContent = response.genContent();
        getMQTTWrapper().publish(autoRepTopic, pubContent);
    }
}
