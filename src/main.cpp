/*
C:\Users\Viktoriya\.platformio\packages\framework-arduinoespressif32\libraries\WebServer\src 
find WebServer.cpp at line 649 and comment log_e
*/


#include <Arduino.h>
#include <myportal.h>
#include <service.h>
#ifdef RELEASE
#include <esp_task_wdt.h>
#endif

void setup() {
	connectionInit();
	Serial.println(ESP.getFreeHeap());
	WiFi_Init();
	MqttInit();
	memoryInit();
	portalInit();
	#ifdef RELEASE
	//esp_task_wdt_init(20, true);
	//esp_task_wdt_add(NULL);
	#endif
}

void loop() {
	boardTick();
	//Serial.println("board tick");
	portalTick();
	//Serial.println("portal tick");
	WiFi_tick();
	//Serial.println("wifi tick");
	Mqtt_tick();
	#ifdef RELEASE
	//esp_task_wdt_reset();  
	#endif
}
