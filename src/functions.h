#pragma once

#include <Arduino.h>
#include <data.h>
#include <hub.h>


void LED_switch(bool state);
void LED_blink(uint16_t period);
bool board_state_toStr(uint16_t board_state, String& board_state_str);
void dataHandler();



void LED_switch(bool state) {
  if (state) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
  }  
}

void LED_blink(uint16_t period) {
  static uint64_t tick = 0;
  static bool led_state = false;
  if (millis() - tick > period) {
    LED_switch(led_state = !led_state);
    tick = millis();
  }
}

void LED_blink(uint16_t period_on, uint16_t period_off) {
  static uint64_t tick = 0;
  static bool led_state = false;
  if (millis() - tick > (led_state?period_on:period_off)) {
    LED_switch(led_state = !led_state);
    tick = millis();
  }
}

bool board_state_toStr(uint16_t board_state, String& board_state_str) {
  if (board_state <= 1) {
    board_state_str = "Board State OK";
    return false;
  }
  String s = "";
  for (uint8_t i = 0; i < 16; i++) {
    if (board_state & (1<<i)) {
      if (i < 10) {
        s += "A0";
        s += String(i);
      } else {
        s += "A";
        s += String(i);
      }
      s += ", ";
    }
  }
  if (s.length()) {
    s.remove(s.length() - 1);
  }
  board_state_str = s;
  return true;
}

void dataHandler() {
  static uint32_t tick = 0;
  if (millis() -  tick >= 1000) {
    gData_stat = 44;
    board_state_toStr(gData_stat, gData_stat_str);
    gData_input = random(190, 300);
    gData_output = random(190, 300);
    gData_load = random(100, 1000)/100;
    tick = millis();
  }
}
