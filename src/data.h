#pragma once
#include <EEManager.h>
#include <Board.h>
#include <Display.h>

#define RELEASE

#define AP_DEFAULT_SSID     "Stab_AP_FREE"			// Стандартное имя точки доступа ESP (До 20-ти символов)
#define AP_DEFAULT_PASS     ""						// Стандартный пароль точки доступа ESP (До 20-ти символов)
#ifdef RELEASE
#define STA_DEFAULT_SSID    "honda"					// Стандартное имя точки доступа роутера (До 20-ти символов)
#define STA_DEFAULT_PASS    "orteamoscow"			// Стандартный пароль точки доступа роутера (До 20-ти символов)
#define STA_CONNECT_EN      1						// 1/0 - вкл./выкл. подключение к роутеру 
#else
#define STA_DEFAULT_SSID    "" 
#define STA_DEFAULT_PASS    "" 
#define STA_CONNECT_EN      0 
#endif
#define MEMORY_KEY          126                     //ключ памяти (от 0 до 255), если изменить, то настройки сбросятся

struct wifisets {                                   // Структура со всеми настройками wifi
    bool staModeEn = STA_CONNECT_EN;                // Подключаться роутеру по умолчанию?
    char apSsid[21] = AP_DEFAULT_SSID;              // Имя сети для AP режима по умолчанию
    char apPass[21] = AP_DEFAULT_PASS;              // Пароль сети для AP режима по умолчанию
    char staSsid[21] = STA_DEFAULT_SSID;            // Имя сети для STA режима по умолчанию
    char staPass[21] = STA_DEFAULT_PASS;            // Пароль сети для STA режима по умолчанию
} wifi_settings;

#define MAX_BOARDS	3
std::vector<Board> board;					//объекты плат
EEManager memoryWIFI(wifi_settings, 20000);
Display Dwin(&Serial);

uint8_t activeBoard = 0;

bool webRefresh = true;  

