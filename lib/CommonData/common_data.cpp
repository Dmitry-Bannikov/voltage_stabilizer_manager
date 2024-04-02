#include "common_data.h"


GyverPortal ui;
GPtime t;
std::vector<Board> board;					//объекты плат

uint8_t activeBoard = 0;
bool mqttConnected = false;
bool webRefresh = true;  
uint8_t boardRequest = 0; //запрос на плату
uint8_t requestResult = 0;


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