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
	EEPROM.begin(4096);
	Serial.printf("\n\n\nProram Started! \nI2C pins state: %d | %d\n", digitalRead(21), digitalRead(22));
	Devices_Init();
}

void Board_Init() {
	//========================//
	Board::StartI2C();
	board.reserve(MAX_BOARDS);
	scanNewBoards();
	Serial.printf("Boards found: %d \n", board.size());
}

void Web_Init() {
	WifiInit();
	MqttInit();
	portalInit();
	uint8_t mac[6];
	Board_SN = getBoardSN(0);
	Serial.printf("\nStab SN: %d \n", Board_SN);
	webRefresh = true;
}




void Board_Tick() {
	static uint32_t tmr = 0, scanTmr = 0;
	static uint8_t denyDataRequest = 0;
	uint8_t boardsAmnt = board.size();
	if (millis() - tmr > 1000 && !boardRequest) {
		for (uint8_t i = 0; i < board.size() && !denyDataRequest; i++) {
			board[i].tick();
		}
		denyDataRequest > 0 ? denyDataRequest-- : (denyDataRequest = 0);
		tmr = millis();
	}
	reqSuccess = BoardRequest(boardRequest);
	

	if (millis() - scanTmr > 30000) {
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
	Devices_Tick();
}

bool scanNewBoards() {
	bool res = false;
	static uint8_t old_amount = 0; 
	Board::scanBoards(board, MAX_BOARDS);
	if (old_amount != board.size()) res = true;
	old_amount = board.size();
	return res;
}

//Запросы на плату
uint8_t BoardRequest(uint8_t &request) {
	uint8_t res = 0;
	static uint8_t requestTry = 0;
	static uint32_t tmr = 0;
	if (!request || millis() - tmr < 500) return reqSuccess;

	if (request == 1) ESP.restart(); 			//1 - рестарт менеджера
	else if (request == 2) {					//2 - сканирование плат
		res = scanNewBoards()+1;
	} else if (request == 3) {					//3 - передача букв всем платам
		res = 2;
		ForBrds {
			if (!board[i].sendMainSets(0, 1, board[i].mainSets.Liter)) res = 1;
		}
	} else if (request == 4) {					//4 - выбор активной платы или нажатие на кнопку чтения настроек
		res = board[activeBoard].readAll() + 1;
	} else if (request == 5) {					//5 - отправка внешнего сигнала на активную плату
		res = board[activeBoard].sendSwitches(SW_OUTSIGN,1);
	} else if (request == 6) {					//6 - отправка настроек на активную плату
		res = board[activeBoard].sendMainSets() + 3;
	} else if (request == 7) {					//7 - рестарт активной платы
		res = board[activeBoard].sendSwitches(SW_REBOOT, 1);
	}  else if (request == 8) {					//8 - кнопка подстройки тока
		res = (board[activeBoard].setCurrClbrt() > 0.0);
	} else if (request >= 90) {					//90|91|92 - кнопки сброса статистики
		uint8_t target = request - 90;
		res = board[target].sendSwitches(SW_RSTST, 1);
	}
	Serial.printf("\nBoard request: %d, Result: %d ", request, res);
	if (res > 0 || requestTry == 3) {
		if (res == 2) webRefresh = true;
		requestTry = 0;
		request = 0;
		
	} else {
		requestTry++;
	}
	tmr = millis();
	return res;
}



//===========================================