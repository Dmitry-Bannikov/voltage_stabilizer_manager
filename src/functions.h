#pragma once

#include <Arduino.h>
#include <data.h>
#include <hub.h>


void LED_switch(bool state);
void LED_blink(uint16_t period);
bool board_state_toStr(uint16_t board_state, String& board_state_str);
void dataHandler();
int motor_type_to_startpwr(int default_startpwr, int mot_type);
void trim_send_to_save();
void trim_save_to_send();
void trim_save_validation(trim_save_t *t);
void hub_init();


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
    s.remove(s.length() - 2);
  }
  board_state_str = s;
  return true;
}

void dataHandler() {
  static uint32_t tick = 0;
  if (millis() -  tick >= 1000) {
    stab_data_toprint.board_state = 44;
    board_state_toStr(stab_data_toprint.board_state, board_state_str);
    stab_data_toprint.inputVoltage = random(190, 300);
    stab_data_toprint.outputVoltage = random(190, 300);
    stab_data_toprint.outputCurrent = random(100, 1000)/100;
    tick = millis();
  }
}

int motortype_to_startpwr(int mot_type) {
  mot_type += 1;
  mot_type = (mot_type > 4 ? 4 : mot_type);
  mot_type = (mot_type < 1 ? 1 : mot_type);
  return min_pwr_GLOB * mot_type;
} 

int startpwr_to_motortype (int startpwr) {
  startpwr = (startpwr > 200 ? 200 : startpwr);
  startpwr = (startpwr < 50 ? 50 : startpwr);
  int res = startpwr/min_pwr_GLOB;
  return res - 1;
}

void trim_save_validation(trim_save_t *t) {
  t->vprecision = constrain(t->vprecision, 1, 5);
  t->startpwr = constrain(t->startpwr, 50, 200);
  t->relBehavior = constrain(t->relBehavior, 0, 2);
  t->vconstOut = constrain(t->vconstOut, 210, 240);
  t->vtuneIn = constrain(t->vtuneIn, -6, 6);
  t->vtuneOut = constrain(t->vtuneOut, -6, 6);
}

