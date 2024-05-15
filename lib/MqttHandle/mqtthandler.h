#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>


#define DATA_ACT 0
#define DATA_MIN 1
#define DATA_MAX 2
#define SETTINGS 3



void MqttInit();
void MqttPublishData();
void onMqttMessage(char* topic, uint8_t* payload, size_t len);
void MqttReconnect();
void Mqtt_tick();
bool sendFaseMqttData(int8_t numBrd, int request);
bool sendMqttJson(const char* topic, const char* data);
void getMqttRequest(const char* json);