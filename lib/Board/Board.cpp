#include <Board.h>
#include <Wire.h>


//==================Public=================//


bool Board::attach(const uint8_t addr) {
	if (addr == 0 || addr > 127) return false;
	if (addr != _board_addr)
		_board_addr = addr;
	startFlag = true;
	return isOnline();
}

bool Board::isBoard(uint8_t addr) {
	Wire.clearWriteError();
	Wire.beginTransmission(addr);
	if (Wire.endTransmission()) return false;
	uint8_t txbuf[100] = {0x20, 0};
	uint8_t rxbuf[100] = {0};
	Wire.beginTransmission(addr);
	Wire.write(txbuf,100);
	uint8_t err = Wire.endTransmission();
	if (err != 0) return false;
	Wire.requestFrom(addr, (size_t)100);
	uint32_t tmr = millis();
	while (millis() - tmr < 50) {
		if (Wire.available()) {
			Wire.readBytes(rxbuf, 100);
			if (*rxbuf == 0xFF) return true;
			else return false;	
		}	
		yield();
	}
	return false;
}

bool Board::isOnline() {
	Wire.clearWriteError();
	Wire.beginTransmission(_board_addr);
	uint8_t error = Wire.endTransmission();	
	if (error && _disconnected > 3) {
		return false;
	}
	_disconnected = 0;
	return true;							
}

bool Board::setAddress(const uint8_t addr) {
	if (addr == 0 || addr > 127) return false;
	_board_addr = addr;
	return true;
}

uint8_t Board::getDataRaw() {
	if (!startFlag) return ERR_INIT;
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_DATA;
	Wire.clearWriteError();
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return ERR_CONNECT;
	flush(RXBUF);
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	if (pollForDataRx()) {
		Wire.readBytes(_rxbuffer, sizeof(_rxbuffer));
	} else {
		return ERR_TIMEOUT;
	}
	if (*_rxbuffer != I2C_DATA_START) return ERR_STARTCODE;
	memcpy(mainData.buffer, _rxbuffer + 1, mainData.structSize);
	mainData.unpackData();
	return ERR_NO;
}

uint8_t Board::getMainSets() {
	if (!startFlag) return ERR_INIT;
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_MAINSETS;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return ERR_CONNECT;
	flush(RXBUF);
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	if (pollForDataRx()) {
		Wire.readBytes(_rxbuffer, sizeof(_rxbuffer));
	} else {
		return ERR_TIMEOUT;
	}
	if (*_rxbuffer != I2C_MAINSETS_START) return ERR_STARTCODE;
	memcpy(mainSets.buffer, _rxbuffer + 1, mainSets.structSize);
	mainSets.unpackData();
	return ERR_NO;
}

uint8_t Board::getAddSets() {
	if (!startFlag) return ERR_INIT;
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_ADDSETS;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return ERR_CONNECT;
	flush(RXBUF);
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	if (pollForDataRx()) {
		Wire.readBytes(_rxbuffer, sizeof(_rxbuffer));
	} else {
		return ERR_TIMEOUT;
	}
	if (*_rxbuffer != I2C_ADDSETS_START) return ERR_STARTCODE;
	memcpy(addSets.buffer, _rxbuffer + 1, addSets.structSize);
	addSets.unpackData();
	return ERR_NO;
}

uint8_t Board::getStatisRaw() {
	Wire.clearWriteError();
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_STAT;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return ERR_CONNECT;
	flush(RXBUF);
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	if (pollForDataRx()) {
		Wire.readBytes(_rxbuffer, sizeof(_rxbuffer));
	} else {
		return ERR_TIMEOUT;
	}
	if (*_rxbuffer != I2C_STAT_START) return ERR_STARTCODE;
	memcpy(mainStats.buffer, _rxbuffer + 1, mainStats.structSize);
	mainStats.unpackData();
	return ERR_NO;
}

uint8_t Board::getStatis() {
	if (!startFlag) return ERR_INIT;
	static uint32_t last_update = 0;
	uint8_t error = 0;
	if (millis() - last_update >= _statisUpdatePrd) {
		error = getStatisRaw();
		getStatisStr();
		last_update = millis();
		
	}
	return error;
	
}

uint8_t Board::getData() {
	if (!startFlag) return ERR_INIT;
	static uint32_t last_update = 0;
	uint8_t error = 0;
	if (millis() - last_update >= _dataUpdatePrd) {
		error = getDataRaw();
		getDataStr();
		last_update = millis();
		if (error) {
			_disconnected++;
		} else {
			_disconnected = 0;
		}
	}
	return error;
}

