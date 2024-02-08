/*
C:\Users\Viktoriya\.platformio\packages\framework-arduinoespressif32\libraries\WebServer\src 
find WebServer.cpp at line 649 and comment log_e
*/


#include <Arduino.h>
#include <myportal.h>
#include <service.h>


void setup() {
	connectionInit();
	WiFi_Init();
	MqttInit();
	memoryInit();
	portalInit();
	Serial.println(WiFi.macAddress());
}

void loop() {
	boardTick();
	portalTick();
	WiFi_tick();
	Mqtt_tick();
	
}

/*
**01 04 addrH addrL bcH bcL CRC CRC
01 04 00 00 00 02 71 cb     U1
01 04 00 02 00 02 d0 0b     U2
01 04 00 04 00 02 30 0a     U3
01 04 00 06 00 02 91 ca     I1
01 04 00 08 00 02 f0 09     I2
01 04 00 0A 00 02 51 c9     I3
01 04 00 24 00 02 31 c0     Fi1
01 04 00 26 00 02 90 00     Fi2
01 04 00 28 00 02 f1 c3     Fi3
*/

//сделать поддержку пирометра?
//добавить почту пароль мак
//добавить время алярмов
//изучить еспшку с симкой