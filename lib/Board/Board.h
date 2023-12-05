#pragma once
//библиотека общения с платой
//
/*методы библиотеки
*!!!ВАЖНО!!! Ключи и команды должны соответствовать таким же ключам и командам в плате
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
#include <EEManager.h>
#include <vector>
#include <string>
#include <sstream>

#define I2C_DATA_START						0x30
#define I2C_MAINSETS_START					0x35
#define I2C_ADDSETS_START					0x40
#define I2C_STAT_START						0x45
#define I2C_SWITCHES_START					0x46

#define I2C_REQUEST_MAINSETS				0x21
#define I2C_REQUEST_ADDSETS					0x22
#define I2C_REQUEST_DATA					0x23
#define I2C_REQUEST_STAT					0x24
#define I2C_REQUEST_SWITCHES				0x25

#define RX_BUF_SIZE							100
#define TX_BUF_SIZE							100

#define SW_ALARM	0
#define SW_REBOOT	1
#define SW_REGDIS	2
#define SW_SAVE		3
#define SW_RSTST	4



struct data {
	int16_t 	inputVoltage;
	int16_t 	outputVoltage;
	float 		outputCurrent;
	float 		outputPower;
	float 		cosfi;
	uint32_t 	events;
	uint8_t 	structSize;
	String 		Str;
	uint8_t* 	buffer = nullptr;
	data() {
		structSize = offsetof(struct data, structSize);
		buffer = new uint8_t[structSize];
		cosfi = 1.0;
		inputVoltage = 0;
		outputVoltage = 0;
		outputCurrent = 0;
		outputPower = 0;
		events = 0;
	}
	void packData() {
		memcpy(buffer, (uint8_t*)&inputVoltage, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*) &inputVoltage, buffer, structSize);
	}
};

struct stats {
	uint32_t workTimeMins = 0;
	uint32_t boardEvents = 0;
	int16_t inVoltage[3] 	= {0,0,300};	//max,avg,min
	int16_t outVoltage[3] 	= {0,0,300};
	float 	outCurrent[3] 	= {0,0,0};
	float	power[3] 	  	= {0,0,0};

	uint8_t 	structSize;
	String  	Str;
	uint8_t* buffer = nullptr;
	stats() {
		structSize = offsetof(struct stats, structSize);
		buffer = new uint8_t[structSize];
	}
	void packData() {
		memcpy(buffer, (uint8_t*)&workTimeMins, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*) &workTimeMins, buffer, structSize);
	}
};

//----------------------BOARD MAIN SETS----------------------//
struct mainsets {
	int8_t ignoreSetsFlag = 0;		//игнорировать настройки с платы (0...1)
	int8_t precision = 3;			//точность/гистерезис (1...6)
	int8_t tuneInVolt = 0;			//подстройка входа (-6...6)
	int8_t tuneOutVolt = 0;		    //подстройка выхода (-6...6)
	uint8_t targetVoltage = 222;		//целевое напряжение (0...3) смотри addSets
	uint8_t motorType = 1;				//тип мотора (0...3)
	uint8_t transRatioIndx = 3;			//коэффициент трансворматора тока (0...6) смотри addSets
	uint8_t	maxCurrent = 30;
	char liter = 'N';
	uint8_t structSize;
	uint8_t *buffer = nullptr;
	mainsets() {
		structSize = offsetof(struct mainsets, structSize); //вычисляем размер структуры
		buffer = new uint8_t[structSize];					//выделяем место под буфер
	}
	void packData() {
		memcpy(buffer, (uint8_t*) &ignoreSetsFlag, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*) &ignoreSetsFlag, buffer, structSize);
	}
};

struct addsets {
	int16_t minVolt = 198;				//мин напряжение
	int16_t maxVolt = 242;				//макс напряжение
	int16_t emergencyTOFF = 500;		//время аварийного отключения
	int16_t emergencyTON = 2000;		//время включения после аварии
	int16_t overloadTransit = 0;		//транзит при перегрузке
	int16_t motKoefsList[4] = {20,90,150,200};			//коэффициент мощности мотора в % от motorDefPwr
	int16_t tcRatioList[6] = {25,40,50,60,80,100};		//список коэффициентов трансов
	int32_t SerialNumber[2] = {0, 0};
	uint8_t structSize;
	uint8_t Switches[8] = {0,0,0,0};
	uint8_t *buffer = nullptr;
	addsets() {
		structSize = offsetof(struct addsets, structSize); //вычисляем размер структуры
		buffer = new uint8_t[structSize];//выделяем место под буфер
		packData();
	}
	void packData() {
		memcpy(buffer, (uint8_t*) &minVolt, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*) &minVolt, buffer, structSize);
	}
};




// =======================================================================================//


class Board
{
private:
	enum EventsFormat {
		EVENTS_FULL,
		EVENTS_SHORT
	};
	String PROGMEM gEventsList[10] = {
	"Нет"
	"Блок мотора", 
	"Макс. напряжение",
	"Мин. напряжение",
	"Транзит",
	"Перенапряжение",
	"Перегрузка",
	"Перегрев",
	"",
	""
	};
	uint8_t _txbuffer[100];
	uint8_t _rxbuffer[100];
	uint8_t _board_addr = 0;
	static const int _poll = 200;
	bool startFlag = false;
	uint32_t _dataUpdatePrd = 1000UL;		//период обновления значений
	uint32_t _statisUpdatePrd = 60000UL;	//период обновления статистики
	bool _active = false;
	uint8_t _disconnected = 0;

	bool pollForDataRx();
	
	String errorsToStr(const int32_t errors, EventsFormat f);
	String getWorkTime(const uint32_t mins);
	
public:
	Board() {attach(0);}
	Board(const uint8_t addr) {attach(addr);};
	bool 		attach(const uint8_t addr);								//подключить плату (указать адрес)
	static bool isBoard(const uint8_t addr);
	static uint8_t scanBoards(std::vector<Board>&brd, const uint8_t max);
	static void waitForReady();
	bool 		isOnline();												//проверить, онлайн ли плата
	bool 		isAnswer();
	uint8_t 	getAddress() {return _board_addr;};							//получить адрес платы		

	uint8_t 	getData();												//получить данные с платы
	uint8_t 	getDataRaw();
	uint8_t 	getMainSets();											//получить настройки с платы
	uint8_t 	sendMainSets();											//отправить настройки
	uint8_t 	sendCommand(uint8_t command, uint8_t value);
	uint8_t 	sendCommand(uint8_t* command);
	void 		getDataStr();
	void 		getStatisStr();
	String 		getLiteral();
	String		getMotKoefList();
	String	 	getTcRatioList();
	void 		setLiteral(String lit);
	void 		setLiteral(char lit);
	void 		tick();
	void 		detach();
	~Board();

	data mainData;
	stats mainStats;
	mainsets mainSets;
	addsets addSets;
	

};



