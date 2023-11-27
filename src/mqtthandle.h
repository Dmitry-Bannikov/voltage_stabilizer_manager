#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <data.h>
//#include <AsyncMqttClient.h>
#include <PubSubClient.h>



#define USE_ORTEA

#ifdef USE_ORTEA
const char *mqtt_broker = "ortea.ru";//185.64.76.226
const char *topicToServer = "stab/toserver";
const char *topicToStab = "stab/tostab";
const char *mqtt_clientId = "esp32mqtt_client";
const char *mqtt_username = "device_stab";
const char *mqtt_password = "2#r]V\\r]+(Dw@WnAd5Kq";
const int mqtt_port = 8880;
#else
const char *mqtt_broker = "m6.wqtt.ru";
const char *topicToServer = "stab/toserver";
const char *topicToStab = "stab/tostab";
const char *mqtt_clientId = "esp32_stab_manager";
const char *mqtt_username = "u_J94WNP";
const char *mqtt_password = "TvTRzLsh";
const int mqtt_port = 15164;
#endif
WiFiClient espClient;
PubSubClient mqttClient(espClient);



void MqttInit();
void MqttPublishData();
void onMqttMessage(char* topic, uint8_t* payload, size_t len);
void MqttReconnect();
void Mqtt_tick();

 


void MqttInit() {
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setCallback(onMqttMessage);
    MqttReconnect();
    //mqttClient.subscribe(topicToServer);
    //mqttClient.subscribe(topicToStab);
    String topic1 = "stab/tostab/alarm1";
    String topic2 = "stab/toserver/alarm1";
    mqttClient.subscribe(topic1.c_str());
    mqttClient.subscribe(topic2.c_str());
}

void MqttReconnect() {
    static uint32_t tmr = 0;
    static uint32_t period = 500;
    if (millis() - tmr <= period) return;
    if (!mqttClient.connected())
    {
        mqttConnected = false;
        if (mqttClient.connect(mqtt_clientId, mqtt_username, mqtt_password)){
            period = 10000;
            mqttConnected = true;
        } else {
            period = 60000;
            mqttConnected = false;
            //Serial.print("\n Mqtt failed, reason: ");
            //Serial.println(mqttClient.state());  
        }
    } else {
        mqttConnected = true;
    }
    tmr = millis();
}

void MqttPublishData() {
    static uint32_t tmr = 0;
    if (millis() < tmr + 1000) return;
    String inV = String(board[activeBoard].mainData.inputVoltage);
    mqttClient.publish("stab/toserver/uin", inV.c_str());

    String outV = String(board[activeBoard].mainData.outputVoltage);
    mqttClient.publish("stab/toserver/uout", outV.c_str());

    String outC = String(board[activeBoard].mainData.outputCurrent, 1);
    mqttClient.publish("stab/toserver/cout", outC.c_str());

    String outP = String((board[activeBoard].mainData.outputPower/1000), 1);
    mqttClient.publish("stab/toserver/pout", outP.c_str());

    String alarm1 = String(board[activeBoard].addSets.Switches[SW_ALARM]);
    mqttClient.publish("stab/toserver/alarm1", alarm1.c_str());
 
    tmr = millis();
}

void onMqttMessage(char* topic, uint8_t* payload, size_t len) {
    String topicStr = String(topic);
    if (topicStr == "stab/tostab/alarm1") {
        //Serial.println("\n Алярм");
        if (String((char*)payload) == "1") {
            board[activeBoard].addSets.Switches[SW_ALARM] = 1;
            board[activeBoard].sendCommand();
        } else {
            board[activeBoard].addSets.Switches[SW_ALARM] = 0;
            board[activeBoard].sendCommand();
        } 
    }





    /*
    uint8_t buffer[250];
    for (uint8_t i = 0; i < len || i < sizeof(buffer); i++) {
        buffer[i] = payload[i];
    }
    if (buffer[0] != 0x5A) return;
    uint16_t addr = 0; int16_t value = 0; int32_t value4B = 0;
    if (len >= 6) {
        addr = reverseBytes(*(uint16_t*)(buffer+4));
        Serial.printf("\nAddress: %d", addr);
    }
    if (len >= 9) {
        value = reverseBytes(*(int16_t*)(buffer+7));
        Serial.printf("\nData: %d", value);
    }
    if (len >= 11) {
        value4B = reverseBytes(*(int32_t*)(buffer+9));
        Serial.printf("\nData: %d", value);
    }
    if (addr == 0x6000) {
        Serial.printf("\nАктивная плата: %d", value);
        //activeBoard = value;
    } else if (addr == 0x6001) {
        Serial.printf("\nЦелевое напряжение: %d", value);
        //board[activeBoard].mainSets.targetVoltage = value;
    }
    */
}

void Mqtt_tick() {
    mqttClient.loop();
    MqttReconnect();
    MqttPublishData();
}

void onMqttReceive(char* topic, char* payload) {}



//-----------------------------------------------------------------------------------//