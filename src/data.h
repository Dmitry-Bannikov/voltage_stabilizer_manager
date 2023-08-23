#pragma once

#include <Board.h>

#define AP_DEFAULT_SSID     "Stab_AP_FREE"            // Стандартное имя точки доступа ESP (До 20-ти символов)
#define AP_DEFAULT_PASS     ""                      // Стандартный пароль точки доступа ESP (До 20-ти символов)
#define STA_DEFAULT_SSID    "honda"                      // Стандартное имя точки доступа роутера (До 20-ти символов)
#define STA_DEFAULT_PASS    "orteamoscow"                      // Стандартный пароль точки доступа роутера (До 20-ти символов)
#define STA_CONNECT_EN      1                       // 1/0 - вкл./выкл. подключение к роутеру 

#define MEMORY_KEY          126                     //ключ памяти (от 0 до 255), если изменить, то настройки сбросятся


struct {                                            // Структура со всеми настройками
    bool staModeEn = STA_CONNECT_EN;                // Подключаться роутеру по умолчанию?
    char apSsid[21] = AP_DEFAULT_SSID;              // Имя сети для AP режима по умолчанию
    char apPass[21] = AP_DEFAULT_PASS;              // Пароль сети для AP режима по умолчанию
    char staSsid[21] = STA_DEFAULT_SSID;            // Имя сети для STA режима по умолчанию
    char staPass[21] = STA_DEFAULT_PASS;            // Пароль сети для STA режима по умолчанию
} wifi_settings;

int32_t gTrimmers[8] = {0, 3, 0, 0, 220, 0, 1, 60};
#define gTrim_ignoreSets	gTrimmers[0]	//игнорировать настройки платы
#define gTrim_precision		gTrimmers[1]	//точность/гистерезис
#define gTrim_tuneIn		gTrimmers[2]	//подстройка напряжения входа
#define gTrim_tuneOut		gTrimmers[3]	//подстройка напряжения выхода
#define gTrim_targetVolt	gTrimmers[4]	//целевое напряжение
#define	gTrim_relSet		gTrimmers[5]	//поведение реле
#define gTrim_motType		gTrimmers[6]	//тип мотора
#define gTrim_tcRatio		gTrimmers[7]	//отношение трансформатора тока (x/5)

int32_t gBoardSets[8] = {-22, 22, 500, 2000, 40, 100, 150, 200};
#define gBSets_vMinTerm		gBoardSets[0]
#define gBSets_vMaxTerm		gBoardSets[1]
#define gBSets_emergToff	gBoardSets[2]
#define gBSets_emergTon		gBoardSets[3]
#define gBSets_mot1koef		gBoardSets[4]
#define gBSets_mot2koef		gBoardSets[5]
#define gBSets_mot3koef		gBoardSets[6]
#define gBSets_mot4koef		gBoardSets[7]

Board board[3];
uint8_t gNumBoards = 0;
uint8_t i2c_boards_addrs[3] = {0};