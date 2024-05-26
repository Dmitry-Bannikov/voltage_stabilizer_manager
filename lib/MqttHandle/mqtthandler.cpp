#include "mqtthandler.h"
#include <common_data.h>
#include <devices.h>
#include <timemodule.h>

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
int mqttRequest = 0;
int mqttNextRequest = 0;


void MqttInit() {
    Time_begin(parseTimeZone(User_Get(OWN_TIMEZONE)));
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
    if (board.size()) mqttReqResult = true;
    for (uint8_t i = 0; i < board.size(); i++) {
        if (!sendFaseMqttData(i, mqttRequest)) mqttReqResult = false;
    }
    if (mqttRequest == 5) {
        String topic = "stab_brd/user/" + S(Board_SN);
        std::string data = User_getJson();
        mqttReqResult = sendMqttJson(topic.c_str(), data.c_str());
    }
    if (mqttRequest == 6) {
        String topic = "stab_brd/devices/" + S(Board_SN);
        std::string data = Device_getJson();
        mqttReqResult = sendMqttJson(topic.c_str(), data.c_str());
    }
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
	if (request == 0 || request == 5 || request == 6) return true;
	String Lit = String(board[numBrd].getLiteral());
	String topic = "";
	std::string data = "";
    //std::string time_S =  "null";
    std::string time_S =  Time_getCurrent();
	if (request == 1) {
		topic = "stab_brd/data/fase_" + Lit + "/" + S(Board_SN);
        board[numBrd].getJsonData(data, DATA_ACT, time_S);
        board[numBrd].Bdata.getMinMax();
	} else if (request == 2) {
		topic = "stab_brd/datamin/fase_" + Lit + "/" + S(Board_SN);
		board[numBrd].getJsonData(data, DATA_MIN, time_S);
        mqttReqResult = sendMqttJson(topic.c_str(), data.c_str());
        topic = "stab_brd/datamax/fase_" + Lit + "/" + S(Board_SN);
		board[numBrd].getJsonData(data, DATA_MAX, time_S);
        mqttReqResult = sendMqttJson(topic.c_str(), data.c_str());
	} else if (request == 3) {
		topic = "stab_brd/sets/fase_" + Lit + "/" + S(Board_SN);
		board[numBrd].getJsonData(data, DATA_MAX, time_S);
	} else if (request == 4 || board[numBrd].mainData.Events > 0) {
		topic = "stab_brd/alarms/fase_" + Lit + "/" + S(Board_SN);
		uint8_t alarm_code = 0;
		std::string alarm_text;
		alarm_code = board[numBrd].getNextActiveAlarm(alarm_text, board[numBrd].mainData.Events);
		data = "{\"Code\":\"" + std::to_string(alarm_code) + "\",";
        data += "\"Text\":\"" + alarm_text + "\"}\0";
	}
	return sendMqttJson(topic.c_str(), data.c_str());
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
    
    static uint32_t lastSec = 0;
    static uint32_t lastMin = 0;
    static uint32_t last2Min = 0;

    if (mqttReqResult) {
        mqttRequest = 0;
        if (mqttNextRequest) {
            mqttRequest = mqttNextRequest;
            mqttNextRequest = 0;
        }
    }
    if (mqttRequest > 3) return;
    if (millis() - lastSec >= 1000 && mqttRequest != 2) {
        mqttRequest = 1;
        lastSec = millis();
    }
    
    if (millis() - lastMin >= 60000) {
        lastMin = millis();
        mqttRequest = 2;
    }

    if (millis() - last2Min >= 150000  && mqttRequest != 2) {
        mqttRequest = 3;
        last2Min = millis();
    }
    
}

void setMqttRequest(const int request) {
    mqttNextRequest = request;
}

int parseTimeZone(const String& tz) {
    if (tz.length() < 4 || tz.substring(0, 3) != "UTC") {
        // Некорректный формат строки
        return 0;
    }

    char sign = tz.charAt(3);
    if (sign != '+' && sign != '-') {
        // Некорректный знак
        return 0;
    }

    int hours = tz.substring(4).toInt();
    int offsetSeconds = hours * 3600;

    return (sign == '+') ? offsetSeconds : -offsetSeconds;
}










//-----------------------------------------------------------------------------------//