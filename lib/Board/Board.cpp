#include <Board.h>
#include <Wire.h>


//==================Public=================//

Board::Board(){}

Board::Board(const uint8_t addr)
{
	attach(addr);
}

bool Board::attach(const uint8_t addr) {
	if (addr == 0 || addr > 127) return false;
	if (addr != _board_addr)
		_board_addr = addr;
	startFlag = true;
	return isOnline();
}

bool Board::isOnline() {
	Wire.beginTransmission(_board_addr);
	bool error = !!Wire.endTransmission();	//если вернет > 1, то будет true
	return !error;							//нужно вернуть значение обратное ошибке
}

uint8_t Board::getAddress() {
	return _board_addr;
}

void Board::setAddress(const uint8_t addr) {
	_board_addr = addr;
}

uint8_t Board::getDataRaw(int32_t* arr, size_t size) {
	if (!startFlag) return 1;
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_DATA;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	flush(RXBUF);
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	uint8_t* p = reinterpret_cast<uint8_t*>(_rxbuffer);
	if (pollForDataRx()) {
		Wire.readBytes(p, sizeof(_rxbuffer));
	} else {
		return 3;
	}
	if (*_rxbuffer != I2C_DATA_START) return 4;
	for (int i = 1; i <= size || _rxbuffer[i] != I2C_TERMINATOR; i++) { 
		arr[i - 1] = _rxbuffer[i];
	}
	return 0;
}

uint8_t Board::getTrimmers(int32_t* arr, size_t size) {
	if (!startFlag) return 1;
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_TRIMS;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	flush(RXBUF);
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	uint8_t* p = reinterpret_cast<uint8_t*>(_rxbuffer);
	if (pollForDataRx()) {
		Wire.readBytes(p, sizeof(_rxbuffer));
	} else {
		return 3;
	}
	if (*_rxbuffer != I2C_TRIM_START) return 4;
	for (int i = 1; i <= size || _rxbuffer[i] != I2C_TERMINATOR; i++) { 
		arr[i - 1] = _rxbuffer[i];
	}
	return 0;
}

uint8_t Board::getStatisRaw(int32_t* arr, size_t size) {
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_STAT;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	flush(RXBUF);
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	uint8_t* p = reinterpret_cast<uint8_t*>(_rxbuffer);
	if (pollForDataRx()) {
		Wire.readBytes(p, sizeof(_rxbuffer));
	} else {
		return 3;
	}
	if (*_rxbuffer != I2C_STAT_START) return 4;
	for (int i = 1; i <= size || _rxbuffer[i] != I2C_TERMINATOR; i++) { 
		arr[i - 1] = _rxbuffer[i];
	}
	return 0;
}

uint8_t Board::getStatis(int32_t* arr, size_t size ) {
	if (!startFlag) return 1;
	static uint32_t last_update = 0;
	static int32_t statis[12] = {0};
	int error = 0;
	if (millis() - last_update >= _statisUpdatePrd) {
		error = getStatisRaw(statis);
		_workTime_mins = statis[0];
		last_update = millis();
	}
	for (int i = 0; i < size; i++) {
		arr[i] = statis[i];
	}
	return error;
	
}

uint8_t Board::getData(int32_t* arr, size_t size) {
	if (!startFlag) return 1;
	static uint32_t last_update = 0;
	static int32_t data[5] = {0};
	int error = 0;
	if (millis() - last_update >= _dataUpdatePrd) {
		error = getDataRaw(data);
	}
	for (int i = 0; i < size; i++) {
		arr[i] = data[i];
	}
	return error;
}

uint8_t Board::sendTrimmers(int32_t* arr, size_t size) {
	if (!startFlag) return 1;
	flush(TXBUF);
	*_txbuffer = I2C_TRIM_START;
	for (int i = 1; i <= size; i++) {
		_txbuffer[i] = arr[i - 1];
	}
	_txbuffer[size + 1] = I2C_TERMINATOR;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	return 0;
}

uint8_t Board::sendBSets(int32_t* arr, size_t size) {
	if (!startFlag) return 1;
	flush(TXBUF);
	*_txbuffer = I2C_BSET_START;
	for (int i = 1; i <= size; i++) {
		_txbuffer[i] = arr[i - 1];
	}
	_txbuffer[size + 1] = I2C_TERMINATOR;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	return 0;
}

