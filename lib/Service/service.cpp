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
	if (board.size()) board[0].readAll();
}

void Web_Init() {
	WifiInit();
	MqttInit();
	portalInit();
	Board_SN = getBoardSN(0);
	Serial.printf("\nStab SN: %d \n", Board_SN);
	webRefresh = true;
}




void Board_Tick() {
	/*------------TEST!!!-------------------
	static uint32_t tmr = 0;
	if (millis() - tmr < 1000) return;
	uint32_t start = millis();
	float result = board[0].readDataRaw();
	float result1 = board[0].readStatsRaw();
	bool sets = board[0].readAll();
	Serial.printf("\nTime: %d | Data: %.1f | Stats: %.1f | Sets: %d ", millis() - start, result, result1, sets);
	tmr = millis();
	*/


	static uint32_t tmr = 0, scanTmr = 0;
	static uint8_t denyDataRequest = 0;
	uint8_t brdSize = board.size();
	if (millis() - tmr > 1000 && !g_boardRequest && brdSize) {
		static uint8_t brdCnt = 0;
		board[brdCnt].tick();
		if (brdCnt == brdSize - 1) {
			tmr = millis();
			brdCnt = 0;
		} else brdCnt++;
	} else {
		g_reqSuccess = BoardRequest(g_boardRequest);
	}
	
	

	if (millis() - scanTmr > (board.size() < 3 ? 10000 : 60000)) {
		g_boardRequest = 2;
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
	if (!request || millis() - tmr < 500) return g_reqSuccess;

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
		res = board[activeBoard].sendSwitches(SW_OUTSIGN,board[activeBoard].mainSets.Switches[SW_OUTSIGN]);
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