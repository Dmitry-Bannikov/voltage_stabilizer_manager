#include <Display.h>


Display::~Display()
{
	userSerial->end();
	delete(userSerial);
}

void Display::Test(int16_t value) {
	uint8_t buffer[8];
	buffer[0] = 0x5A;
	buffer[1] = 0xA5;
	buffer[2] = 0x05;
	buffer[3] = 0x82;
	buffer[4] = 0x50;
	buffer[5] = 0x00;
	buffer[6] = (uint8_t)(value >> 8);
	buffer[7] = (uint8_t)(value & 0xFF);
	userSerial->write(buffer, sizeof(buffer));
}



void Display::tick() {
	static uint32_t tmr = 0;
	if (millis() - tmr > 1000) {
		Test(255);
		tmr = millis();
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





























//