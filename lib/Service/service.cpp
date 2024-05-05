#include "Arduino.h"
#include "service.h"
#include "common_data.h"
#include "netconnection.h"
#include "mqtthandler.h"
#include "webinterface.h"






void System_Init() {
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(21, INPUT);
	pinMode(22, INPUT);
	Serial.begin(115200);
	Serial.printf("\nProram Started! \rI2C pins state: %d | %d\n", digitalRead(21), digitalRead(22));
}

void Board_Init() {
	Board::StartI2C();
	board.reserve(MAX_BOARDS);
	delay(10);
	scanNewBoards();
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
}




void Board_Tick() {
	static uint32_t tmr = 0, scanTmr = 0;
	static uint8_t denyDataRequest = 0;
	uint32_t period = dataReqDelay ? 5000 : 1000;
	uint8_t boardsAmnt = board.size();
	if (millis() - tmr > period && !boardRequest) {
		for (uint8_t i = 0; i < board.size() && !denyDataRequest; i++) {
			t = ui.getSystemTime();
			board[i].tick(t.encode());
		}
		denyDataRequest > 0 ? denyDataRequest-- : (denyDataRequest = 0);
		tmr = millis();
	} else if (boardRequest && board.size()){
		denyDataRequest = 3;
		BoardRequest(boardRequest);
	}

	if (millis() - scanTmr > 60000) {
		for (uint8_t i = 0; i < board.size() && !denyDataRequest; i++) {
			board[i].readAll();
		}
		delay(100);
		scanNewBoards();
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

void BoardRequest(uint8_t &request) {
	if (!request) return;
	if (!board[activeBoard].isAnswer()) {
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
				webRefresh = true;
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
			if(!board[target].sendMainSets() && !board[target].sendAddSets()) {
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
			if (board[target].sendSwitches(SW_RSTST, 1, 1)) {
				requestResult = 1;
			}
		}
		else if (command == 5) {
			if (!board[activeBoard].sendSwitches(SW_OUTSIGN, 1, target)) {
				requestResult = 1;
			}
		}
		else if (command == 6) {
			board[target].mainSets.CurrClbrtValue = (int16_t)(board[activeBoard].CurrClbrtValue*100.0);
			board[target].sendMainSets(14, 1);
			board[target].readMainSets(15, 1);
			board[target].CurrClbrtKoeff = ((float)board[target].mainSets.CurrClbrtKoeff/100.0);
			requestResult = 1;
		}
	}
	
	if (requestResult > 0) {
		request = 0;
		webRefresh = requestResult == 2;
	}
}


//===========================================