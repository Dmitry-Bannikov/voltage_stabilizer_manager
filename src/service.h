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
void SerialTest(int16_t value);
void processData();
void sendDwinData();


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
	Dwin.begin(&Serial2, processData);
	delay(10);
	Serial.println("Initializing connection!");
	board.reserve(MAX_BOARDS);
	#ifdef RELEASE
	scanNewBoards();
	#else
	board.emplace_back(0x10);
	board.emplace_back(0x12);
	#endif
}

void memoryInit() {
	LED_switch(1);
	EEPROM.begin(512);
	memoryWIFI.begin(0, 127);
	LED_switch(0);
	for (uint8_t i = 0; i < board.size(); i++) {	
		uint8_t attempts = 0;
		// пробуем считать настройки с платы
		while (board[i].getMainSets() || board[i].getAddSets()  || attempts < 5) {
			attempts++;
		}
	}
	for (uint8_t i = 0; i < board.size();i++) {	//раздаем адреса памяти платам
		if (i > 1) {
			board[i].setMemAddr(board[i-1].getEndMemAddr()); //или board[i].setMemAddr(i*100+100);
		} else {
			board[0].setMemAddr(100);
		}
		
	}
}

void boardTick() {
	static uint32_t tmr = 0;
	memoryWIFI.tick();
	
	for (uint8_t i = 0; i < board.size(); i++) {
		Wire.clearWriteError();
		board[i].tick();
	}
	if (millis() -  tmr > 60000) {
		scanNewBoards();
		tmr = millis();
	}
}

void scanNewBoards() {
	static uint8_t old_amount = 0; 

	Board::scanBoards(board, MAX_BOARDS);
	if (old_amount != board.size()) {
		webRefresh = true;
		old_amount = board.size();
		Serial.println(String("Boards found: ") + board.size());
	}
	
}


void SerialTest(int16_t value) {
	static uint32_t tmr = 0;
	if (millis() - tmr < 1000) return;
	uint8_t buffer[8];
	buffer[0] = 0x5A;
	buffer[1] = 0xA5;
	buffer[2] = 0x05;
	buffer[3] = 0x82;
	buffer[4] = 0x50;
	buffer[5] = 0x00;
	buffer[6] = (uint8_t)(value >> 8);
	buffer[7] = (uint8_t)(value & 0xFF);
	Serial2.write(buffer, sizeof(buffer));
	tmr = millis();
}

void processData() {
	
}
void sendDwinData() {
	static uint32_t tmr;
	if (millis() - tmr < 1000) return;
	for (uint8_t i = 0; i < board.size(); i++) {
		Dwin.addNewValue(board[i].mainData.inputVoltage);
		Dwin.addNewValue(board[i].mainData.outputVoltage);
		Dwin.addNewValue(board[i].mainData.outputCurrent);
		Dwin.addNewValue(board[i].mainData.outputPower);
		Dwin.writeAddedValues(0x5000 + i*256);
		Dwin.waitUntillTx();
	}
	
}