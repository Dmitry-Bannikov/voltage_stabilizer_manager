#include "mqtthandler.h"
#include <common_data.h>
#include <devices.h>


#define USE_ORTEA

#ifdef USE_ORTEA
const char *mqtt_broker = "ortea.ru";
String mqtt_clientId = "esp32_stab";
const char *mqtt_username = "device_stab";
const char *mqtt_password = "2#r]V\\r]+(Dw@WnAd5Kq";
const int mqtt_port = 8880;
#else
const char *mqtt_broker = "m6.wqtt.ru";
String mqtt_clientId = "esp32_stab";
const char *mqtt_username = "u_J94WNP";
const char *mqtt_password = "TvTRzLsh";
const int mqtt_port = 15164;
#endif
WiFiClient espClient;
PubSubClient mqttClient(espClient);



void MqttInit() {
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setBufferSize(500);
	mqtt_clientId = "stab_brd_" + String(Board_SN);
	for (uint8_t i = 0; i < 3; i++) {
		String sub_topic_sets = "stab_brd/getsets/fase_" + String((char)(65+i)) + "/";
		mqttClient.subscribe(sub_topic_sets.c_str());
	}
	MqttReconnect();
	mqttClient.subscribe("stab_brd/getsets/fase_N");
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
        if (!mqttClient.connect(mqtt_clientId.c_str(), mqtt_username, mqtt_password)) {
            Serial.print("  Mqtt failed, reason: ");
            Serial.println(mqttClient.state()); 
        }
    }
    tmr = millis();
}

void MqttPublishData() {
    static uint32_t tmr = 0;
    if (mqttRequest==0) return;
    for (uint8_t i = 0; i < board.size(); i++) {
        sendFaseMqttData(i, mqttRequest);
    }
    if (mqttRequest == 5) {
        String topic = "stab_brd/user/" + S(Board_SN);
        std::string data = User_getJson();
        mqttConnected = sendMqttJson(topic.c_str(), data.c_str());
    }
    if (mqttRequest == 6) {
        String topic = "stab_brd/devices/" + S(Board_SN);
        std::string data = Device_getJson();
        mqttConnected = sendMqttJson(topic.c_str(), data.c_str());
    }
	mqttRequest = 0;
    tmr = millis();
}

void onMqttMessage(char* topic, uint8_t* payload, size_t len) {
    String topicStr = String(topic);
    if (topicStr.indexOf("MqttRequest") != -1) {
        getMqttRequest((char*)payload);
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
    
    if (topicStr.indexOf("getsets") != -1){
        board[board_num].setJsonData(std::string((char*)payload));
    }
    mqttClient.flush();
}

void Mqtt_tick() {
    mqttClient.loop();
    MqttReconnect();
    MqttPublishData();
    createMqttRequest();
}

bool sendFaseMqttData(int8_t numBrd, int request) {
	if (request == 0) return true;
	String Lit = String(board[numBrd].getLiteral());
	String topic = "";
	std::string data = "";
    String time_S = ui.getSystemTime().encode();
    std::string time =  time_S.c_str();
	if (request == 1) {
		topic = "stab_brd/data/fase_" + Lit + "/" + S(Board_SN);
        board[numBrd].getJsonData(data, DATA_ACT, time);
        board[numBrd].Bdata.getMinMax();
	} else if (request == 2) {
		topic = "stab_brd/datamin/fase_" + Lit + "/" + S(Board_SN);
		board[numBrd].getJsonData(data, DATA_MIN, time);
        mqttConnected = sendMqttJson(topic.c_str(), data.c_str());
        topic = "stab_brd/datamax/fase_" + Lit + "/" + S(Board_SN);
		board[numBrd].getJsonData(data, DATA_MAX, time);
        mqttConnected = sendMqttJson(topic.c_str(), data.c_str());
	} else if (request == 3) {
		topic = "stab_brd/sets/fase_" + Lit + "/" + S(Board_SN);
		board[numBrd].getJsonData(data, DATA_MAX, time);
	} else if (request == 4 || board[numBrd].mainData.Events > 0) {
		topic = "stab_brd/alarms/fase_" + Lit + "/" + S(Board_SN);
		uint8_t alarm_code = 0;
		std::string alarm_text;
		alarm_code = board[numBrd].getNextActiveAlarm(alarm_text, board[numBrd].mainData.Events);
		data = "{\"Code\":\"" + std::to_string(alarm_code) + "\",";
        data += "\"Text\":\"" + alarm_text + "\"}\0";
	}
	mqttConnected = sendMqttJson(topic.c_str(), data.c_str());
    return mqttConnected;
}


bool sendMqttJson(const char* topic, const char* data) {
    size_t length = strlen(data)+1;
    size_t bytesWritten = 0;
    bool isStart = mqttClient.beginPublish(topic, length, false);
    while (bytesWritten < length) {
        size_t chunkSize = (length - bytesWritten) > 50 ? 50 : (length - bytesWritten) ;
        mqttClient.write((uint8_t*)(data + bytesWritten), chunkSize);
        bytesWritten += chunkSize;
    }
    Serial.printf("Sended via mqtt, request: %d\n", mqttRequest);
    Serial.println(topic);
    Serial.println(data);
    bool result = isStart && mqttClient.endPublish();
    return result;
}


void getMqttRequest(const char* json) {
	String Request = String(json);
	//if (Request.indexOf("MqttRequest") == -1) return;
	if (Request.indexOf("Data") != -1) mqttRequest = 1;
	else if (Request.indexOf("DataStat") != -1) mqttRequest = 2;
	else if (Request.indexOf("Settings") != -1) mqttRequest = 3;
	else if (Request.indexOf("Alarms") != -1) mqttRequest = 4;
}

void createMqttRequest() {
    if (mqttRequest) return;
    
    static uint32_t lastSecondTime = 0;
    static uint32_t lastMinuteTime = 0;
    static uint32_t lastTwoMinutesTime = 0;
    
    uint32_t currentTime = millis();

    
    if (millis() - lastSecondTime >= 1000) {
        mqttRequest = 1;
        lastSecondTime = millis();
    }
    
    if (millis() - lastMinuteTime >= 60000) {
        mqttRequest = 2;
        lastMinuteTime = millis();
    }

    if (millis() - lastTwoMinutesTime >= 120000) {
        mqttRequest = 3;
        lastTwoMinutesTime = millis();
    }
    
}














//-----------------------------------------------------------------------------------//