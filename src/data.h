#pragma once
#include <Arduino.h>
//================= Настройки ==================//
#define AP_DEFAULT_SSID     "Stab_AP_FREE"            // Стандартное имя точки доступа ESP (До 20-ти символов)
#define AP_DEFAULT_PASS     ""                      // Стандартный пароль точки доступа ESP (До 20-ти символов)
#define STA_DEFAULT_SSID    ""                      // Стандартное имя точки доступа роутера (До 20-ти символов)
#define STA_DEFAULT_PASS    ""                      // Стандартный пароль точки доступа роутера (До 20-ти символов)
#define STA_CONNECT_EN      0                       // 1/0 - вкл./выкл. подключение к роутеру 

#define MEMORY_KEY          128                     //ключ памяти (от 0 до 255), если изменить, то настройки сбросятся




//--------------------Настройки Сети------------------------------------------------//
#define NET_MODE_AP         0
#define NET_MODE_STA        1
int networkConnectionMode = NET_MODE_AP;            // режим вифи: 0 - AP, 1 - STA
struct {                                            // Структура со всеми настройками
    bool staModeEn = STA_CONNECT_EN;                // Подключаться роутеру по умолчанию?
    char apSsid[21] = AP_DEFAULT_SSID;              // Имя сети для AP режима по умолчанию
    char apPass[21] = AP_DEFAULT_PASS;              // Пароль сети для AP режима по умолчанию
    char staSsid[21] = STA_DEFAULT_SSID;            // Имя сети для STA режима по умолчанию
    char staPass[21] = STA_DEFAULT_PASS;            // Пароль сети для STA режима по умолчанию
} wifi_settings;


//-----------------------Данные и настройки платы стабилизатора------------------------//
struct {
    uint16_t board_state;
    int inputVoltage;
    int outputVoltage;
    float outputCurrent;
} stab_data_toprint;

struct trim_save_t {                                        // Структура с данными для сохранения
  //настройки платы
  int vprecision = 3;                   //точность, гистерезис
  int vtuneIn = 0;                      //подстройка напряжения по входу
  int vtuneOut = 0;                     //подстройка напряжения по выходу
  int vconstOut = 220;                  //напряжение регуляции
  int relBehavior = -1;                 //поведение реле
  int startpwr = 100;
} stab_trim_save;

struct {
                                        //настройки платы
  int vprecision = 3;                   //точность, гистерезис
  int vtuneIn = 0;                      //подстройка напряжения по входу
  int vtuneOut = 0;                     //подстройка напряжения по выходу
  int vconstOut = 220;                  //напряжение регуляции
  int relBehavior = 0;                  //поведение реле
  int startpwr = 100;                   //начальная мощность
} stab_trim_send;


//------------------------Служебные переменные-------------------------------------//
String board_state_str = "";
bool needUpdateFlag = false;
int min_pwr_GLOB = 50;

