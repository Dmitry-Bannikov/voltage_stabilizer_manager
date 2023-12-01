#pragma once

#include <Arduino.h>
#include <data.h>
#include <Wire.h>
#include <mqtthandle.h>
#include <esp32-hal-i2c.h>

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
	Serial.println("Scanning boards...");
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
		Serial.printf("\n Read settings: %d", board[i].getMainSets()); 
		delay(250);
		Serial.printf("\n Read Data: %d", board[i].getDataRaw()); 
		delay(250);
	}
}

void boardTick() {
	static uint32_t tmr = 0;
	static uint32_t scanTmr = 0;
	//sendDwinData();
	uint8_t boardsAmnt = board.size();
	
	if (boardsAmnt && (millis() - tmr > 300)) {
		static uint8_t i = 0;
		board[i].tick();
		(i == boardsAmnt-1 ? (i=0) : (i++));
		tmr = millis();
	}
	
	if (millis() -  scanTmr < 10000) return;
	scanNewBoards();
	scanTmr = millis();
}

void scanNewBoards() {
	static uint8_t old_amount = 0; 
	static uint8_t counter = 0;
	counter++;
	Board::scanBoards(board, MAX_BOARDS);
	if (old_amount != board.size()) {
		webRefresh = true;
		old_amount = board.size();
	} 
	if (counter == 6 && board.size()) {
		counter = 0;
		for (uint8_t i = 0; i < board.size(); i++) {
			if(!board[i].getMainSets()) webRefresh = true;
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
	memoryWIFI.tick();
	if (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA) {
		LED_blink(100, 2000);
	} else if (WiFi.getMode() == WIFI_AP){
		LED_blink(1000);
	}
}























//-------------------------------------------------------------//