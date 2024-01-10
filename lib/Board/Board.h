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
#include <vector>
#include <string>
#include <sstream>

#define ACT						0
#define MAX						1
#define AVG						2
#define MIN						3

#define I2C_DATA_START						0x30	//начало данных
#define I2C_MAINSETS_START					0x35	//начало главных настроек
#define I2C_ADDSETS_START					0x40	//начало доп настроек
#define I2C_SWITCHES_START					0x46	//переключатели

#define I2C_REQUEST_ISBOARD					0x20	//это плата?
#define I2C_REQUEST_MAINSETS				0x21	//запрос главных настроек
#define I2C_REQUEST_ADDSETS					0x22	//запрос доп настроек
#define I2C_REQUEST_DATA					0x23	//запрос данных
#define I2C_REQUEST_SWITCHES				0x25	//запрос состояния пере

#define RX_BUF_SIZE							66
#define TX_BUF_SIZE							66

#define SW_OUTSIGN	0
#define SW_REBOOT	1
#define SW_REGDIS	2
#define SW_RSTST	3
#define SW_TRANSIT	4


struct data {
	int16_t Uin[4] 		= {0,0,0,400};	//act,max,avg,min
	int16_t Uout[4] 	= {0,0,0,400};
	float 	Current[4] 	= {0,0,0,0};
	float	Power[4] 	= {0,0,0,0};
	float 	CosFi = 1;
	uint32_t WorkTimeMins;
	uint32_t Events[2] = {0, 0};
	uint8_t FlashCtrl 	= 36;
	uint8_t structSize;
	uint8_t* buffer = nullptr;
	String StrData;
	String StrStat;
	data() {
		structSize = offsetof(struct data, structSize);
		buffer = new uint8_t[structSize];
	}
	void packData() {
		memcpy(buffer, (uint8_t*)Uin, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*)Uin, buffer, structSize);
	}
};

//----------------------BOARD MAIN SETS----------------------//
struct sets {
	int8_t ignoreSetsFlag = 0;		//игнорировать настройки с платы (0...1)
	int8_t enableTransit = 1;		//транзит при перегрузке
	int8_t precision = 3;			//точность/гистерезис (1...6)
	int8_t tuneInVolt = 0;			//подстройка входа (-10...10)
	int8_t tuneOutVolt = 0;		    //подстройка выхода (-10...10)
	uint8_t Target = 222;			//целевое напряжение (до 255)
	uint8_t motorType = 2;				//тип мотора (1...4)
	uint8_t transRatioIndx = 3;			//коэффициент трансформатора тока (0...6) смотри addSets
	uint8_t i2c_addr = 0;
	char liter = 'N';
	uint8_t	maxCurrent = 30;
	int16_t minVolt = 198;			//мин напряжение
	int16_t maxVolt = 242;			//макс напряжение
	int16_t emergencyTOFF = 500;		//время аварийного отключения
	int16_t emergencyTON = 2000;		//время включения после аварии
	uint8_t FlashCtrl = 32;
	uint8_t structSize;
	uint8_t *buffer = nullptr;
	sets() {
		structSize = offsetof(struct sets, structSize); //вычисляем размер структуры
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
	int16_t motKoefsList[5] = {0,15,90,150,200};				//коэффициент мощности мотора в % от motorDefPwr
	int16_t motorMaxCurrentList[4] = {3000,4000,5000,6000}; //список макс токов на каждый из типов моторов
	int16_t tcRatioList[7] = {25,40,50,60,80,100,150};		//список коэффициентов трансов
	int32_t SerialNumber[2] = {123456789, 123456};			//серийники (до 9 цифр)
	uint8_t structSize;
	uint8_t *buffer = nullptr;
	uint8_t Switches[8] = {0,0,0,0};
	addsets() {
		structSize = offsetof(struct addsets, structSize); //вычисляем размер структуры
		buffer = new uint8_t[structSize];//выделяем место под буфер
		packData();
	}
	void packData() {
		memcpy(buffer, (uint8_t*)motKoefsList, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*)motKoefsList, buffer, structSize);
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
	String PROGMEM gEventsList[32] = {
	"Нет",
	"Блок мотора", 
	"Тревога-1",
	"Тревога-2",
	"<80 В",
	"Недо-напряжение",
	"Перенапряжение",
	"Мин. напряжение",
	"Мак. напряжение",
	"Транзит",
	"Перегрузка",
	"Внеш. сигнал"
	};
	uint8_t _txbuffer[66];
	uint8_t _rxbuffer[66];
	uint8_t _board_addr = 0;
	static const int _poll = 200;
	bool startFlag = false;
	uint32_t _dataUpdatePrd = 1000UL;		//период обновления значений
	uint32_t _statisUpdatePrd = 60000UL;	//период обновления статистики
	bool _active = false;
	uint8_t _disconnected = 0;
	void 	validate();
	String errorsToStr(const int32_t errors, EventsFormat f);
	String getWorkTime(const uint32_t mins);
	
public:
	Board() {attach(0);}
	Board(const uint8_t addr) {attach(addr);};
	static int8_t StartI2C();		
	static int8_t StopI2C();
	bool 		attach(const uint8_t addr);								//подключить плату (указать адрес)
	static uint8_t isBoard(const uint8_t addr);
	static uint8_t scanBoards(std::vector<Board>&brd, const uint8_t max);
	bool 		isOnline();												//проверить, онлайн ли плата
	bool 		isAnswer();
	uint8_t 	getAddress() {return _board_addr;};						//получить адрес платы		

	uint8_t 	getData();												//получить данные с платы
	uint8_t 	getDataRaw();
	uint8_t 	getMainSets();											//получить настройки с платы
	uint8_t 	sendMainSets(uint8_t attempts = 0);						//отправить настройки
	uint8_t 	sendCommand(uint8_t command, uint8_t value);
	uint8_t 	sendCommand(uint8_t* command);
	uint8_t 	sendCommand();
	void 		getDataStr();
	void 		getStatisStr();
	String 		createJsonData(uint8_t mode);
	void 		getMotTypesList(String &result, bool mode);
	void 		setMotKoefsList(String &str);
	void	 	getTcRatioList(String &result);
	void 		setLiteral(char lit);
	char 		getLiteral();
	void 		tick();
	void 		detach();
	~Board();

	data mainData;
	sets mainSets;
	addsets addSets;
	

};



