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
#define I2C_SLAVE_GET_BSETS				0x05
//эти запросы передаем мастеру
#define I2C_MASTER_GET_PARAMS			0x11
#define I2C_MASTER_GET_DATA				0x12
#define I2C_MASTER_GET_STAT				0x13

#define DEF_TRIM_PRECISION				  	3
#define DEF_TRIM_TUNEIN					    0
#define DEF_TRIM_TUNEOUT				    0
#define DEF_TRIM_TARGETVOLT				  	220
#define DEF_TRIM_RELSET					    0
#define DEF_TRIM_MOTTYPE				    1
#define DEF_TRIM_TCRATIO				    60



//------------------------BOARD DATA------------------------//
int16_t gData[6] = {0};
#define gData_input			gData[0]
#define gData_output		gData[1]
#define gData_load			gData[2]
#define gData_stat			gData[3]
#define gData_fullpwr		gData[4]


//----------------------BOARD TRIMMERS----------------------//
int16_t gTrimmers[8] = {0};
#define gTrim_ignoreSets	gTrimmers[0]	//игнорировать настройки платы
#define gTrim_precision		gTrimmers[1]	//точность/гистерезис
#define gTrim_tuneIn		gTrimmers[2]	//подстройка напряжения входа
#define gTrim_tuneOut		gTrimmers[3]	//подстройка напряжения выхода
#define gTrim_targetVolt	gTrimmers[4]	//целевое напряжение
#define	gTrim_relSet		gTrimmers[5]	//поведение реле
#define gTrim_motType		gTrimmers[6]	//тип мотора
#define gTrim_tcRatio		gTrimmers[7]	//отношение трансформатора тока (x/5)

//--------------------BOARD SETS------------------//
int16_t gBoardSets[8] = {
    -22,
    22,
    500,
    2000,
    20,
    100,
    150,
    200
};
#define gBSets_vMinTerm		gBoardSets[0]
#define gBSets_vMaxTerm		gBoardSets[1]
#define gBSets_emergToff	gBoardSets[2]
#define gBSets_emergTon		gBoardSets[3]
#define gBSets_mot1koef		gBoardSets[4]
#define gBSets_mot2koef		gBoardSets[5]
#define gBSets_mot3koef		gBoardSets[6]
#define gBSets_mot4koef		gBoardSets[7]




//---------------------BOARD STATISTICS------------------------//
int16_t gStatis[15] = {0};
#define gStat_OutV_max				gStatis[0]
#define gStat_OutV_avg				gStatis[1]
#define gStat_OutV_min				gStatis[2]
#define gStat_InV_max				gStatis[3]
#define gStat_InV_avg				gStatis[4]
#define gStat_InV_min				gStatis[5]
#define gStat_Curr_max				gStatis[6]
#define gStat_Curr_avg				gStatis[7]
#define gStat_Curr_min				gStatis[8]
#define gStat_FullP_max				gStatis[9]
#define gStat_FullP_avg				gStatis[10]
#define gStat_FullP_min				gStatis[11]





//------------------------Служебные переменные-------------------------------------//
String gData_stat_str = "";
bool needUpdateFlag = false;
int16_t i2c_master_tx_buffer[10] = {0};
int16_t i2c_master_rx_buffer[20] = {0};


#define SERIAL_DEBUG