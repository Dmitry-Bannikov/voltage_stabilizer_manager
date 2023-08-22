#include <Board.h>
#include <Wire.h>


//==================Public=================//

Board::Board()
{
	allocateBuffers();
}

Board::Board(const uint8_t addr)
{
	attach(addr);
}


bool Board::attach(const uint8_t addr) {
	if (addr == 0 || addr > 127) return false;
	if (addr != _board_addr)
		_board_addr = addr;
	bool res1 = Board::allocateBuffers();
	bool res2 = isOnline();
	if (res1 && res2) {
		startFlag = true;
		return true;
	}
	return false;
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

bool Board::setBufferSizes(const uint16_t txbufsize, const uint16_t rxbufsize) {
	if (!startFlag) return false;
	_txsize = txbufsize;
	_rxsize = rxbufsize;
	return Board::allocateBuffers();
}

bool Board::sendData(const int32_t command) {
	if (!startFlag) return false;
	flushTxBuf();
	*_txbuffer = command;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (!error) return true;
	return false;
}

bool Board::sendData(const int32_t command, const int32_t* txdata, const int txdataSize) {
	if (!startFlag) return false;
	if (txdataSize >= _txsize ) return false;
	flushTxBuf();
	*_txbuffer = command;
	for (int i = 0; i < txdataSize; i++) {
		*(_txbuffer + i + 1) = *(txdata + i);
	}
	_txbuffer[txdataSize + 1] = I2C_TERMINATOR;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeofTx());
	uint8_t error = Wire.endTransmission();
	if (!error) return true;
	return false;
}

size_t Board::getData(int32_t* rxdata) {
	if (!startFlag) return 0;
	flushRxBuf();
	Wire.flush();
	Wire.requestFrom(_board_addr, sizeofRx());
	int tmr = millis();
	if (pollForDataRx()) {
		Wire.readBytes((uint8_t*)_rxbuffer, sizeofRx());
	} else {
		return false;
	}
	size_t res = 0;
	for (int i = 0; i < _rxsize; i++, ++res ) {
		*(rxdata + i) = *(_rxbuffer + i);
		if (_rxbuffer[i] == I2C_TERMINATOR) break;
	}
	return res;
}

size_t Board::getData(const int32_t key, int32_t* rxdata) {
	if (!startFlag) return 0;
	flushRxBuf();
	Wire.flush();
	Wire.requestFrom(_board_addr, sizeofRx());
	int tmr = millis();
	if (pollForDataRx()) {
		Wire.readBytes((uint8_t*)_rxbuffer, sizeofRx());
	} else {
		return false;
	}
	if (*_rxbuffer != key) return 0;
	size_t res = 0;
	for (int i = 1; i < _rxsize; i++, ++res ) {
		if (_rxbuffer[i] == I2C_TERMINATOR) break;
		*(rxdata + i - 1) = *(_rxbuffer + i);
	}
	return res;
}

size_t Board::getDataKey(int32_t &key, int32_t* rxdata) {
	if (!startFlag) return 0;
	flushRxBuf();
	Wire.flush();
	Wire.requestFrom(_board_addr, sizeofRx());
	int tmr = millis();
	if (pollForDataRx()) {
		Wire.readBytes((uint8_t*)_rxbuffer, sizeofRx());
	} else {
		return false;
	}
	key = *_rxbuffer;
	size_t res = 0;
	for (int i = 1; i < _rxsize; i++, ++res ) {
		if (_rxbuffer[i] == I2C_TERMINATOR) break;
		*(rxdata + i - 1) = *(_rxbuffer + i);
	}
	return res;
}

size_t Board::getDataCommand(const int32_t command, int32_t* rxdata) {
	if (!startFlag) return 0;
	sendData(command);
	return getData(rxdata);
}

bool Board::getValues(int32_t* arr, size_t size) {
	if (!startFlag) return false;
	int32_t temp_arr[20];
	sendData(I2C_REQUEST_DATA);
	int res = getData(I2C_DATA_START, temp_arr);
	for (int i = 0; i < res || i < size; i++) {
		*(arr + i) = *(temp_arr + i);
	}
	return !!res;
}

bool Board::getTrimmers(int32_t* arr, size_t size) {
	if (!startFlag) return false;
	int32_t temp_arr[20];
	sendData(I2C_REQUEST_TRIMS);
	int res = getData(I2C_TRIM_START, temp_arr);
	for (int i = 0; i < res || i < size; i++) {
		*(arr + i) = *(temp_arr + i);
	}
	return !!res;
}

