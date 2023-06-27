#pragma once
#include <Arduino.h>
#include <EEManager.h>

//================= Настройки ==================//
#define AP_DEFAULT_SSID     "Stab_board"            // Стандартное имя точки доступа ESP (До 20-ти символов)
#define AP_DEFAULT_PASS     ""                      // Стандартный пароль точки доступа ESP (До 20-ти символов)
#define STA_DEFAULT_SSID    ""                      // Стандартное имя точки доступа роутера (До 20-ти символов)
#define STA_DEFAULT_PASS    ""                      // Стандартный пароль точки доступа роутера (До 20-ти символов)
#define STA_CONNECT_EN      0                       // 1/0 - вкл./выкл. подключение к роутеру 

#define MEMORY_KEY          127                     //ключ памяти (от 0 до 255), если изменить, то при следующем запуске данные не 
                                                    //будут читаться из памяти



struct {                                            // Структура со всеми настройками
    bool staMode = false;                           // режим вифи: false - AP, true - STA
    bool staModeEn = STA_CONNECT_EN;                // Подключаться роутеру по умолчанию?
    char apSsid[21] = AP_DEFAULT_SSID;              // Имя сети для AP режима по умолчанию
    char apPass[21] = AP_DEFAULT_PASS;              // Пароль сети для AP режима по умолчанию
    char staSsid[21] = STA_DEFAULT_SSID;            // Имя сети для STA режима по умолчанию
    char staPass[21] = STA_DEFAULT_PASS;            // Пароль сети для STA режима по умолчанию
} wifi_settings;


struct {
    uint16_t board_errors;
    int inputVoltage;
    int outputVoltage;
    int outputLoad;
} stab_data_toprint;


struct {                                            // Структура с данными для сохранения
    uint16_t board_errors = 0;                      // Ошибки и флаги состояния платы
    int days_from_start = 0;                        // Прошло дней от перезагрузки
    int max_inputVolt_day = 220;                    //максимальное напряжение за сутки
    int min_inputVolt_day = 220;                    //минимальное напряжение за сутки

    //настройки платы
    int vprecision = 3;                             //точность, гистерезис
    int vtuneIn = 0;                                //подстройка напряжения по входу
    int vtuneOut = 0;                               //подстройка напряжения по выходу
    int vconstOut = 220;                            //напряжение регуляции
    int relBehavior = 0;                            //поведение реле
    int startpwr = 100;                             //начальная мощность
} stab_data_tosave;

EEManager memoryBoard(stab_data_tosave);
EEManager memoryWIFI(wifi_settings, 10000);


void stab_data_default() {

}