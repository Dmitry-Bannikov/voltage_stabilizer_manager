#include <Arduino.h>
#include <functions.h>



void setup() {
  connectingInit();
  memoryInit();
  portalInit();
}

void loop() {
  memoryTick();
  portalTick();
  dataHandler();
}

