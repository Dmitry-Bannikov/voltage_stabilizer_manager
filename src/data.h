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



std::vector<Board> board;
std::vector<uint8_t> i2c_boards;
std::vector<uint8_t> i2c_other_addrs = {0x68};
std::vector<uint8_t> i2c_active_board(3);

uint32_t boards_worktime[3] = {0};

String gBoard_data[3];
String gBoard_stat[3];
bool espStarted = false;