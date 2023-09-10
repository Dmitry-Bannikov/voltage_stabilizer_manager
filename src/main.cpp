#include <Arduino.h>
#include <portal.h>
#include <service.h>


void setup() {
	connectionInit();
	memoryInit();
	portalInit();
}

void loop() {
	//comment for test
	//boardTick();
	memoryTick();
	portalTick();
}
