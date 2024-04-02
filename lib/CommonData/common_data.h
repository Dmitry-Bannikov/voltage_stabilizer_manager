#pragma once

#include <GyverPortal.h>
#include <TimeTicker.h>
#include <Board.h>



#define MAX_BOARDS	3
extern GyverPortal ui;
extern GPtime t;
extern std::vector<Board> board;					//объекты плат

extern uint8_t activeBoard;
extern bool mqttConnected;
extern bool webRefresh;  
extern uint8_t boardRequest; //запрос на плату
extern uint8_t requestResult;
void LED_blink(uint16_t period_on, uint16_t period_off = 0);

