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
#include <map>
#include <nlohmann/json.hpp>


#define MAX						0
#define AVG						1
#define MIN						2

#define HEADER_DATA			0x10
#define HEADER_STATS		0x20
#define HEADER_MSETS		0x30
#define HEADER_ASETS		0x40
#define HEADER_SWITCH		0x50

#define XFER_WRITE	0x04
#define XFER_READ	0x03

#define RX_BUF_SIZE							70
#define TX_BUF_SIZE							70

#define SW_OUTSIGN	0
#define SW_REBOOT	1
#define SW_REGDIS	2
#define SW_RSTST	3
#define SW_TRANSIT	4


struct data {	//20
	volatile float Uin = 0;
	volatile float Uout = 0;
	volatile float Current = 0;
	volatile float Power = 0;
	int Events = 0;
	uint8_t structSize;
	uint8_t* buffer = nullptr;
	data() {
        structSize = offsetof(struct data, structSize);
	    buffer = new uint8_t[structSize];
    }
	void unpackData() {
        memcpy((uint8_t*)&Uin, buffer, structSize);
    }
	void packData() {
        memcpy(buffer, (uint8_t*)&Uin, structSize);
    }
};

struct stats {	//56
	int FlashCtrl;					//контрольный код
	float Uin[3] = { 0, 0, 300 };	//max,avg,min
	float Uout[3] = { 0, 0, 300 };
	float Current[3] = { 0, 0, 0 };
	float Power[3] = { 0, 0, 0 };
	int WorkTimeMins = 0;
	int Events = 0;
	uint8_t structSize;
	uint8_t* buffer = nullptr;
	stats() {
        structSize = offsetof(struct stats, structSize) - sizeof(FlashCtrl);
	    buffer = new uint8_t[structSize];
    }
	void unpackData() {
        memcpy((uint8_t*)Uin, buffer, structSize);
    }
	void packData() {
        memcpy(buffer, (uint8_t*)Uin, structSize);
    }

};

struct mainsets {	//32
	int16_t  FlashCtrl;				//контрольный код

	int16_t Liter = 78;				//0 буква платы
    int16_t IgnoreSetsFlag = 1;		//1 игнорировать настройки с платы (0...1)
	int16_t EnableTransit = 0;		//2 транзит при перегрузке
	int16_t MinVolt = 170;			//3 мин напряжение
	int16_t MaxVolt = 250;			//4 макс напряжение
	int16_t Hysteresis = 5;			//5 точность/гистерезис (1...11)
	int16_t Target = 220;			//6 целевое напряжение (210...240)
	int16_t TuneInVolt = 0;			//7 подстройка входа (-6...6)
	int16_t TuneOutVolt = 0;		//8 подстройка выхода (-6...6)
	int16_t TcRatioIndx = 5;		//9 коэффициент трансворматора тока (0...6) смотри addSets
	int16_t MotorType = 1;			//10 тип мотора (1...4) (0 - служебный)
	int16_t EmergencyTON = 2000;	//11 время включения после аварии
	int16_t EmergencyTOFF = 500;	//12 время аварийного отключения
	int16_t MaxCurrent = 35;		//13 макс ток платы
	int16_t CurrClbrtValue = 0;		//14 знач/100 - величина калибровочного тока
	int16_t	CurrClbrtKoeff = 100;	//15 знач/100 - калибровочный коэффициент

	uint8_t structSize;
	uint8_t *buffer = nullptr;
	mainsets() {
        structSize = offsetof(struct mainsets, structSize) - sizeof(FlashCtrl); //вычисляем размер структуры
	    buffer = new uint8_t[structSize];	
    }
	void packData() {
        memcpy(buffer, (uint8_t*)&Liter, structSize);
    }
	void unpackData() {
        memcpy((uint8_t*)&Liter, buffer, structSize);
    }
};

struct addsets {
	int16_t password = 1234;
	int16_t motKoefsList[5] = {0,30,60,90,120};			//(0 - служебный)коэффициент мощности мотора в % от motorDefPwr
	int16_t motorMaxCurrentList[4] = {3000, 4000, 5000, 6000};
	int16_t TcRatioList[7] = {25,40,50,60,80,100,150};		//список коэффициентов трансов
	int32_t SerialNumber[2] = {123456789, 123456};
	
	uint8_t structSize;
	uint8_t *buffer = nullptr;
	uint8_t Switches[8] = {0,0,0,0,0,0,0,0};
	
	addsets() {
		structSize = offsetof(struct addsets, structSize); //вычисляем размер структуры
		buffer = new uint8_t[structSize];//выделяем место под буфер
	}
	void packData() {
		memcpy(buffer, (uint8_t*)&password, structSize);
	}
	void unpackData() {
		memcpy((uint8_t*)&password, buffer, structSize);
	}
};

