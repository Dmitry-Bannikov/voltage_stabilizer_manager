#pragma once

#include <Arduino.h>
#include <data.h>
#include <Wire.h>
#include <mqtthandle.h>


void connectionInit();
void memoryInit();
void LED_blink(uint16_t period_on, uint16_t period_off);
void scanNewBoards();
void boardTick();
void SerialTest(int16_t value);
void sendDwinData();
void WiFi_Init();
void WiFi_tick();

void LED_blink(uint16_t period_on, uint16_t period_off = 0) {
  	static uint64_t tick = 0;
	static bool led_state = false;
	if (!period_on) {
		digitalWrite(LED_BUILTIN, HIGH);
		return;
	} 
	if (period_on == 1) {
		digitalWrite(LED_BUILTIN, LOW);
		return;
	}
	if (!period_off) {
		if (millis() - tick > period_on) {
			digitalWrite(LED_BUILTIN, (led_state = !led_state));
			tick = millis();
		}
	} else {
		if (millis() - tick > (led_state ? period_on : period_off)) {
			digitalWrite(LED_BUILTIN, (led_state = !led_state));
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
	//HardwareSerial Serial2(115200);
	//Dwin.begin(&Serial, processData);
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
	LED_blink(1);
	EEPROM.begin(512);
	memoryWIFI.begin(0, 127);
	LED_blink(0);
	for (uint8_t i = 0; i < board.size(); i++) {	
		uint8_t attempts = 0;
		// пробуем считать настройки с платы
		while (board[i].getMainSets() || board[i].getAddSets()  || attempts < 5) {
			attempts++;
			delay(1000);
		}
	}
}

void boardTick() {
	static uint32_t tmr = 0;
	memoryWIFI.tick();
	sendDwinData();
	for (uint8_t i = 0; i < board.size(); i++) {
		Wire.clearWriteError();
		board[i].tick();
	}
	if (board.size()) {
		if (millis() -  tmr < 30000) return;
		scanNewBoards();
	} else {
		if (millis() - tmr < 3000) return;
		scanNewBoards();
	}
	tmr = millis();
}

void scanNewBoards() {
	static uint8_t old_amount = 0; 
	static uint8_t counter = 0;
	counter++;
	Board::scanBoards(board, MAX_BOARDS);
	if (old_amount != board.size()) {
		webRefresh = true;
		old_amount = board.size();
		Serial.println(String("Boards found: ") + board.size());
	}
	if (counter == 3) {
		counter = 0;
		if (!board[activeBoard].getMainSets() && !board[activeBoard].getAddSets())
		{
			webRefresh = true;
		}
	}
}

void sendDwinData() {
	static uint32_t tmr;
	if (millis() - tmr < 1000) return;
	for (uint8_t i = 0; i < board.size(); i++) {
		//SerialTest(0x1234);
		Dwin.writeValue(0x5000, board[0].mainData.inputVoltage);
		/*
		Dwin.addNewValue(board[i].mainData.inputVoltage);
		Dwin.addNewValue(board[i].mainData.outputVoltage);
		Dwin.addNewValue(board[i].mainData.outputCurrent);
		Dwin.addNewValue(board[i].mainData.outputPower);
		Dwin.writeAddedValues(0x5000 + i*256);
		Dwin.waitUntillTx();
		*/
		
	}
	tmr = millis();
}

void WiFi_Init() {
	if (wifi_settings.staModeEn)
	{
		WiFi.mode(WIFI_STA);
		WiFi.begin(wifi_settings.staSsid, wifi_settings.staPass);
		int attemptCount = 0;
		while (WiFi.status() != WL_CONNECTED)
		{
			LED_blink(100);
			if (++attemptCount == 10)
			{
				LED_blink(0);
				wifi_settings.staModeEn = 0; // переключаемся на режим точки доступа
				memoryWIFI.updateNow();		 // сохраняемся
				ESP.restart();				 // перезапускаем есп
				return;
			}
			delay(1000);
		}
		Serial.println(WiFi.localIP());
	}
	// Иначе создаем свою сеть
	else
	{
		WiFi.mode(WIFI_AP);
		WiFi.softAP(wifi_settings.apSsid, wifi_settings.apPass);
		delay(1000);
		Serial.println(WiFi.softAPIP());
	}
}

void WiFi_tick() {
	if (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA) {
		LED_blink(100, 2000);
	} else if (WiFi.getMode() == WIFI_AP){
		LED_blink(1000);
	} else {
		//WiFi_Init(); //test
	}
}























//-------------------------------------------------------------//