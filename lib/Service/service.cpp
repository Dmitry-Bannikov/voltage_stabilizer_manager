#include "Arduino.h"
#include "service.h"
#include "common_data.h"
#include "netconnection.h"
#include "mqtthandler.h"
#include "webinterface.h"
#include "devices.h"





void System_Init() {
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(21, INPUT);
	pinMode(22, INPUT);
	Serial.begin(115200);
	EEPROM.begin(2048);
	Serial.printf("\nProram Started! \rI2C pins state: %d | %d\n", digitalRead(21), digitalRead(22));
	Devices_Init();
}

void Board_Init() {
	Board::StartI2C();
	board.reserve(MAX_BOARDS);
	delay(10);
	scanNewBoards();
	for (uint8_t i = 0; i < board.size(); i++) {
		readCurrentCalibrate(i);
		delay(100);
	}
}

void Web_Init() {
	WifiInit();
	MqttInit();
	portalInit();
	uint8_t mac[6];
	WiFi.macAddress(mac);
	memcpy(&Board_SN, mac, 4);
	Serial.printf("\nStab SN: %d \n", Board_SN);
	Serial.println(WiFi.macAddress());
	webRefresh = true;
}




void Board_Tick() {
	static uint32_t tmr = 0, scanTmr = 0;
	static uint8_t denyDataRequest = 0;
	uint32_t period = dataReqDelay ? 5000 : 1000;
	uint8_t boardsAmnt = board.size();
	if (millis() - tmr > period && !boardRequest) {
		for (uint8_t i = 0; i < board.size() && !denyDataRequest; i++) {
			//t = ui.getSystemTime().encode();
			board[i].tick();
		}
		denyDataRequest > 0 ? denyDataRequest-- : (denyDataRequest = 0);
		tmr = millis();
	} else if (boardRequest){
		denyDataRequest = 3;
		BoardRequest(boardRequest);
	}

	if (millis() - scanTmr > 60000) {
		for (uint8_t i = 0; i < board.size() && !denyDataRequest; i++) {
			board[i].readAll();
			delay(50);
		}
		boardRequest = 2;
		scanTmr = millis();
	}
}

void System_Tick() {

}

void Web_Tick() {
	wifi_tick();
	Mqtt_tick();
	portalTick();
}

void scanNewBoards() {
	static uint8_t old_amount = 0; 
	Board::scanBoards(board, MAX_BOARDS);
	if (old_amount != board.size()) {
		webRefresh = true;
		old_amount = board.size();
	}
}

//Запросы на плату
void BoardRequest(uint8_t &request) {
	static uint8_t requestTry = 0;
	static uint32_t tmr = 0;
	if (!request || millis() - tmr < 500) return;
	if (!board[activeBoard].isAnswer() && request != 1 && request != 2) {
		request = 0;
		return;
	}
	delay(30);
	if (request < 10) {
		if (request == 1) {//reboot esp
			delay(1000);
			requestResult = 1;
			ESP.restart();
		} else  if (request == 2) { //rescan boards
			scanNewBoards();
			requestResult = 1;
		} else if (request == 3) { //save liters
			for (uint8_t i = 0; i < board.size(); i++) {
				delay(50);
				if (board[i].sendMainSets(0, 1, board[i].mainSets.Liter)) return;
			}
			requestResult = 2;
		} else if (request == 4) {	//set active
			if (board[activeBoard].readAll()) {
				requestResult = 1;
			}
			webRefresh = true;
		} else if (request == 5) {
			if (!board[activeBoard].sendSwitches(SW_OUTSIGN,1)) {
				requestResult = 1;
			}
		}	
	} else {
		uint8_t command = request / 10;
		uint8_t target = request % 10;
		if (command == 1) {//read settings
			if (board[target].readAll()) {
				requestResult = 2;
			}
		}
		else if (command == 2) {//write settings

			uint8_t res1 = board[target].sendMainSets();
			delay(20);
			uint8_t res2 = board[target].sendAddSets();
			if (!res1 && !res2) {
				requestResult = 2;
			}
			
		}
		else if (command == 3) {//reboot board
			if(!board[target].sendSwitches(SW_REBOOT, 1, 1)) {
				delay(250);
				requestResult = 2;
			}
		}
		else if (command == 4) {
			if (!board[target].sendSwitches(SW_RSTST, 1, 1)) {
				requestResult = 1;
			}
		}
		else if (command == 5) {
			requestResult = sendCurrentCalibrate(target);
		}
	}
	Serial.printf("\nBoard request: %d, Result: %d ", request, requestResult);
	if (requestResult > 0 || requestTry == 3) {
		requestTry = 0;
		request = 0;
		if (requestResult == 2) webRefresh = true;
	} else {
		requestTry++;
	}
	tmr = millis();
}

bool sendCurrentCalibrate(uint8_t brd) {
	board[brd].mainSets.CurrClbrtValue = (int16_t)(board[brd].CurrClbrtValue*100.0);
	board[brd].sendMainSets(14);
	board[brd].readMainSets(13);
	board[brd].CurrClbrtKoeff = ((float)board[brd].mainSets.CurrClbrtKoeff/100.0);
	return (board[brd].mainSets.CurrClbrtKoeff != 100 && board[brd].mainSets.CurrClbrtKoeff != 3588);
}

bool readCurrentCalibrate(uint8_t brd) {
	board[brd].readMainSets(13);
	board[brd].CurrClbrtKoeff = ((float)board[brd].mainSets.CurrClbrtKoeff/100.0);
	return (board[brd].mainSets.CurrClbrtKoeff != 100 && board[brd].mainSets.CurrClbrtKoeff != 3588);
}



//===========================================