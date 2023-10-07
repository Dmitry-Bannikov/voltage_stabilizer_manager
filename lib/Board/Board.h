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
	int16_t ignoreSetsFlag;		//игнорировать настройки с платы (0...1)
	int16_t precision;			//точность/гистерезис (1...6)
	int16_t tuneInVolt;			//подстройка входа (-6...6)
	int16_t tuneOutVolt;		//подстройка выхода (-6...6)
	int16_t targetVolt;			//целевое напряжение (210, 220, 230, 240)
	int16_t relaySet;			//поведение реле (0...2)
	int16_t motorType;			//тип мотора (0...3)
	int16_t transRatio;			//коэффициент трансворматора тока
	char 	liter;
	uint8_t structSize;

	int16_t motorStartPwr;
	int16_t motorMaxCurr;

	uint8_t *buffer = nullptr;
	mainsets() {
		structSize = offsetof(struct mainsets, structSize); //вычисляем размер структуры
		buffer = new uint8_t[structSize];					//выделяем место под буфер
		ignoreSetsFlag = 0;
		precision = 3;
		tuneInVolt = 0;
		tuneOutVolt = 0;
		targetVolt = 220;
		relaySet = 1;
		motorType = 1;
		transRatio = 60;
		motorStartPwr = 100;
		motorMaxCurr = 3000;
		liter = '\0';
	}
	void packData() {
		memcpy(buffer, (uint8_t*) &ignoreSetsFlag, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*) &ignoreSetsFlag, buffer, structSize);
	}
};

struct addsets {
	int16_t minVoltRelative;	//мин напряжение относительно целевого
	int16_t maxVoltRelative;	//макс напряжение относительно целевого
	uint16_t emergencyTOFF;		//время аварийного отключения
	uint16_t emergencyTON;		//время включения после аварии
	uint16_t motKoef_0;			//коэффициент мощности мотора в %
	uint16_t motKoef_1;
	uint16_t motKoef_2;
	uint16_t motKoef_3;
	uint8_t structSize;
	int16_t motorDefPwr;
	uint8_t *buffer = nullptr;
	addsets() {
		structSize = offsetof(struct addsets, structSize); //вычисляем размер структуры
		buffer = new uint8_t[structSize];//выделяем место под буфер
		minVoltRelative = -22;
		maxVoltRelative = 22;
		emergencyTOFF = 500;
		emergencyTON = 2000;
		motKoef_0 = 40;
		motKoef_1 = 100;
		motKoef_2 = 150;
		motKoef_3 = 200;
		motorDefPwr = 100;
		packData();
	}
	void packData() {
		memcpy(buffer, (uint8_t*) &minVoltRelative, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*) &minVoltRelative, buffer, structSize);
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
	enum BufferType {
		RXBUF,
		TXBUF
	};
	EEManager memSets;

	uint8_t _txbuffer[100];
	uint8_t _rxbuffer[100];
	uint8_t _memsets_buf[33] = {0};
	uint8_t _board_addr = 0;
	static const int _poll = 500;
	bool startFlag = false;
	uint32_t _dataUpdatePrd = 1000UL;		//период обновления значений
	uint32_t _statisUpdatePrd = 60000UL;	//период обновления статистики
	bool _active = false;
	uint8_t _disconnected = 0;
	uint16_t _memoryAddr = 100;
	uint8_t _memoryKey = 10;

	void flush(BufferType type);
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
	void 		setLiteral(String lit);
	void 		tick();
	void 		detach();
	~Board();

	data mainData;
	stats mainStats;
	mainsets mainSets;
	addsets addSets;
	

};



