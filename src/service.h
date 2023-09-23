#pragma once

#include <Arduino.h>
#include <data.h>
#include <Wire.h>
#include <EEManager.h>

enum Memory {
	TRIMS,
	BSETS,
	WIFI
};

void connectionInit();
void memoryInit();
void LED_switch(bool state);
void LED_blink(uint16_t period_on, uint16_t period_off);
void memoryTick();
uint8_t scanBoards(std::vector<uint8_t>&addrs, std::vector<uint8_t>denied);
void remember(Memory type); 
void recall(Memory type);
void boardTick();
void scanNewBoards();

EEManager memoryWIFI(wifi_settings, 20000);
EEManager memoryTrims(gTrimmers, 20000);
EEManager memoryBSets(gBoardSets, 20000);

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
	Wire.setTimeOut(250);
	delay(10);
	Serial.begin(115200);
	delay(10);
	Serial.println("Initializing connection!");
	//scanNewBoards();

}

void memoryInit() {
	LED_switch(1);
	EEPROM.begin(512);
	recall(BSETS);
	recall(WIFI);
	LED_switch(0);
}

void memoryTick() {
	memoryWIFI.tick();
	memoryTrims.tick();
	memoryBSets.tick();
}

uint8_t scanBoards(std::vector<uint8_t>&addrs, std::vector<uint8_t>denied) {
	uint8_t indx = 0;
	for (int addr = 1; addr < 128 && addrs.size() <= 3; addr++) {
		if (std::find(denied.begin(), denied.end(), addr ) != denied.end()) { //если адрес есть в списке "запрещенных"
			continue; //пропускаем итерацию
		}
		Wire.beginTransmission(addr);
		uint8_t error = Wire.endTransmission();
		//если ошибки нет и адреса нет в списке существующих адресов, то добавляем в конец списка
		if (error == 0 && std::find(addrs.begin(), addrs.end(), addr ) == addrs.end()) { 
			addrs.push_back(addr);
		} 
		//если ошибка есть и адрес есть в списке, то удаляем адрес из списка
		if (error != 0 && std::find(addrs.begin(), addrs.end(), addr ) != addrs.end()) {
			auto it = std::remove(addrs.begin(), addrs.end(), addr);
    		addrs.erase(it, addrs.end());
		}
	}
	return addrs.size();
}

void remember(Memory type) {
	if (type == TRIMS) {
		memoryTrims.updateNow();
	} else if (type == BSETS) {
		memoryBSets.updateNow();
	} else if (type == WIFI) {
		memoryWIFI.updateNow();	
	}
}

void recall(Memory type) {
	if (type == TRIMS) {
		memoryTrims.begin(100, MEMORY_KEY);
	} else if (type == BSETS) {
		memoryBSets.begin(140, MEMORY_KEY);
	} else if (type == WIFI) {
		memoryWIFI.begin(0, 127);
	}
}

void boardTick() {
	static uint32_t tmr = 0;
	if (millis() -  tmr > 60000) {
		scanNewBoards();
		tmr = millis();
	}
}

void scanNewBoards() {
	static uint8_t numBoards = 0;
	scanBoards(i2c_boards, i2c_other_addrs);
	for (uint8_t addr: i2c_boards) {
		board.emplace_back(addr);
	}
	if (board.size() != numBoards) {
		espStarted = false;
		numBoards = board.size();
	}
}




















