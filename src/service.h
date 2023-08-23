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
uint8_t scanBoards(uint8_t* addrs, const uint8_t num = 128);
void remember(Memory type); 
void recall(Memory type);
void boardTick();
void write_read();

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
	gNumBoards = scanBoards(i2c_boards_addrs);	//сканируем шину и получаем количество плат
	Serial.print("Scan boards: ");
	Serial.println(gNumBoards);
	for (int i = 0; i < gNumBoards; i++) {
		bool res = board[i].attach(i2c_boards_addrs[i]);
		Serial.print("Found board on address: ");
		Serial.println(board[i].getAddress());
		Serial.print("Send startkey: ");
		Serial.println(board[i].setStartKey());
	}

	uint32_t tmr = millis();
	int32_t gData[5];
	String data = "";
	String statis = "";
	int i = 0;
	while(1) {
		if (millis() - tmr > 1000){
			if (i % 10 == 0) {
				Serial.println(String("Online: ") + board[0].isOnline());
				board[0].getDataStr(data);
				Serial.println(data);
				Serial.println();
			}
			if (i == 60) {
				Serial.println("========Statis=======");
				board[0].getStatisStr(statis);
				Serial.println(statis);
				Serial.println("=====================");
				i = 0;
			}
			i++;
			tmr = millis();
		}
	}
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

uint8_t scanBoards(uint8_t* addrs, const uint8_t num) {
	uint8_t indx = 0;
	for (int i = 1; i < 128; i++) {
		Wire.beginTransmission(i);
		uint8_t res = Wire.endTransmission();
		if (res == 0 && indx < num) {
			addrs[indx] = i;
			indx++;
		}
	}
	return indx;
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
	
	uint8_t addrs[128] = {0};
	int32_t key = 0;
	if (millis() -  tmr > 1000) {
		
		tmr = millis();
	}
}

void write_read() {
	int32_t data[20] = {0};
	int32_t data_tx[20] = {0x22, 0};
	
	Wire.beginTransmission(0x01);
	Wire.write((uint8_t*)data_tx, sizeof(data_tx));
	Serial.println(String("Send result: ") + Wire.endTransmission());
	Wire.requestFrom(0x01, sizeof(data));
	uint8_t* p = reinterpret_cast<uint8_t*>(data);
	if (Wire.available()) {
		Wire.readBytes(p, sizeof(data));
		Serial.println(String("Get result: ") + data[0]);
	}
}






















