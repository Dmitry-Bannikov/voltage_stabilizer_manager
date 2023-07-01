#include <Arduino.h>
#include <EEManager.h>
#include <data.h>
#include <hub.h>
#include <functions.h>



void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     // Пара подмигиваний
  LED_switch(1);
  delay(30);
  LED_switch(0);
  EEPROM.begin(512);
  memoryWIFI.begin(0, MEMORY_KEY);
  memorySETS.begin(100, MEMORY_KEY);
  // memoryBoard.begin(100, MEMORY_KEY);
  hub_init();
}

void loop() {
  ui.tick();
  memoryWIFI.tick();
  memorySETS.tick();
  if (WiFi.status() == WL_CONNECTED) {
    LED_blink(1000);
  }
  else {
    LED_blink(50, 3000);
  }
  dataHandler();
}