uint8_t Board::reboot() {
	if (!startFlag) return 1;
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_REBOOT;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	return 0;
}

uint8_t Board::toggleRegulation() {
	if (!startFlag) return 1;
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_NOREG;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	return 0;
}

void Board::getDataStr(String& out) {
	int32_t gData[5] = {0};
	getData(gData);
	float fullPwr_kVA = (float)(gData[4])/1000.0;
	float load_Amps = (float)(gData[2])/1000.0;

	String s = "";
	s += F(" Board Data: 0x");
	s += String(_board_addr, HEX);
	s += F("\nInput V    : ");
	s += String(gData[0]);
	s += F("\nOutput V   : ");
	s += String(gData[1]);
	s += F("\nOutput Load: ");
	s += String(load_Amps, 1);
	s += F("\nFull Power : ");
	s += String(fullPwr_kVA, 1);
	s += F("\nErrors     : ");
	s += errorsToStr(gData[3]);
	out = s;
}

void Board::getStatisStr(String& out) {
	int32_t gStatis[12] = {0};
	getStatis(gStatis);
	String s = "";
	s += F(" Board Stats: 0x");
	s += String(_board_addr, HEX);
	s += F("\nWork T: ");
	s += getWorkTime(_workTime_mins);
	s += F("\nMax output V : ");
	s += String(gStatis[1]);
	s += F("\nAvg output V : ");
	s += String(gStatis[2]);
	s += F("\nMin output V : ");
	s += String(gStatis[3]);

	s += F("\nMax input V  : ");
	s += String(gStatis[4]);
	s += F("\nAvg input V  : ");
	s += String(gStatis[5]);
	s += F("\nMin input V  : ");
	s += String(gStatis[6]);

	float max_load = (float)(gStatis[7])/1000.0;
	float avg_load = (float)(gStatis[8])/1000.0;
	s += F("\nMax load A   : ");
	s += String(max_load, 1);
	s += F("\nAvg load A   : ");
	s += String(avg_load, 1);
	
	float max_pwr = (float)(gStatis[9])/1000.0;
	float avg_pwr = (float)(gStatis[10])/1000.0;
	s += F("\nMax power    : ");
	s += String(max_pwr, 1);
	s += F("\nAvg power    : ");
	s += String(avg_pwr, 1);

	s += F("\nErrors: ");
	s += errorsToStr(gStatis[11]);
	out = s;
}

void Board::tick() {
	static uint32_t tmr = 0;
	if (millis() - tmr > 1000) {
		int32_t statis[12];
		getStatis(statis);
		tmr = millis();
	}

}

int32_t Board::getWorkTime() {
	return _workTime_mins;
}

void Board::detach() {
	if (!startFlag) return;
	startFlag = false;
}







//========Private=======//

void Board::flush(BufferType type) {
	if (type == RXBUF) {
		memset(_rxbuffer, 0, sizeof(_rxbuffer));
	} else {
		memset(_txbuffer, 0, sizeof(_txbuffer));
	}
}

bool Board::pollForDataRx() {
	uint32_t tmr = millis();
	while (millis() - tmr < _poll) {
		if (Wire.available()) 
			return true;
		yield();
	}
	return false;
}

String Board::errorsToStr(const int32_t errors) {
	String s = "";
	if (errors <= 1) {
		s = "No";
		return s;
	}
	for (uint8_t i = 0; i < 16; i++) {
		if (errors & (1<<i)) {
		if (i < 10) {
			s += "A0";
			s += String(i);
		} else {
			s += "A";
			s += String(i);
		}
		s += ", ";
		}
	}
	if (s.length()) {
		s.remove(s.length() - 2);
	}
	return s;
}

String Board::getWorkTime(const uint32_t mins) {
	uint32_t raw_mins = mins;
	uint32_t days, hours, minutes;
	days = raw_mins / (24 * 60);
	raw_mins %= (24 * 60);
	hours = raw_mins / 60;
    raw_mins %= 60;
	minutes = raw_mins;
	String s = "";
	s += F(" ");
	s += String(days);
	s += F("d ");
	s += String(hours);
	s += F("h ");
	s += String(minutes);
	s += F("m ");
	return s;
}

Board::~Board(){}






