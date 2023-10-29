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

#define I2C_REQUEST_MAINSETS				0x21
#define I2C_REQUEST_ADDSETS					0x22
#define I2C_REQUEST_DATA					0x23
#define I2C_REQUEST_STAT					0x24
#define I2C_REQUEST_REBOOT					0x25
#define I2C_REQUEST_NOREG					0x26
#define I2C_REQUEST_SMARTCONNECT			0x27

#define RX_BUF_SIZE							100
#define TX_BUF_SIZE							100




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
	uint32_t 	workTimeMins;
	uint32_t 	boardEvents;
	int16_t 	outVoltMax;
	int16_t 	outVoltAvg;
	int16_t 	outVoltMin;
	int16_t 	inVoltMax;
	int16_t 	inVoltAvg;
	int16_t 	inVoltMin;
	float 		outLoadMax;
	float 		outLoadAvg;
	float		powerMax;
	float 		powerAvg;
	uint8_t 	structSize;
	String  	Str;
	uint8_t* buffer = nullptr;
	stats() {
		structSize = offsetof(struct stats, structSize);
		buffer = new uint8_t[structSize];
		workTimeMins = 0;
		boardEvents = 0;
		inVoltAvg = 0;
		inVoltMax = 0;
		inVoltMin = 0;
		outVoltAvg = 0;
		outVoltMax = 0;
		outVoltMin = 0;
		outLoadMax = 0;
		outLoadAvg = 0;
		powerAvg = 0;
		powerMax = 0;
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
	int8_t tuneOutVolt = 0;			//подстройка выхода (-6...6)
	int8_t targetVoltIndx = 1;			//целевое напряжение (0...3) смотри addSets
	int8_t displayType = 1;				//выбор дисплея (1 - двин, 2, маленький, 0 -откл)
	int8_t motorType = 1;				//тип мотора (0...3)
	int8_t transRatioIndx = 2;			//коэффициент трансворматора тока (0...6) смотри addSets
	int8_t	maxCurrentIndx = 1;      	//максимальный ток (0...4) смотри addSets
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
	int16_t minVolt = 242;								//мин напряжение
	int16_t maxVolt = 198;								//макс напряжение
	int16_t emergencyTOFF = 500;						//время аварийного отключения
	int16_t emergencyTON = 2000;						//время включения после аварии
	int16_t overloadTransit = 1;						//транзит при перегрузке
	int16_t motKoefsList[4] = {20,100,150,200};			//коэффициент мощности мотора в % от motorDefPwr
	int16_t targetVotageList[4] = {210,220,230,240};	//список выходных напряжений
	int16_t maxCurrentList[4] = {25,30,35,40};			//список максимальных токов выхода
	int16_t tcRatioList[6] = {25,40,50,60,80,100};		//список коэффициентов трансов
	uint8_t structSize;
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
	enum Error_type {
		ERR_NO = 0,
		ERR_INIT,
		ERR_CONNECT,
		ERR_TIMEOUT,
		ERR_STARTCODE
	};
	enum EventsFormat {
		EVENTS_FULL,
		EVENTS_SHORT
	};
	EEManager memSets;
	std::string PROGMEM gEventsList[10] = {
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
	uint8_t _memsets_buf[60] = {0};//должен быть размером больше чем 2 структуры настроек
	uint8_t _board_addr = 0;
	static const int _poll = 500;
	bool startFlag = false;
	uint32_t _dataUpdatePrd = 1000UL;		//период обновления значений
	uint32_t _statisUpdatePrd = 60000UL;	//период обновления статистики
	bool _active = false;
	uint8_t _disconnected = 0;
	uint16_t _memoryAddr = 100;
	uint8_t _memoryKey = 20;

	bool pollForDataRx();
	uint8_t getStatisRaw();
	uint8_t getDataRaw();
	String errorsToStr(const int32_t errors, EventsFormat f);
	String getWorkTime(const uint32_t mins);
	
public:
	Board() : memSets(_memsets_buf,20000) {attach(0);}
	Board(const uint8_t addr)  : memSets(_memsets_buf,20000) {attach(addr);};
	bool 		attach(const uint8_t addr);								//подключить плату (указать адрес)
	static bool isBoard(const uint8_t addr);
	static uint8_t scanBoards(std::vector<Board>&brd, const uint8_t max);
	bool 		isOnline();												//проверить, онлайн ли плата
	uint8_t 	getAddress() {return _board_addr;};							//получить адрес платы		
	bool 		setAddress(const uint8_t addr) ;							//установить адрес плате
	void 		setMemAddr(uint16_t memaddr) { _memoryAddr = memaddr;}
	uint16_t 	getEndMemAddr() { return _memoryAddr + memSets.blockSize(); };
	void 		setMemoryKey(const uint8_t key) { _memoryKey = key; }

	uint8_t 	saveSettings();
	uint8_t 	readSettings();
	uint8_t 	getData();												//получить данные с платы
	uint8_t 	getMainSets();											//получить осн настройки с платы
	uint8_t 	getAddSets();											//получить доп настройки с платы
	uint8_t 	getStatis();											//получить статистику
	uint8_t 	sendMainSets();											//отправить триммеры
	uint8_t 	sendAddSets();											//отправить настройки
	uint8_t		reboot();												//перезагрузить плату
	uint8_t 	toggleRegulation();										//вкл/откл регуляцию напряжения
	void 		getDataStr();
	void 		getStatisStr();
	String 		getLiteral();
	String		getMotKoefList();
	String	 	getMaxCurrList();
	String	 	getTargetVList();
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



