#pragma once

#include <GyverPortal.h>
#include <TimeTicker.h>
#include <Board.h>

#define S(arg)  String(arg)

#define MAX_BOARDS	3
extern GyverPortal ui;
extern GPtime t;
extern std::vector<Board> board;					//объекты плат

extern bool dataReqDelay;
extern uint8_t activeBoard;
extern bool mqttReqResult;
extern bool webRefresh;  
extern uint8_t boardRequest; //запрос на плату
extern uint8_t requestResult;
extern uint32_t Board_SN;

void LED_blink(uint16_t period_on, uint16_t period_off = 0);
int getBoardSN(int sn);