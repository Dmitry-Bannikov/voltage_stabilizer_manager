#include "common_data.h"


GyverPortal ui;

std::vector<Board> board;					//объекты плат

bool dataReqDelay = false;
uint8_t activeBoard = 0;
bool mqttReqResult = false;
bool webRefresh = true;  
uint8_t reqSuccess = 0;
uint8_t boardRequest = 0; //запрос на плату
uint32_t Board_SN = 0;


void LED_blink(uint16_t period_on, uint16_t period_off) {
  	static uint64_t tick = 0;
	static bool led_state = false;
	if (period_on <= 1) {
		digitalWrite(LED_BUILTIN, !period_on);
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

int getBoardSN(int sn) {
	int new_sn = 0;
	if (sn == 0) {
		uint8_t mac[6];
		WiFi.macAddress(mac);
		memcpy(&new_sn, mac, 4);
		new_sn += *(uint16_t*)(mac + 4);
		return new_sn;
	}
	return sn;
}