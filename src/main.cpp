#include <Arduino.h>
#include <data.h>
#include <hub.h>

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     // Пара подмигиваний
  for (uint8_t i = 0; i < 6; i++) { // Для индикации запуска
    digitalWrite(LED_BUILTIN, LOW);
    delay(30);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(30);
  }
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_BUILTIN, INPUT); // Выключаем и продолжаем
  EEPROM.begin(512);
  memoryWIFI.begin(0, MEMORY_KEY);
  // memoryBoard.begin(100, MEMORY_KEY);
  hub_init();
}

void loop() {
  ui.tick();
  memoryWIFI.tick();
}
