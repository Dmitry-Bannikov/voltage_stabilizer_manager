/*
*Библиотека для общения с дисплеем
* Методы библиотеки:
*

*/


#pragma once
#include <Arduino.h>
#include <cstring>

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
	bool _inited = false;
	uint8_t rxbuf[255];
	uint8_t txbuf[255];
	
	uint8_t header1 = 0x5A;
	uint8_t header2 = 0xA5;
	uint8_t pointer = 0;
	bool pollForDataRx();

	template<typename T>
	uint8_t* convertData(T value) {
		uint8_t *bytes = new uint8_t[sizeof(T)];
		std::memcpy(bytes, &value, sizeof(T));
		for (size_t i = 0; i < sizeof(T) / 2; i++) {
			std::swap(bytes[i], bytes[sizeof(T) - 1 - i]);
		}
		return bytes;
	}
	typedef void (*CallbackFunction)(void);
	//CallbackFunction onDataReceived;
	HardwareSerial* userSerial = nullptr;
public:

	Display() {};
	~Display();

	void begin(HardwareSerial *Ser, CallbackFunction callback);

	template<typename T>
	void addNewValue(T value) {
		if (!_inited) return;
		uint8_t* bytes = convertData(value);
		uint8_t size = sizeof(T);
		memcpy(txbuf + 6 + pointer, bytes, size);
		pointer += size;
	}

	template<typename T>
	void writeValue(const uint16_t addr, T value) {
		if (!_inited) return;
		uint8_t tx_buf[255];
		uint8_t* addrPtr = convertData(addr);
		uint8_t* dataPtr = convertData(value);
		tx_buf[0] = header1;
		tx_buf[1] = header2;
		tx_buf[2] = sizeof(T) + 3;
		tx_buf[3] = 0x82;
		memcpy(tx_buf + 4, addrPtr, 2);
		memcpy(tx_buf + 6, dataPtr, sizeof(T));
		userSerial->write(tx_buf, sizeof(T) + 6);
	}
	void writeAddedValues(const uint16_t addr);
	void Test(int16_t value);
	void requestFrom(const uint16_t addr, const uint8_t words);
	void tick();
	void waitUntillTx() {userSerial->flush();}
};

