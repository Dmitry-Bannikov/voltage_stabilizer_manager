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
#define I2C_BOARD_ADDR					    0x01
#define I2C_REQUEST_SEND_PARAMS			0x01
#define I2C_REQUEST_GET_PARAMS			0x02
#define I2C_REQUEST_SEND_DATA			  0x03
#define I2C_REQUEST_GET_DATA			  0x04
#define I2C_REQUEST_SEND_STAT			  0x05
#define I2C_REQUEST_GET_STAT			  0x06
#define I2C_REQUEST_DISABLE_TRIMS		0x07
#define I2C_REQUEST_ENABLE_TRIMS		0x08

#define DEF_TRIM_PRECISION				  3
#define DEF_TRIM_TUNEIN					    0
#define DEF_TRIM_TUNEOUT				    0
#define DEF_TRIM_TARGETVOLT				  220
#define DEF_TRIM_RELSET					    0
#define DEF_TRIM_MOTTYPE				    1
#define DEF_TRIM_TCRATIO				    60



//------------------------BOARD DATA------------------------//
int16_t gData[4] = {
		0,
		0,
		0,
		0
};
#define gData_input			gData[0]
#define gData_output		gData[1]
#define gData_load			gData[2]
#define gData_stat			gData[3]



//----------------------BOARD TRIMMERS----------------------//
int16_t gTrimmers[7] = {
		DEF_TRIM_PRECISION,
		DEF_TRIM_TUNEIN,
		DEF_TRIM_TUNEOUT,
		DEF_TRIM_TARGETVOLT,
		DEF_TRIM_RELSET,
		DEF_TRIM_MOTTYPE,
		DEF_TRIM_TCRATIO
};

#define gTrim_precision		gTrimmers[0]
#define gTrim_tuneIn		gTrimmers[1]
#define gTrim_tuneOut		gTrimmers[2]
#define gTrim_targetVolt	gTrimmers[3]
#define	gTrim_relSet		gTrimmers[4]
#define gTrim_motType		gTrimmers[5]
#define gTrim_tcRatio		gTrimmers[6]

//------------------------Служебные переменные-------------------------------------//
String gData_stat_str = "";
bool needUpdateFlag = false;
int min_pwr_GLOB = 50;

