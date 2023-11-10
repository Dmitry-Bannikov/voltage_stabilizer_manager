#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <data.h>
//#include <AsyncMqttClient.h>
#include <PubSubClient.h>

/*Структура пакета передачи/приема:
header1
header2
bytesCnt
modeRX/ modeTX
addrH
addrL
data[]

*/
#define header1 0x5A
#define header2 0xA5
#define byteCnt 0x09
#define footer1 0x5B
#define footer2 0xB5
#define modeTX  0x82
#define modeRX  0x83
#define inV_addr    ((uint16_t)0x5000)
#define outV_addr   ((uint16_t)0x5010)
#define outC_addr   ((uint16_t)0x5020)
#define outP_addr   ((uint16_t)0x5030)


const char *mqtt_broker = "m6.wqtt.ru";//185.64.76.226
const char *topicToServer = "stab/toserver";
const char *topicToStab = "stab/tostab";
//const char *mqtt_clientId = "mqttx_6a007db2";
const char *mqtt_username = "u_J94WNP";
const char *mqtt_password = "TvTRzLsh";
const int mqtt_port = 15164;


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
    mqttClient.subscribe(topicToServer);
    mqttClient.subscribe(topicToStab);
}

void MqttReconnect() {
    static uint32_t tmr = 0;
    static uint32_t period = 500;
    if (millis() - tmr <= period) return;
    if (!mqttClient.connected())
    {
        period = 60000;
        Serial.print("Attempting MQTT connection.");
        //String clientId = "ESP32-" + WiFi.macAddress();
        String clientId = "mqttx_6a007db2";
        Serial.print("..");
        if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)){
            Serial.println("connected");
            return;
        } else {
            Serial.print("failed, rc=");
            Serial.println(mqttClient.state());
            
        }
    } else {
        Serial.print("Mqtt server connected...");
        Serial.println(mqttClient.state());
    }
    tmr = millis();
}

void MqttPublishData() {
    static uint32_t tmr = 0;
    if (millis() < tmr + 2000) return;
    
    uint8_t inV[30] = {header1,header2,byteCnt, modeTX}; 
    uint8_t outV[30] = {header1,header2,byteCnt, modeTX};
    uint8_t outC[30] = {header1,header2,byteCnt, modeTX};
    uint8_t outP[30] = {header1,header2,byteCnt, modeTX};

    Buffer_addNewValue(inV_addr, inV, 30, 1);
    for (uint8_t i = 0; i < board.size(); i++) Buffer_addNewValue(board[i].mainData.inputVoltage, inV, 30, 0);

    Buffer_addNewValue(outV_addr, outV, 30, 1);
    for (uint8_t i = 0; i < board.size(); i++) Buffer_addNewValue(board[i].mainData.outputVoltage, outV, 30, 0);

    Buffer_addNewValue(outC_addr, outC, 30, 1);
    for (uint8_t i = 0; i < board.size(); i++) Buffer_addNewValue(board[i].mainData.outputCurrent, outC, 30, 0);

    Buffer_addNewValue(outP_addr, outP, 30, 1);
    for (uint8_t i = 0; i < board.size(); i++) Buffer_addNewValue(board[i].mainData.outputPower, outP, 30, 0);

    mqttClient.publish(topicToServer, inV, sizeof(inV));
    mqttClient.publish(topicToServer, outV, sizeof(inV));
    mqttClient.publish(topicToServer, outC, sizeof(inV));
    mqttClient.publish(topicToServer, outP, sizeof(inV));
    tmr = millis();
}

void onMqttMessage(char* topic, uint8_t* payload, size_t len) {
    if (strcmp(topic, topicToStab) != 0) {
        Serial.println("Не та тема!");
        return;
    }
    Serial.println("\n Данные приняты.");
    uint8_t buffer[250];
    for (uint8_t i = 0; i < len || i < sizeof(buffer); i++) {
        buffer[i] = payload[i];
    }
    if (buffer[0] != 0x5A) return;
    
    uint16_t addr = *(uint16_t*)(buffer+4);
    int16_t value = *(int16_t*)(buffer+7);
    if (addr = 0x6000) {
        Serial.printf("\nАктивная плата: %d", value);
        activeBoard = value;
    } else if (addr == 0x6001) {
        Serial.printf("\nЦелевое напряжение: %d", value);
        //board[activeBoard].mainSets.targetVoltage = value;
    }
}

void Mqtt_tick() {
    mqttClient.loop();
    MqttReconnect();
    MqttPublishData();
}





//-----------------------------------------------------------------------------------//