#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <data.h>
//#include <AsyncMqttClient.h>
#include <PubSubClient.h>



#define USE_ORTEA

#ifdef USE_ORTEA
const char *mqtt_broker = "ortea.ru";//185.64.76.226
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
bool createFaseMqttData(char fase);
 


void MqttInit() {
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setCallback(onMqttMessage);
    mqttClient.setBufferSize(500);
    MqttReconnect();
    String esp_mac = WiFi.macAddress();
    String topicSets_A = "stab_brd/getsets/fase_A/"+esp_mac;
    String topicSets_B = "stab_brd/getsets/fase_B/"+esp_mac;
    String topicSets_C = "stab_brd/getsets/fase_C/"+esp_mac;
    String topicSets_A_outS = "stab_brd/outsignal/fase_A/"+esp_mac;
    String topicSets_B_outS = "stab_brd/outsignal/fase_B/"+esp_mac;
    String topicSets_C_outS = "stab_brd/outsignal/fase_C/"+esp_mac;
    mqttClient.subscribe(topicSets_A.c_str());
    mqttClient.subscribe(topicSets_B.c_str());
    mqttClient.subscribe(topicSets_C.c_str());
    mqttClient.subscribe(topicSets_A_outS.c_str());
    mqttClient.subscribe(topicSets_B_outS.c_str());
    mqttClient.subscribe(topicSets_C_outS.c_str());
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
            Serial.print("\n Mqtt failed, reason: ");
            Serial.println(mqttClient.state());  
        }
    } else {
        mqttConnected = true;
    }
    tmr = millis();
}

void MqttPublishData() {
    static uint32_t tmr = 0;
    if (millis() < tmr + 1000) return;
    createFaseMqttData('A');
    createFaseMqttData('B');
    createFaseMqttData('C');
    tmr = millis();
}

void onMqttMessage(char* topic, uint8_t* payload, size_t len) {
    String topicStr = String(topic);
    char* payloadChar = (char*)payload;
    char* endBracket = strchr(payloadChar, '}');
    if (endBracket != NULL) {
        *(endBracket + 1) = '\0';
    }
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
    if (topicStr.indexOf("getsets") != -1) {
        board[board_num].getJsonData(payloadChar, 0);
    } else if (topicStr.indexOf("outsignal") != -1) {
        int value = -1;
        sscanf(payloadChar, "%d", &value);
        if (value != -1) {
            board[board_num].addSets.Switches[SW_OUTSIGN] = value;
            board[board_num].sendCommand();
        }
        Serial.println(value);
    }
    //добавить значение 2 (если 2, то каждые 10 минут)
    //stab_brd/outsignal/fase_A/
}

void Mqtt_tick() {
    mqttClient.loop();
    MqttReconnect();
    MqttPublishData();
}

bool createFaseMqttData(char fase) {
    /*
    topic: "stab_brd/data/fase_A/mac"
    json : {"Mode":"Data","Fase":"A","Uin":"000","Uout":"000","I":"0.0","P":"0.0","Uin_avg":"000",
    "Uout_avg":"000","I_avg":"0.0","P_avg":"0.0","Uin_max":"000","Uout_max":"000","I_max":"0.0","P_max":"0.0","work_h":"000"}
    or...
    {"Mode":"Sets","Fase":"A","Uout_minoff":"170","Uout_maxoff":"250","Accuracy":"2","Uout_target":"220","Uin_tune":"-2",
    "Uout_tune":"-2","t_5":"60","SN_1":"123456789","SN_2":"123456","M_type":"2","Time_on":"0.5","Time_off":"2.0","Rst_max":"0","Save":"0",
    "Transit":"0","Password":"0","Outsignal":"0"}
    or...
    {"Mode\":"Alarms","Fase":"A","Нет":"0","Блок мотора":"1","Тревога 1":"0","Тревога 2":"0",
	"Нет питания":"0","Недо-напряжение":"0","Пере-напряжение":"0","Макс напряжение":"0","Мин напряжение":"0",
	"Транзит":"0","Перегрузка":"0","Внешний сигнал":"1","Выход откл":"0"}
    
    */
    static uint8_t cnt = 0;
    int8_t board_number = -1;
    
    for (uint8_t i = 0; i < board.size(); i++) {
        if(board[i].getLiteral() == fase) { //ищем номер платы с такой буквой
            board_number = i;
            break;
        }
    }
    if (board_number == -1) return false;
    String topic;
    String data;
    if (cnt < 30) {
        topic = "stab_brd/data/fase_";
        topic += String(fase) + "/";
        topic += WiFi.macAddress();
        board[board_number].createJsonData(data, 0);
        mqttClient.publish(topic.c_str(), data.c_str());
        cnt++;
    } else {
        Serial.println("Sending sets...");
        topic = "stab_brd/sets/fase_";
        topic += String(fase) + "/";
        topic += WiFi.macAddress();
        board[board_number].createJsonData(data, 1);
        mqttClient.publish(topic.c_str(), data.c_str());
        cnt = 0;
    }
    if (board[board_number].mainData.Events != 0) {
        topic = "stab_brd/alarms/fase_";
        topic += String(fase) + "/";
        topic += WiFi.macAddress();
        board[board_number].createJsonData(data, 2);
        mqttClient.publish(topic.c_str(), data.c_str());
    }
    
    return true;
}



//-----------------------------------------------------------------------------------//