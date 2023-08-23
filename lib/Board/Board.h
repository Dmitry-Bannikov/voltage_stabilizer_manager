#pragma once
//библиотека общения с платой
//
/*методы библиотеки
*!!!ВАЖНО!!! Ключи и команды должны соответствовать таким же ключам и командам в плате
* Структура пакета:
1) int32_t command || int32_t key;	//стартовый ключ или команда. По ним устройство понимает, что нужно делать с данными
2) int32_t* data;					//данные
3) int32_t terminator;				//завершающий ключ
*
*===Коды ошибок методов===
 0: Ошибок нет
 1: Объект не инициализирован
 2: Ошибка передачи запроса/данных на плату
 3: Таймаут ожидания приема
 4: Стартовый код не соответствует запросу
*
*/



#include <Arduino.h>
/*
#define gStat_startkey				gStatis[0]
#define gStat_output_max			gStatis[1]
#define gStat_output_avg			gStatis[2]
#define gStat_output_min			gStatis[3]
#define gStat_input_max				gStatis[4]
#define gStat_input_avg				gStatis[5]
#define gStat_input_min				gStatis[6]
#define gStat_load_max				gStatis[7]
#define gStat_load_avg				gStatis[8]
#define gStat_fullpwr_max			gStatis[9]
#define gStat_fullpwr_avg			gStatis[10]
#define gStat_errors				gStatis[11]
*/


#define I2C_DATA_START						0x30
#define I2C_TRIM_START						0x35
#define I2C_BSET_START						0x40
#define I2C_STAT_START						0x45
#define I2C_SET_STARTKEY					0x50
#define I2C_TERMINATOR						-255

#define I2C_REQUEST_TRIMS					0x21
#define I2C_REQUEST_DATA					0x22
#define I2C_REQUEST_STAT					0x23
#define I2C_REQUEST_REBOOT					0x24
#define I2C_REQUEST_NOREG					0x25

#define RX_BUF_SIZE							20
#define TX_BUF_SIZE							20





class Board
{
private:
	enum BufferType {
		RXBUF,
		TXBUF
	};

	int32_t _txbuffer[TX_BUF_SIZE];
	int32_t _rxbuffer[RX_BUF_SIZE];
	uint8_t _board_addr = 0;
	const int32_t _flush_val = 0;
	const int _poll = 500;
	bool startFlag = false;
	uint32_t _workTime_mins = 0;
	int32_t _startkey = 0;
	uint32_t _dataUpdatePrd = 1000UL;		//период обновления значений
	uint32_t _statisUpdatePrd = 60000UL;	//период обновления статистики

	void flush(BufferType type);
	bool pollForDataRx();
	uint8_t getStatisRaw(int32_t* arr, size_t size = 12);
	uint8_t getDataRaw(int32_t* arr, size_t size = 5);
	static String errorsToStr(const int32_t errors);
	static String getWorkTime(const uint32_t mins);
	
public:
	Board();
	Board(const uint8_t addr);
	bool 	attach(const uint8_t addr);								//подключить плату (указать адрес)
	bool 	isOnline();												//проверить, онлайн ли плата
	uint8_t getAddress();											//получить адрес платы		
	void 	setAddress(const uint8_t addr);							//установить адрес плате
	uint8_t 	getData(int32_t* arr, size_t size = 5);				//получить данные с платы
	uint8_t 	getTrimmers(int32_t* arr, size_t size = 8);			//получить триммеры с платы
	uint8_t 	getStatis(int32_t* arr, size_t size = 12);			//получить статистику
	uint8_t 	sendTrimmers(int32_t* arr, size_t size = 8);		//отправить триммеры
	uint8_t 	sendBSets(int32_t* arr, size_t size = 8);			//отправить настройки
	uint8_t		reboot();									//перезагрузить плату
	uint8_t 	toggleRegulation();							//вкл/откл регуляцию напряжения
	uint8_t 	setStartKey();								//установить стартовый ключ
	void 		setWorkTime(const uint32_t mins);
	uint32_t 	getWorkTime();
	void 	getDataStr(String& out);
	void 	getStatisStr(String& out);
	void 	tick();
	void 	detach();

	~Board();
};



