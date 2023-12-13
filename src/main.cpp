/*
C:\Users\Viktoriya\.platformio\packages\framework-arduinoespressif32\libraries\WebServer\src 
find WebServer.cpp at line 649 and comment log_e
*/


#include <Arduino.h>
#include <myportal.h>
#include <service.h>


//#define INTER_TEST
void setup() {
	#ifndef INTER_TEST
	connectionInit();
	WiFi_Init();
	MqttInit();
	memoryInit();
	portalInit();
	#else
	Serial.begin(115200);
	pinMode(21, INPUT);
	pinMode(22, INPUT);
	#endif
}

void loop() {
	#ifndef INTER_TEST
	boardTick();
	portalTick();
	WiFi_tick();
	Mqtt_tick();
	#else
	if (!digitalRead(21) || !digitalRead(22)) {
		Serial.println("\n INTERFERENCE");
		delay(500);
	}
	#endif
}
