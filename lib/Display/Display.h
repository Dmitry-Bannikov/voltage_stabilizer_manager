/*
*Библиотека для общения с дисплеем
* Методы библиотеки:
*

*/


#pragma once
#include <Arduino.h>
#include <cstring>


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
	void convertData(T value, uint8_t* res) {
		std::memcpy(res, &value, sizeof(T));
		for (size_t i = 0; i < sizeof(T) / 2; i++) {
			std::swap(res[i], res[sizeof(T) - 1 - i]);
		}
	}
	typedef void (*CallbackFunction)(void);
	HardwareSerial* userSerial = nullptr;
public:

	Display() {};
	~Display();

	void begin(HardwareSerial *Ser, CallbackFunction callback);

	template<typename T>
	void addNewValue(T value) {
		if (!_inited) return;
		uint8_t size = sizeof(T);
		uint8_t* bytes = new uint8_t[size];
		convertData(value, bytes);
		if (pointer + size >= sizeof(txbuf)) {
			//log_e("Cannot put new value");
			pointer = 0;
		}
		memcpy(txbuf + 6 + pointer, bytes, size);
		pointer += size;
		delete(bytes);
	}

	template<typename T>
	void writeValue(const uint16_t addr, T value) {
		if (!_inited) return;
		uint8_t tx_buf[255];
		uint8_t* addrPtr = new uint8_t[sizeof(addr)];
		convertData(addr, addrPtr);
		uint8_t* dataPtr = new uint8_t[sizeof(T)];
		convertData(value, dataPtr);
		tx_buf[0] = header1;
		tx_buf[1] = header2;
		tx_buf[2] = sizeof(T) + 3;
		tx_buf[3] = 0x82;
		memcpy(tx_buf + 4, addrPtr, 2);
		memcpy(tx_buf + 6, dataPtr, sizeof(T));
		userSerial->write(tx_buf, sizeof(T) + 6);
		delete(addrPtr);
		delete(dataPtr);
	}
	void writeAddedValues(const uint16_t addr);
	void Test(int16_t value);
	void requestFrom(const uint16_t addr, const uint8_t words);
	void tick();
	void waitUntillTx() {userSerial->flush();}
	uint16_t parseAddress(const uint8_t* buffer);
	void sendRawData(const uint16_t addr, const uint8_t* data, uint8_t size);

};

