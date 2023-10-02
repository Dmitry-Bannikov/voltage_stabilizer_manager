#pragma once

#include <Arduino.h>
#include <data.h>
#include <Wire.h>

void connectionInit();
void memoryInit();
void LED_switch(bool state);
void LED_blink(uint16_t period_on, uint16_t period_off);
void scanNewBoards();
void boardTick();


void LED_switch(bool state) {
	if (state) {
		digitalWrite(LED_BUILTIN, LOW);
	}
	else {
		digitalWrite(LED_BUILTIN, HIGH);
	}  
}

void LED_blink(uint16_t period_on, uint16_t period_off = 0) {
  	static uint64_t tick = 0;
	static bool led_state = false;
	if (!period_off) {
		if (millis() - tick > period_on) {
		LED_switch(led_state = !led_state);
		tick = millis();
		}
	} else {
		if (millis() - tick > (led_state ? period_on : period_off)) {
		LED_switch(led_state = !led_state);
		tick = millis();
		}
	}
}

void connectionInit() {
	pinMode(LED_BUILTIN, OUTPUT);
	Wire.begin();
	Wire.setTimeout(500);
	delay(10);
	Serial.begin(115200);
	delay(10);
	Serial.println("Initializing connection!");
	board.reserve(MAX_BOARDS);
	#ifdef RELEASE
	scanNewBoards();
	#else
	board.emplace_back(0x10);
	board.emplace_back(0x12);
	#endif
	Serial.print("Boards found: ");
	Serial.println(board.size());
}

void memoryInit() {
	LED_switch(1);
	EEPROM.begin(512);
	memoryWIFI.begin(0, 127);
	LED_switch(0);
}

void boardTick() {
	static uint32_t tmr = 0;
	memoryWIFI.tick();
	for (uint8_t i = 0; i < board.size(); i++) {
		board[i].tick();
	}
	if (millis() -  tmr > 60000) {
		//scanNewBoards();
		tmr = millis();
	}
}

void scanNewBoards() {
	Board::scanBoards(board, MAX_BOARDS);
}