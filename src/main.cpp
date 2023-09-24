#include <Arduino.h>
#include <portal.h>
#include <service.h>
#include <esp_task_wdt.h>

void setup() {
	connectionInit();
	memoryInit();
	portalInit();
	esp_task_wdt_init(5, true);
	esp_task_wdt_add(NULL);
}

void loop() {
	boardTick();
	memoryTick();
	portalTick();
	esp_task_wdt_reset();
}
