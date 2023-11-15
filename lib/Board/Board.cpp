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
	if (!startFlag) return 1;
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_REQUEST_DATA;
	Wire.clearWriteError();
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	memset(_rxbuffer, 0, sizeof(_rxbuffer));
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	if (pollForDataRx()) {
		Wire.readBytes(_rxbuffer, sizeof(_rxbuffer));
	} else {
		return 3;
	}
	if (*_rxbuffer != I2C_DATA_START) return 4;
	memcpy(mainData.buffer, _rxbuffer + 1, mainData.structSize);
	mainData.unpackData();
	return 0;
}

uint8_t Board::getMainSets() {
	if (!startFlag) return 1;
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_REQUEST_MAINSETS;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	memset(_rxbuffer, 0, sizeof(_rxbuffer));
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	if (pollForDataRx()) {
		Wire.readBytes(_rxbuffer, sizeof(_rxbuffer));
	} else {
		return 3;
	}
	if (*_rxbuffer != I2C_MAINSETS_START) return 4;
	memcpy(mainSets.buffer, _rxbuffer + 1, mainSets.structSize);
	mainSets.unpackData();
	return 0;
}

uint8_t Board::getAddSets() {
	if (!startFlag) return 1;
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_REQUEST_ADDSETS;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	memset(_rxbuffer, 0, sizeof(_rxbuffer));
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	if (pollForDataRx()) {
		Wire.readBytes(_rxbuffer, sizeof(_rxbuffer));
	} else {
		return 3;
	}
	if (*_rxbuffer != I2C_ADDSETS_START) return 4;
	memcpy(addSets.buffer, _rxbuffer + 1, addSets.structSize);
	addSets.unpackData();
	return 0;
}

uint8_t Board::getStatisRaw() {
	Wire.clearWriteError();
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_REQUEST_STAT;
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	memset(_rxbuffer, 0, sizeof(_rxbuffer));
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	if (pollForDataRx()) {
		Wire.readBytes(_rxbuffer, sizeof(_rxbuffer));
	} else {
		return 3;
	}
	if (*_rxbuffer != I2C_STAT_START) return 4;
	memcpy(mainStats.buffer, _rxbuffer + 1, mainStats.structSize);
	mainStats.unpackData();
	return 0;
}

uint8_t Board::getStatis() {
	if (!startFlag) return 1;
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
	if (!startFlag) return 1;
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
	if (!startFlag) return 1;
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_MAINSETS_START;
	mainSets.packData();
	memcpy(_txbuffer+1, mainSets.buffer, mainSets.structSize);
	Wire.beginTransmission(_board_addr);
	Wire.write(_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	return 0;
}

uint8_t Board::sendAddSets() {
	if (!startFlag) return 1;
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_ADDSETS_START;
	addSets.packData();
	memcpy(_txbuffer+1, addSets.buffer, addSets.structSize);
	Wire.beginTransmission(_board_addr);
	Wire.write(_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	return 0;
}



uint8_t Board::sendCommand(uint8_t command, uint8_t value) {
	if (!startFlag) return 1;
	if (command > (sizeof(addSets.Switches)*8)-1) {
		return 1;
	}
	if (value) {
		addSets.Switches |= (1<<command);
	} else {
		addSets.Switches &=~ (1<<command);
	}
	
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_SWITCHES_START;
	memcpy(_txbuffer+1, (uint8_t*)&addSets.Switches, sizeof(addSets.Switches));
	Wire.beginTransmission(_board_addr);
	Wire.write(_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	return 0;
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

	float maxPwr = mainStats.power[0]/1000.0;
	float avgPwr = mainStats.power[1]/1000.0;
	String s = "";
	s += F("Cтатистика : ");
	if (mainSets.liter != 'N') {
		s += getLiteral();
	} else {
		s += String(_board_addr);
	}
	s += F("\nWork T: ");
	s += getWorkTime(mainStats.workTimeMins);

	s += F("\nU вх  |");
	s += String(mainStats.inVoltage[0]);
	s += F("|");
	s += String(mainStats.inVoltage[1]);
	s += F("|");
	s += String(mainStats.inVoltage[2]);

	s += F("\nU вых |");
	s += String(mainStats.outVoltage[0]);
	s += F("|");
	s += String(mainStats.outVoltage[1]);
	s += F("|");
	s += String(mainStats.outVoltage[2]);

	s += F("\nI вых |");
	s += String(mainStats.outCurrent[0], 1);
	s += F("|");
	s += String(mainStats.outCurrent[1], 1);

	s += F("\nP акт |");
	s += String(maxPwr,1);
	s += F("|");
	s += String(avgPwr,1);

	s += F("\nСобытия: ");
	s += errorsToStr(mainStats.boardEvents, EVENTS_SHORT);
	
	mainStats.Str = s;
}

void Board::tick() {
	static uint32_t tmr = 0;
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

String Board::getLiteral() {
	return String(mainSets.liter);
}

void Board::setLiteral(String lit) {
	mainSets.liter = lit.charAt(0);
}

void Board::setLiteral(char lit) {
	mainSets.liter = lit;
}

String Board::getMotKoefList() {
	String result = "";
	for (uint8_t i = 0; i < sizeof(addSets.motKoefsList); i++) {
		result += String(addSets.motKoefsList[i]);
		result += String(",");
	}
	return result;
}

String Board::getTcRatioList() {
	String result = "";
	for (uint8_t i = 0; i < sizeof(addSets.tcRatioList); i++) {
		result += String(addSets.tcRatioList[i]);
		result += String(",");
	}
	
	return result;
}

//========Private=======//

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
		s = "Нет";
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
		static uint8_t i = 0;
		while (i <= 32) {
			if (errors & (1<<i)) {
				s = gEventsList[i-1];
				i++;
				return s;
			}
			i < 32 ? i++ : (i = 0);
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



