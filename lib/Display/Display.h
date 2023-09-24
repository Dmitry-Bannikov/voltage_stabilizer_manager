/*
*Библиотека для общения с дисплеем
* Методы библиотеки:
*

*/


#pragma once
#include <Arduino.h>


class Display
{
private:
	
	uint8_t* rxbuf;
	uint8_t* txbuf;
	const int16_t nullTerm = -256;
	HardwareSerial* userSerial = nullptr;

	uint8_t getDataFromAddr(uint16_t addr, int16_t* out, uint8_t length);
	void writeValue(uint16_t addr, int16_t value);
	void writeValue(uint16_t addr, float value);
	void writeValuesSeq(uint16_t startAddr, int16_t* values, uint8_t dsize);
	void writeValuesSeq(uint16_t startAddr, float* values, uint8_t dsize);
	uint8_t readValue(uint16_t addr, int16_t& value);
	int16_t* readValuesSeq(uint16_t startAddr, void (*callback)(), uint8_t dsize);

	bool pollForDataRx();


public:
	void tick();
	void displayVoltage(int32_t* voltages);
	void displayCurrent(float* currents);
	void displayPower(float* powers);
	void displayEvents(int32_t* events);
	int32_t* readTrimmers();
	int32_t* readSettings();
	Display();
	Display(uint8_t SerialNumber);
	~Display();
};

Display::Display()
{
	readData.read_addr = 0;
	readData.length = 0;
	readData.readCallback = nullptr;
	rxbuf = nullptr;
	txbuf = nullptr;
	userSerial = nullptr;
	userSerial = &Serial1;
}

Display::Display(uint8_t SerialNumber)
{
	readData.read_addr = 0;
	readData.length = 0;
	readData.readCallback = nullptr;
	rxbuf = nullptr;
	txbuf = nullptr;
	userSerial = nullptr;
    switch (SerialNumber) {
        case 0:
          	userSerial = &Serial;
          	break;
        case 1:
          	userSerial = &Serial1;
          	break;
        case 2:
          	userSerial = &Serial2;
          	break;
		default: 
			userSerial = &Serial1;
          // Добавьте другие ваши варианты по мере необходимости
    }
	userSerial->begin(9600, SERIAL_8N1);
}

Display::~Display()
{
	delete(rxbuf);
	delete(txbuf);
	userSerial->end();
	delete(userSerial);
}
