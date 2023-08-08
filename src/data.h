#pragma once
#include <Arduino.h>
//================= Настройки ==================//
#define AP_DEFAULT_SSID     "Stab_AP_FREE"            // Стандартное имя точки доступа ESP (До 20-ти символов)
#define AP_DEFAULT_PASS     ""                      // Стандартный пароль точки доступа ESP (До 20-ти символов)
#define STA_DEFAULT_SSID    "honda"                      // Стандартное имя точки доступа роутера (До 20-ти символов)
#define STA_DEFAULT_PASS    "orteamoscow"                      // Стандартный пароль точки доступа роутера (До 20-ти символов)
#define STA_CONNECT_EN      1                       // 1/0 - вкл./выкл. подключение к роутеру 

#define MEMORY_KEY          127                     //ключ памяти (от 0 до 255), если изменить, то настройки сбросятся




//--------------------Настройки Сети------------------------------------------------//
#define NET_MODE_AP         0
#define NET_MODE_STA        1
int networkConnectionMode = NET_MODE_STA;            // режим вифи: 0 - AP, 1 - STA
struct {                                            // Структура со всеми настройками
    bool staModeEn = STA_CONNECT_EN;                // Подключаться роутеру по умолчанию?
    char apSsid[21] = AP_DEFAULT_SSID;              // Имя сети для AP режима по умолчанию
    char apPass[21] = AP_DEFAULT_PASS;              // Пароль сети для AP режима по умолчанию
    char staSsid[21] = STA_DEFAULT_SSID;                     // Имя сети для STA режима по умолчанию
    char staPass[21] = STA_DEFAULT_PASS;               // Пароль сети для STA режима по умолчанию
} wifi_settings;


//-----------------------Данные и настройки платы стабилизатора------------------------//
#define I2C_BOARD_ADDR_1					0x01
#define I2C_BOARD_ADDR_2					0x02
#define I2C_BOARD_ADDR_3					0x03

//запросы к ведомому
#define I2C_SLAVE_GET_PARAMS			0x01
#define I2C_SLAVE_SEND_PARAMS			0x02
#define I2C_SLAVE_SEND_DATA				0x03
#define I2C_SLAVE_SEND_STAT				0x04
#define I2C_SLAVE_DISABLE_TRIMS			0x05
#define I2C_SLAVE_ENABLE_TRIMS			0x06
//запросы от ведущего
#define I2C_MASTER_GET_PARAMS			0x07
#define I2C_MASTER_GET_DATA				0x08
#define I2C_MASTER_GET_STAT				0x09

#define DEF_TRIM_PRECISION				  	3
#define DEF_TRIM_TUNEIN					    0
#define DEF_TRIM_TUNEOUT				    0
#define DEF_TRIM_TARGETVOLT				  	220
#define DEF_TRIM_RELSET					    0
#define DEF_TRIM_MOTTYPE				    1
#define DEF_TRIM_TCRATIO				    60



//------------------------BOARD DATA------------------------//
int16_t gData[4] = {0};
#define gData_input			gData[0]
#define gData_output		gData[1]
#define gData_load			gData[2]
#define gData_stat			gData[3]


//----------------------BOARD TRIMMERS----------------------//
int16_t gTrimmers[8] = {0};
#define gTrim_precision		gTrimmers[0]
#define gTrim_tuneIn		gTrimmers[1]
#define gTrim_tuneOut		gTrimmers[2]
#define gTrim_targetVolt	gTrimmers[3]
#define	gTrim_relSet		gTrimmers[4]
#define gTrim_motType		gTrimmers[5]
#define gTrim_tcRatio		gTrimmers[6]
#define gTrim_ignoreSets	gTrimmers[7]

//---------------------BOARD STATISTICS------------------------//
int16_t gStats[] = {0};




//------------------------Служебные переменные-------------------------------------//
String gData_stat_str = "";
bool needUpdateFlag = false;
int16_t i2c_master_tx_buffer[10] = {0};
int16_t i2c_master_rx_buffer[20] = {0};


#define SERIAL_DEBUG