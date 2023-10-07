#include <Display.h>


void Display::writeValue(uint16_t addr, int16_t value) {
	uint8_t size = 1 + sizeof(addr) + sizeof(value);
	uint8_t totalSize = size + 3;
	txbuf = new uint8_t[totalSize];
	uint8_t header[4] = {0x5A, 0xA5, size, 0x82};
	memcpy(txbuf, header, 4);
	memcpy(txbuf + 4, &addr, 2);
	memcpy(txbuf + 6, &value, 4);
	userSerial->write(txbuf, totalSize);
	delete(txbuf);
}

void Display::writeValue(uint16_t addr, float value) {
	uint8_t size = 1 + sizeof(addr) + sizeof(value);
	uint8_t totalSize = size + 3;
	txbuf = new uint8_t[totalSize];
	uint8_t header[4] = {0x5A, 0xA5, size, 0x82};
	memcpy(txbuf, header, 4);
	memcpy(txbuf + 4, &addr, 2);
	memcpy(txbuf + 6, &value, 4);
	userSerial->write(txbuf, totalSize);
	delete(txbuf);
}

void Display::writeValuesSeq(uint16_t startAddr, int16_t* values, uint8_t dsize) {
	uint8_t size = 1 + sizeof(startAddr) + dsize*sizeof(*values);
	uint8_t totalSize = size + 3;
	txbuf = new uint8_t[totalSize];
	uint8_t header[4] = {0x5A, 0xA5, size, 0x82};
	memcpy(txbuf, header, 4);
	memcpy(txbuf + 4, &startAddr, 2);
	memcpy(txbuf + 6, values, totalSize - 4);
	userSerial->write(txbuf, totalSize);
	delete(txbuf);
}

void Display::writeValuesSeq(uint16_t startAddr, float* values, uint8_t dsize) {
	uint8_t size = 1 + sizeof(startAddr) + dsize*sizeof(*values);
	uint8_t totalSize = size + 3;
	txbuf = new uint8_t[totalSize];
	uint8_t header[4] = {0x5A, 0xA5, size, 0x82};
	memcpy(txbuf, header, 4);
	memcpy(txbuf + 4, &startAddr, 2);
	memcpy(txbuf + 6, values, totalSize - 4);
	userSerial->write(txbuf, totalSize);
	delete(txbuf);
}

uint8_t Display::readValue(uint16_t addr, int16_t& value) {
	txbuf = new uint8_t[7];
	uint8_t header[4] = {0x5A, 0xA5, 0x04, 0x83};
	memcpy(txbuf, header, 4);
	memcpy(txbuf + 4, &addr, 2);
	*(txbuf + 6) = 0x01;
	userSerial->write(txbuf, 7);
	delete(txbuf);
	uint8_t res;
	if (pollForDataRx()) {
		res = getDataFromAddr(addr, 1);
	}
}



void Display::tick() {
	
	if (userSerial->available()) {
		size_t rxSize = 7 + 2*readData.length();
		rxbuf = new uint8_t[rxSize];
		userSerial->readBytes(rxbuf, rxSize);
		int16_t addr = int16_t(rxbuf[4]<<8)|rxbuf[5];
		uint8_t length = rxbuf[6];
	}
}

bool Display::pollForDataRx() {
	uint32_t tmr = millis();
	while(millis() - tmr < 500) {
		if(userSerial->available()) 
			return true;
		yield();
	}
	return false;
}

uint8_t Display::getDataFromAddr(uint16_t addr, int16_t* out, uint8_t length) {
	int16_t* values = nullptr;	//указатель куда будут писатьс прочитанные значения
	uint16_t address = 0;		//адрес, который будет считан с порта
	uint8_t command = 0;		//команда чтения
	uint8_t headerH = userSerial->read();	//get header
	uint8_t headerL = userSerial->read();	//get header
	if (headerL != 0xA5)		//если хедер не валидный
		return 1;
	uint8_t rxSize = userSerial->read();	//get data size in bytes
	rxbuf = new uint8_t[rxSize];			//выделяем буфер размером = (команда + адрес + данные)
	userSerial->readBytes(rxbuf, rxSize);
	command = *rxbuf;
	address = uint16_t( *(rxbuf+1) << 8)|*(rxbuf+2);
	values = new int16_t[1 + length];
	memcpy(values, rxbuf + 3, length*2);
	values[length] = nullTerm;
	delete(rxbuf);
	if (address == addr && command == 0x83) {
		for(int i = 0; i < length + 1 || values[i] != nullTerm; i++) {
			out[i] = values[i];
		}
		return 0;
	}
	return 2;
}




























//