bool Board::getStatis(int32_t* arr, size_t size ) {
	if (!startFlag) return false;
	static int32_t temp_arr[20] = {0};
	int res = 0;
	if (millis() - _lastGetStat > _statisUpdatePrd) { 	//если прошло больше минуты
    	sendData(I2C_REQUEST_STAT);						//забираем свежие данные с платы
		res = getData(I2C_STAT_START, temp_arr);
		if (res > 0) {
			_startkey = temp_arr[0];										//проверяем стартовый ключ
			_workTime_mins = (_startkey == 0 ? 0 : _workTime_mins + 1);		//если он == 0, обнуляем счетчик работы
			_lastGetStat = millis();					//обновляем таймер
		}
    } 
	for (int i = 0; i < size; i++) {
		*(arr + i) = *(temp_arr + i);
	}
	return !!res;
}

bool Board::sendTrimmers(int32_t* arr, size_t size) {
	if (!startFlag) return false;
	return sendData(I2C_TRIM_START, arr, size);
}

bool Board::sendBSets(int32_t* arr, size_t size) {
	if (!startFlag) return false;
	return sendData(I2C_BSET_START, arr, size);
}

bool Board::reboot() {
	if (!startFlag) return false;
	return sendData(I2C_REQUEST_REBOOT);
}

bool Board::toggleRegulation() {
	if (!startFlag) return false;
	return sendData(I2C_REQUEST_NOREG);
}

bool Board::setStartKey() {
	if (!startFlag) return false;
	return sendData(I2C_SET_STARTKEY);
}

void Board::getValuesStr(String& out) {
	int32_t gData[5] = {0};
	getValues(gData);
	float fullPwr_kVA = (float)(gData[4])/1000.0;
	float load_Amps = (float)(gData[2])/1000.0;

	String s = "";
	s += F(" Board: ");
	s += String(_board_addr, HEX);
	s += F("\nInput Voltage, V:\t");
	s += String(gData[0]);
	s += F("\nOutput Voltage, V:\t");
	s += String(gData[1]);
	s += F("\nOutput Current, A:\t");
	s += String(load_Amps, 3);
	s += F("\nFull Power, kVA:\t");
	s += String(fullPwr_kVA, 3);
	s += F("\nErrors: ");
	s += errorsToStr(gData[3]);
	out = s;
}

void Board::getStatisStr(String& out) {
	static uint32_t tmr = 0;
	if (millis() - _lastGetStat < 59999) return;
	int32_t gStatis[12] = {0};
	getStatis(gStatis);
	String s = "";
	s += F(" Board: ");
	s += String(_board_addr, HEX);
	s += F("\nWork Time: ");
	s += getWorkTime(_workTime_mins);
	s += F("\nMax output V:\t");
	s += String(gStatis[1]);
	s += F("\nAvg output V:\t");
	s += String(gStatis[2]);
	s += F("\nMin output V:\t");
	s += String(gStatis[3]);

	s += F("\nMax input V:\t");
	s += String(gStatis[4]);
	s += F("\nAvg input V:\t");
	s += String(gStatis[5]);
	s += F("\nMin input V:\t");
	s += String(gStatis[6]);

	float max_load = float(gStatis[7])/1000.0;
	float avg_load = float(gStatis[8])/1000.0;
	s += F("\nMax load A:\t");
	s += String(max_load, 2);
	s += F("\nAvg load A:\t");
	s += String(avg_load, 2);
	
	s += F("\nMax power, VA:\t");
	s += String(gStatis[9]);
	s += F("\nAvg power, VA:\t");
	s += String(gStatis[10]);

	s += F("\nErrors: ");
	s += errorsToStr(gStatis[11]);
}

void Board::tick() {
	static uint32_t tmr = 0;
	if (millis() - tmr > 1000) {
		int32_t data[6];
		int32_t statis[20];
		getValues(data);
		getStatis(statis);
		tmr = millis();
	}

}

void Board::detach() {
	if (!startFlag) return;
	deleteBuffers();
	startFlag = false;
}







//========Private=======//

bool Board::allocateBuffers() {
	//удаление старых буферов
	bool res = false;
	deleteBuffers();
	//создание новых
	_txbuffer = new int32_t[_txsize];
	Board::flushTxBuf();
	_rxbuffer = new int32_t[_rxsize];
	Board::flushRxBuf();
	if (_txbuffer != nullptr && _rxbuffer != nullptr) {
		res = true;
	}
	return res;
}

void Board::deleteBuffers() {
	delete[] _txbuffer;
  	delete[] _rxbuffer;
  	_txbuffer = nullptr;
 	_rxbuffer = nullptr;
}

void Board::flushRxBuf() {
	memset(_rxbuffer, _flush_val, _rxsize*sizeof(*_rxbuffer));
}

void Board::flushTxBuf() {
	memset(_txbuffer, _flush_val, _txsize*sizeof(*_txbuffer));
}

uint32_t Board::sizeofTx() {
	return _txsize*sizeof(*_txbuffer);
}

uint32_t Board::sizeofRx() {
	return _rxsize*sizeof(*_rxbuffer);
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
		s = "Board State OK";
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

Board::~Board()
{
	deleteBuffers();
}






