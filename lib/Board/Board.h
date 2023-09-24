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

#define RX_BUF_SIZE							100
#define TX_BUF_SIZE							100

struct data {
	int16_t inputVoltage;
	int16_t outputVoltage;
	float 	outputCurrent;
	float 	outputPower;
	float 	cosfi;
	uint32_t events;
	uint8_t structSize;
	uint8_t* buffer = nullptr;
	data() {
		structSize = offsetof(struct data, structSize);
		buffer = new uint8_t[structSize];
		cosfi = 1.0;
	}
	void packData() {
		memcpy(buffer, (uint8_t*)&inputVoltage, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*) &inputVoltage, buffer, structSize);
	}
};

struct stats {
	uint32_t workTimeMins;
	uint32_t boardEvents;
	int16_t outVoltMax;
	int16_t outVoltAvg;
	int16_t outVoltMin;
	int16_t inVoltMax;
	int16_t inVoltAvg;
	int16_t inVoltMin;
	float 	outLoadMax;
	float 	outLoadAvg;
	float	powerMax;
	float 	powerAvg;
	uint8_t structSize;
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
	int16_t ignoreSetsFlag;		//игнорировать настройки с платы (0...1)
	int16_t precision;			//точность/гистерезис (1...6)
	int16_t tuneInVolt;			//подстройка входа (-6...6)
	int16_t tuneOutVolt;		//подстройка выхода (-6...6)
	int16_t targetVolt;			//целевое напряжение (210, 220, 230, 240)
	int16_t relaySet;			//поведение реле (0...2)
	int16_t motorType;			//тип мотора (0...3)
	int16_t transRatio;			//коэффициент трансворматора тока
	uint8_t structSize;

	int16_t motorStartPwr;
	int16_t motorMaxCurr;

	uint8_t *buffer = nullptr;
	mainsets() {
		structSize = offsetof(struct mainsets, structSize); //вычисляем размер структуры
		buffer = new uint8_t[structSize];					//выделяем место под буфер
		ignoreSetsFlag = DEF_IGNORE_SETS_FLG;
		precision = DEF_TRIM_PRECISION;
		tuneInVolt = DEF_TRIM_TUNEIN;
		tuneOutVolt = DEF_TRIM_TUNEOUT;
		targetVolt = DEF_TRIM_TARGETVOLT;
		relaySet = DEF_TRIM_RELSET;
		motorType = DEF_TRIM_MOTTYPE;
		transRatio = DEF_TRIM_TCRATIO;
		motorStartPwr = 100;
		motorMaxCurr = 3000;
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
		minVoltRelative = DEF_VMIN_TERM;
		maxVoltRelative = DEF_VMAX_TERM;
		emergencyTOFF = DEF_EMERG_TIMEOFF;
		emergencyTON = DEF_EMERG_TIMEON;
		motKoef_0 = DEF_MOTOR0_KOEF;
		motKoef_1 = DEF_MOTOR1_KOEF;
		motKoef_2 = DEF_MOTOR2_KOEF;
		motKoef_3 = DEF_MOTOR3_KOEF;
		motorDefPwr = 100;
	}
	void packData() {
		memcpy(buffer, (uint8_t*) &minVoltRelative, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*) &minVoltRelative, buffer, structSize);
	}
};

struct boardData
{
	data mainData;
	stats mainStats;
	mainsets mainSets;
	addsets addSets;
};

class Board
{
private:
	enum BufferType {
		RXBUF,
		TXBUF
	};
	
	int8_t _txbuffer[TX_BUF_SIZE];
	int8_t _rxbuffer[RX_BUF_SIZE];
	uint8_t _board_addr = 0;
	const int32_t _flush_val = 0;
	const int _poll = 500;
	bool startFlag = false;
	int32_t _workTime_mins = 0;
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
	int32_t 	getWorkTime();
	void 	getDataStr(String& out);
	void 	getStatisStr(String& out);
	void 	tick();
	void 	detach();
	~Board();
	boardData bdata;


};



