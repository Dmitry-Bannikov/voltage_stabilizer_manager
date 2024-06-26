#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <data.h>
#include <PubSubClient.h>



#define USE_ORTEA

#ifdef USE_ORTEA
const char *mqtt_broker = "ortea.ru";
const char *mqtt_clientId = "esp32_stabbrd";
const char *mqtt_username = "device_stab";
const char *mqtt_password = "2#r]V\\r]+(Dw@WnAd5Kq";
const int mqtt_port = 8880;
#else
const char *mqtt_broker = "m6.wqtt.ru";
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
bool createFaseMqttData(int8_t numBrd);
bool sendMqttJson(const char* topic, const char* data);


void MqttInit() {
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setBufferSize(500);
    MqttReconnect();
    String esp_mac = WiFi.macAddress();
    String topicSets_A = "stab_brd/getsets/fase_A/"+esp_mac;
    String topicSets_B = "stab_brd/getsets/fase_B/"+esp_mac;
    String topicSets_C = "stab_brd/getsets/fase_C/"+esp_mac;
    mqttClient.subscribe(topicSets_A.c_str());
    mqttClient.subscribe(topicSets_B.c_str());
    mqttClient.subscribe(topicSets_C.c_str());
    mqttClient.setCallback(onMqttMessage);
}

void MqttReconnect() {
    static uint32_t tmr = 0;
    static uint32_t period = 500;
    if (millis() - tmr <= period) return;
    bool mqttConn = mqttClient.connected();
    if (mqttConn) {
        period = 60000;
    } else {
        period = 5000;
        if (!mqttClient.connect(mqtt_clientId, mqtt_username, mqtt_password)) {
            Serial.print("  Mqtt failed, reason: ");
            Serial.println(mqttClient.state()); 
        }
    }
    tmr = millis();


}

void MqttPublishData() {
    static uint32_t tmr = 0;
    if (millis() < tmr + 1000) return;
    for (uint8_t i = 0; i < board.size(); i++) {
        createFaseMqttData(i);
    }
    tmr = millis();
}

void onMqttMessage(char* topic, uint8_t* payload, size_t len) {
    String topicStr = String(topic);
    payload[len] = '\0';
    //std::string payloadStr = std::string((char*)payload);
    int index = topicStr.indexOf("fase_") + 5;
    if (index == -1) return;
    char fase = topicStr.charAt(index);
    int8_t board_num = -1;
    for (int i = 0; i < board.size(); i++) {
        if (board[i].mainSets.Liter == fase) {
            board_num = i;
            break;
        }
    }
    if (board_num == -1) return;
    
    if (topicStr.indexOf("getsets") != -1){
        board[board_num].setJsonData(std::string((char*)payload));
    }
    mqttClient.flush();
}

void Mqtt_tick() {
    mqttClient.loop();
    MqttReconnect();
    MqttPublishData();
}

bool createFaseMqttData(int8_t numBrd) {
    static uint8_t sentNullAlarm[3] = {0,0,0};
    static uint8_t cnt = 0;
    String Lit = String(board[numBrd].getLiteral());
    String topic;
    std::string data;
    if (cnt < 120) {
        topic = "stab_brd/data/fase_" + Lit + "/" + WiFi.macAddress();
        board[numBrd].getJsonData(data, 0);
        board[numBrd].Bdata.getMinMax();
        cnt++;
    } else {
        topic = "stab_brd/sets/fase_" + Lit + "/" + WiFi.macAddress();
        board[numBrd].getJsonData(data, 4);
        cnt = 0;
    }
    mqttConnected = sendMqttJson(topic.c_str(), data.c_str());
    if (cnt == 60) {
        String topicMin = "stab_brd/datamin/fase_" + Lit + "/" + WiFi.macAddress();
        std::string dataMin; board[numBrd].getJsonData(dataMin, 1);
        String topicMax = "stab_brd/datamax/fase_" + Lit + "/" + WiFi.macAddress();
        std::string dataMax; board[numBrd].getJsonData(dataMax, 2);
        mqttConnected = sendMqttJson(topicMin.c_str(), dataMin.c_str());
        mqttConnected = sendMqttJson(topicMax.c_str(), dataMax.c_str());
        board[numBrd].Bdata.getMinMax(true);
    }
    if (board[numBrd].mainData.Events != 0) {
        std::string alarm_text;
        uint8_t alarm_code = 0;
        String data;
        String topic = "stab_brd/alarms/fase_" + Lit + "/" + WiFi.macAddress();
        alarm_code = board[numBrd].getNextActiveAlarm(alarm_text, board[numBrd].mainData.Events);
        
        data = "{\"Code\":\"" + String(alarm_code) + "\",";
        data += "\"Text\":\"" + String(alarm_text.c_str()) + "\"}\0";
        mqttClient.publish(topic.c_str(), data.c_str());
        sentNullAlarm[numBrd] = false;
    } else if (!sentNullAlarm[numBrd]) {
        String topic = "stab_brd/alarms/fase_" + Lit + "/" + WiFi.macAddress();
        String data = "{\"Code\":\"0\",\"Text\":\"\"}\0";
        mqttClient.publish(topic.c_str(), data.c_str());
        sentNullAlarm[numBrd] = true;
    }
    return true;
}


bool sendMqttJson(const char* topic, const char* data) {
    size_t length = strlen(data)+1;
    size_t bytesWritten = 0;
    const char* topicPtr = topic;
    const char* dataPtr = data;
    bool isStart = mqttClient.beginPublish(topicPtr, length, false);
    while (bytesWritten < length) {
        size_t chunkSize = (length - bytesWritten) > 50 ? 50 : (length - bytesWritten) ;
        mqttClient.write((uint8_t*)(dataPtr + bytesWritten), chunkSize);
        bytesWritten += chunkSize;
    }
    return isStart && mqttClient.endPublish();
}



//-----------------------------------------------------------------------------------//