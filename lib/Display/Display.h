/*
*Библиотека для общения с дисплеем
* Методы библиотеки:
*

*/


#pragma once
#include <Arduino.h>

struct dwinsets {
	// auto
	uint8_t* autoWdata;
	uint16_t autoWaddr;
	uint8_t autoWsize;

	uint8_t* autoRdata;
	uint16_t autoRaddr;

	uint8_t inited;
	uint8_t readComplete;

};

struct dwindata {
	uint8_t header1;			//5A
	uint8_t header2;			//A5
	uint8_t byte_cnt;			//{кол-во данных}
	uint8_t command;			//0x82 -write/0x83 - read
	uint16_t address;			//0x1000 {address 2bytes}
	uint8_t data[250];			//data
	//------------------//
	uint8_t buffer[256];
	uint8_t dataPointer;
	uint8_t wordsToRead;
	dwindata() {
		header1 = 0x5A;
		header2 = 0xA5;
		byte_cnt = 0;
		command = 0;
		address = 0;
		dataPointer = 0;
		wordsToRead = 0;
	}
	void packData() {
		header1 = 0x5A;
		header2 = 0xA5;
		memcpy(buffer, (uint8_t*)&header1, dataPointer + 6);
	}
	void unpackData() {
		memcpy(buffer, (uint8_t*)&header1, 256);
	}
};

class Display
{
private:
	
	uint8_t rxbuf[255];
	uint8_t txbuf[255];
	HardwareSerial* userSerial;
	uint8_t header1 = 0x5A;
	uint8_t header2 = 0xA5;
	uint8_t pointer = 0;

	dwindata dwinDataTx;
	dwindata dwinDataRx;
	

	bool pollForDataRx();


public:
	Display();
	Display(HardwareSerial *Serial) {
		userSerial = Serial;
		userSerial->begin(115200);
	}
	~Display();
	template<typename T>
	void addNewValue(T value) {
		
	}



	void Test(int16_t value);
	void tick();
	void displayVoltage(int32_t* voltages);
	void displayCurrent(float* currents);
	void displayPower(float* powers);
	void displayEvents(int32_t* events);
	int32_t* readTrimmers();
	int32_t* readSettings();
	

};