uint8_t Board::sendMainSets() {
	if (!startFlag) return ERR_INIT;
	flush(TXBUF);
	*_txbuffer = I2C_MAINSETS_START;
	mainSets.packData();
	memcpy(_txbuffer+1, mainSets.buffer, mainSets.structSize);
	Wire.beginTransmission(_board_addr);
	Wire.write(_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return ERR_CONNECT;
	return ERR_NO;
}

uint8_t Board::sendAddSets() {
	if (!startFlag) return ERR_INIT;
	flush(TXBUF);
	*_txbuffer = I2C_ADDSETS_START;
	addSets.packData();
	memcpy(_txbuffer+1, addSets.buffer, addSets.structSize);
	Wire.beginTransmission(_board_addr);
	Wire.write(_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return ERR_CONNECT;
	return ERR_NO;
}

uint8_t Board::reboot() {
	if (!startFlag) return ERR_INIT;
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_REBOOT;
	Wire.beginTransmission(_board_addr);
	Wire.write(_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return ERR_CONNECT;
	return ERR_NO;
}

uint8_t Board::toggleRegulation() {
	if (!startFlag) return ERR_INIT;
	flush(TXBUF);
	*_txbuffer = I2C_REQUEST_NOREG;
	Wire.beginTransmission(_board_addr);
	Wire.write(_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return ERR_CONNECT;
	return ERR_NO;
}

void Board::getDataStr() {
	float full_pwr = mainData.outputPower/mainData.cosfi/1000.0;
	String s = "";
	s += F(" Данные: ");
	if (mainSets.liter != '\0') {
		s += getLiteral();
	} else {
		s += String(_board_addr);
	}

	s += F("\nU вход   : ");
	s += String(mainData.inputVoltage);
	s += F("\nU выход  : ");
	s += String(mainData.outputVoltage);
	s += F("\nI выход  : ");
	s += String(mainData.outputCurrent, 1);
	s += F("\nP полн.  : ");
	s += String(full_pwr, 1);
	s += F("\nP актив. : ");
	s += String(mainData.outputPower/1000.0, 1);
	s += F("\nСобытия  : ");
	s += errorsToStr(mainData.events, EVENTS_FULL);
	mainData.Str = s;
}

void Board::getStatisStr() {
/*

U вход   |	220	220	220
U выход  | 
Вых. Ток |	
Мощность |
События  | A01, A03, A04

*/

	float maxPwr = mainStats.powerMax/1000.0;
	float avgPwr = mainStats.powerAvg/1000.0;
	String s = "";
	s += F("Cтат. : ");
	if (mainSets.liter != '\0') {
		s += getLiteral();
	} else {
		s += String(_board_addr);
	}
	s += F("\nWork T: ");
	s += getWorkTime(mainStats.workTimeMins);

	s += F("\nU вход   |");
	s += String(mainStats.outVoltMax);
	s += F("|");
	s += String(mainStats.outVoltAvg);
	s += F("|");
	s += String(mainStats.outVoltMin);

	s += F("\nU выход  |");
	s += String(mainStats.inVoltMax);
	s += F("|");
	s += String(mainStats.inVoltAvg);
	s += F("|");
	s += String(mainStats.inVoltMin);

	s += F("\nВых. ток |");
	s += String(mainStats.outLoadMax, 1);
	s += F("|");
	s += String(mainStats.outLoadAvg, 1);

	s += F("\nМощность |");
	s += String(maxPwr,1);
	s += F("|");
	s += String(avgPwr,1);

	s += F("\nСобытия  |");
	s += errorsToStr(mainStats.boardEvents, EVENTS_SHORT);
	
	mainStats.Str = s;
}

void Board::tick() {
	static uint32_t tmr = 0;
	memMainSets.tick();
	memAddSets.tick();
	if (millis() - tmr >= 1000) {
		getData();
		getStatis();
		tmr = millis();
	}

}

void Board::detach() {
	if (!startFlag) return;
	startFlag = false;
}

void Board::saveSettings() {
	mainSets.packData();
	addSets.packData();
	memMainSets.updateNow();
	memAddSets.updateNow();
}

void Board::readSettings() {
	memMainSets.begin(_memoryAddr, _memoryKey);
	delay(1);
	memAddSets.begin(_memoryAddr + mainSets.structSize + 1, _memoryKey);
	delay(1);
	mainSets.unpackData();
	addSets.unpackData();
}

String 	Board::getLiteral() {
	return String(mainSets.liter);
}

void Board::setLiteral(String lit) {
	mainSets.liter = lit.charAt(0);
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

String Board::errorsToStr(const int32_t errors, EventsFormat f) {
	String s = "";
	if (errors <= 1) {
		s = "No";
		return s;
	}
	if (f == EVENTS_SHORT) {
		for (uint8_t i = 0; i < 32; i++) {
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
	} else {
		for (uint8_t i = 0; i < 32; i++) {
			if (errors & (1<<i)) {
				switch (i)
				{
				case 1:
					s += "перегрузка";
					break;
				case 2:
					s += "внеш.авария";
					break;
				case 3:
					s += "меньше 80";
					break;
				case 4:
					s += "недонапряжение";
					break;
				case 5:
					s += "перенапряжение";
					break;
				case 6:
					s += "концевик";
					break;
				case 7:
					s += "температура";
					break;
				case 8:
					s += "заклинило";
					break;
				case 9:
					s += "";
					break;
				case 10:
					s += "";
					break;
				
				default:
					break;
				}
				s += ", ";
			}

		}
		if (s.length()) {
			s.remove(s.length() - 2);
		}
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

uint8_t Board::scanBoards(std::vector<Board> &brd, const uint8_t max) {
	for (uint8_t i = 0; i < brd.size(); i++) {
		if (!brd[i].isOnline()) { 					//если плата под номером i не онлайн
			brd.erase(brd.begin() + i);				//удаляем 
			delay(1);
		}
	}

	for (uint8_t addr = 1; addr < 128; addr++) {
		if (Board::isBoard(addr) && brd.size() < max) {
			if (!brd.size()) {
				brd.emplace_back(addr);
				continue;
			}
			else {
				bool reserved = false;
				for (uint8_t i = 0; i < brd.size(); i++) {
					if (brd[i].getAddress() == addr) {
						reserved = true;
						break;
					}
				}
				if (!reserved) brd.emplace_back(addr);
			}
			delay(10);
		}
	}
	
	return brd.size();
}