struct mqttDataMetrics {
	const char* Names[17] = {
		"Uin" , "Uout" , "I" , "P" , 
		"Uin_avg" , "Uout_avg" , "I_avg" , "P_avg" ,
		"Uin_max" , "Uout_max" , "I_max" , "P_max" ,
		"Uin_min" , "Uout_min" , "I_min" , "P_min"
		"work_h"
	};
	float Act[17] = {-1};
	float Min[4] = {-1};
	float Max[4] = {-1};
	void getMinMax(bool set_zero) {
		static uint32_t tmr = 0;
		if (millis() -  tmr > 70000) {
			set_zero = true;
		}
		if (set_zero) {
			memset(Min, 0x46, 16);
			memset(Max, 0, 16);
			tmr = millis();
			return;
		}
		for (uint8_t i = 0; i < 4; i++) {
			Min[i] = (Act[i] < Min[i] ? Act[i] : Min[i]);
			Max[i] = (Act[i] > Max[i] ? Act[i] : Max[i]);
		}
	}
	void Sync(data Data, stats Stat) {
		Act[0] = Data.Uin; Act[1] = Data.Uout; Act[2] = Data.Current; Act[3] = Data.Power;
		Act[4] = Stat.Uin[AVG]; Act[5] = Stat.Uout[AVG]; Act[6] = Stat.Current[AVG]; Act[7] = Stat.Power[AVG]; 
		Act[8] = Stat.Uin[MAX]; Act[9] = Stat.Uout[MAX]; Act[10] = Stat.Current[MAX]; Act[11] = Stat.Power[MAX]; 
		Act[12] = Stat.Uin[MIN]; Act[13] = Stat.Uout[MIN]; Act[14] = Stat.Current[MIN]; Act[15] = Stat.Power[MIN]; 
		Act[16] = Stat.WorkTimeMins/60;
	}
};

struct mqttDataSettings {
	const char* Names[17] = {
		"Transit" , "Uout_minoff" , "Uout_maxoff" , "Accuracy",
		"Target"  , "Uin_tune"    , "Uout_tune"   , "T_5",
		"M_type"  , "Time_on"     , "Time_off"    ,"Password", 
		"SN_1"    , "SN_2"        ,"Rst_max"      , "Save", 
		"Outsignal"
	};
	int16_t Values[17] = {-1};
	void Sync(mainsets Sets, addsets Add) {
		Values[0] = Sets.EnableTransit; Values[1] = Sets.MinVolt; Values[2] = Sets.MaxVolt; Values[3] = Sets.Hysteresis;
		Values[4] = Sets.Target; Values[5] = Sets.TuneInVolt; Values[6] = Sets.TuneOutVolt; Values[7] = Add.TcRatioList[Sets.TcRatioIndx];
		Values[8] = Sets.MotorType; Values[9] = Sets.EmergencyTON; Values[10] = Sets.EmergencyTOFF; Values[11] = Add.password;
		Values[12] = Add.SerialNumber[0]; Values[13] = Add.SerialNumber[1]; Values[14] = 0; Values[15] = 0;
		Values[16] = Add.Switches[SW_OUTSIGN];
	};
}





// =======================================================================================//


class Board
{
private:
	enum EventsFormat {
		EVENTS_FULL,
		EVENTS_SHORT
	};
	uint8_t _board_addr = 0;
	static const int _poll = 200;
	bool startFlag = false;
	uint32_t _dataUpdatePrd = 1000UL;		//период обновления значений
	uint32_t _statisUpdatePrd = 60000UL;	//период обновления статистики
	bool _active = false;
	uint8_t _disconnected = 0;
	String actTime;
	std::map<int, std::string> gEventsList;
	void 	validate();
	String errorsToStr(const int32_t errors, EventsFormat f);
	String getWorkTime(const uint32_t mins);
	
	
public:
	Board() {attach(0, 'N');}
	Board(const uint8_t addr) {attach(addr, 'N');};
	static int8_t StartI2C();		
	static int8_t StopI2C();
	bool 		attach(const uint8_t addr, const char Liter);								//подключить плату (указать адрес)
	static uint8_t isBoard(const uint8_t addr);
	static uint8_t scanBoards(std::vector<Board>&brd, const uint8_t max);
	bool 		isOnline();												//проверить, онлайн ли плата
	bool 		isAnswer();
	uint8_t 	getAddress() {return _board_addr;};						//получить адрес платы		

	uint8_t 	getData();		
	//=============================Получение данных с платы=========================================//										
	float 		readDataRaw(const uint8_t val_addr = 0, uint8_t vals_cnt = 5);		//получить данные с платы
	float 		readStatsRaw(const uint8_t val_addr = 0, uint8_t vals_cnt = 14);		//получить статистику с платы
	int16_t 	readMainSets(const uint8_t val_addr = 0, uint8_t vals_cnt = 16);		//получить настройки с платы
	int16_t 	readAddSets(const uint8_t val_addr = 0, uint8_t vals_cnt = 21);		//получить настройки с платы
	uint8_t 	readSwitches(const uint8_t val_addr = 0, uint8_t vals_cnt = 8);
	//=============================Отправка данных на плату========================================//	
	uint8_t 	sendMainSets(const uint8_t val_addr = 0, uint8_t vals_cnt = 14, int16_t value = INT16_MIN);	
	uint8_t 	sendAddSets( const uint8_t val_addr = 0, uint8_t vals_cnt = 21, int16_t value = INT16_MIN);
	uint8_t 	sendSwitches(const uint8_t val_addr = 0, uint8_t vals_cnt = 8, uint8_t value = 255);
	
	bool 		readAll();
	bool 		writeAll();
	void 		getDataStr(String & result);
	void 		getStatisStr(String & result);
	void 		getJsonData(std::string & result, uint8_t mode);
	uint8_t 	setJsonData(std::string input);
	void 		getMotKoefsList(String &result, bool typeNumber);
	void 		setMotKoefsList(String str);
	void	 	getTcRatioList(String &result);
	uint8_t 	getNextActiveAlarm(std::string& result, uint32_t alarms);
	void 		setLiteral(char lit);
	char 		getLiteral();
	float 		getData(std::string request);
	void 		tick(const String time);
	void 		detach();
	~Board();
	
	data mainData;
	stats mainStats;
	mainsets mainSets;
	addsets addSets;
	mqttDataMetrics Metrics;
	float CurrClbrtKoeff = 1.0;
	float CurrClbrtValue = 0;
};



