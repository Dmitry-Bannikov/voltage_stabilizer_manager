/*
C:\Users\Viktoriya\.platformio\packages\framework-arduinoespressif32\libraries\WebServer\src 
find WebServer.cpp at line 649 and comment log_e
*/

#include <esp_task_wdt.h>
#include <Arduino.h>
#include <service.h>

void setup() {
	System_Init();
	Board_Init();
	Web_Init();
	esp_task_wdt_init(10, true);
    esp_task_wdt_add(NULL);
}

void loop() {
	static uint32_t max = 0;
	static uint32_t tmr = 0;
	uint32_t start = millis();
	Board_Tick(); //max time 27ms
	uint32_t res = millis() - start;
	max = res > max ? res : max;
	if (millis() - tmr > 5000) {
		Serial.printf("\nMAX TIME: %d \n", max);
		tmr = millis();
	}
	Web_Tick();
	esp_task_wdt_reset();
	
}