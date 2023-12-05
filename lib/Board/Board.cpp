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

//плата А 3-я 220
//плата В центральная 230


bool Board::isBoard(uint8_t addr) {
	Wire.clearWriteError();
	Wire.setTimeOut(50);
	Wire.beginTransmission(addr);
	uint8_t res = Wire.endTransmission();
	if (res) return false;
	uint8_t txbuf[sizeof(_txbuffer)] = {0x20, 0};
	uint8_t rxbuf[sizeof(_rxbuffer)] = {0};
	Wire.beginTransmission(addr);
	Wire.write(txbuf,100);
	uint8_t err = Wire.endTransmission();
	if (err != 0) return false;
	Wire.requestFrom(addr, (size_t)100);
	uint32_t tmr = millis();
	while (millis() - tmr < 50) {
		if (Wire.available()) {
			Wire.readBytes(rxbuf, 100);
			if (rxbuf[0] == 0x20 && rxbuf[1] == 0xF0) return true;
			else return false;	
		}	
		yield();
	}
	return false;
}

void Board::waitForReady() {
	//delay(5);
	uint32_t tmr = millis();
	while (millis() - tmr < 100) {
		if (!Wire.available() && !Wire.availableForWrite()) {
			return;
		}	
		yield();
	} 
}

bool Board::isOnline() {
	Wire.beginTransmission(_board_addr);
	uint8_t error = Wire.endTransmission();
	if (error) {
		return false;
	}
	return true;							
}

bool Board::isAnswer() {
	if (_disconnected) return false;
	return true;
}

uint8_t Board::getDataRaw() {
	if (!startFlag) return 1;
	Board::waitForReady();
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_REQUEST_DATA;
	Wire.clearWriteError();
	Wire.beginTransmission(_board_addr);
	Wire.write((uint8_t*)_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 20+error;
	memset(_rxbuffer, 0, sizeof(_rxbuffer));
	Wire.requestFrom(_board_addr, sizeof(_rxbuffer));
	if (pollForDataRx()) {
		Wire.readBytes(_rxbuffer, sizeof(_rxbuffer));
	} else {
		return 3;
	}
	if (*_rxbuffer != I2C_DATA_START) return 4;
	memcpy(mainData.buffer, _rxbuffer + 1, mainData.structSize);
	memcpy(mainStats.buffer, _rxbuffer + 1 + mainData.structSize, mainStats.structSize);
	mainData.unpackData();
	mainStats.unpackData();
	return 0;
}

uint8_t Board::getMainSets() {
	if (!startFlag) return 1;
	Board::waitForReady();
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
	memcpy(addSets.buffer, _rxbuffer + 1 + mainSets.structSize, addSets.structSize);
	mainSets.unpackData();
	addSets.unpackData();
	return 0;
}

uint8_t Board::getData() {
	if (!startFlag) return 1;
	static uint8_t disconn = 0;
	static uint32_t last_update = 0;
	uint8_t error = getDataRaw();
	getDataStr();
	getStatisStr();
	if (error) {
		(disconn < 10) ? (disconn++) : (disconn = 10, _disconnected = 1);
	} else {
		disconn = 0;
		_disconnected = 0;
	}
	return error;
}

uint8_t Board::sendMainSets() {
	if (!startFlag) return 1;
	Board::waitForReady();
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_MAINSETS_START;
	mainSets.packData();
	addSets.packData();
	memcpy(_txbuffer+1, mainSets.buffer, mainSets.structSize);
	memcpy(_txbuffer+1 + mainSets.structSize, addSets.buffer, addSets.structSize);
	Wire.beginTransmission(_board_addr);
	Wire.write(_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	return 0;
}

uint8_t Board::sendCommand(uint8_t command, uint8_t value) {
	if (!startFlag) return 1;
	Board::waitForReady();
	addSets.Switches[command] = value;
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_SWITCHES_START;
	memcpy(_txbuffer+1, addSets.Switches, sizeof(addSets.Switches));
	Wire.beginTransmission(_board_addr);
	Wire.write(_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	return 0;
}

uint8_t Board::sendCommand(uint8_t* command) {
	if (!startFlag) return 1;
	Board::waitForReady();
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_SWITCHES_START;
	memcpy(_txbuffer+1, command, 8);
	Wire.beginTransmission(_board_addr);
	Wire.write(_txbuffer, sizeof(_txbuffer));
	uint8_t error = Wire.endTransmission();
	if (error != 0) return 2;
	return 0;
}

void Board::getDataStr() {
	float full_pwr = mainData.outputPower/mainData.cosfi/1000.0;
	String s = "";
	s += F(" Данные платы: ");
	if (mainSets.liter != '\0') {
		s += getLiteral();
	} else {
		s += String(_board_addr);
	}

	s += F("\nU вход   : ");
	s += String(mainData.inputVoltage);
	s += F("V");

	s += F("\nU выход  : ");
	s += String(mainData.outputVoltage);
	s += F("V");

	s += F("\nI выход  : ");
	s += String(mainData.outputCurrent, 1);
	s += F("A");

	s += F("\nP полн.  : ");
	s += String(full_pwr, 1);
	s += F("kVA");

	//s += F("\nP актив. : ");
	//s += String(mainData.outputPower/1000.0, 1);
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
	s += F("\nt работы:");
	s += getWorkTime(mainStats.workTimeMins);

	s += F("\nU вх.макс : ");
	s += String(mainStats.inVoltage[0]);

	s += F("\nU вх.сред : ");
	s += String(mainStats.inVoltage[1]);

	s += F("\nU вх.мин  : ");
	s += String(mainStats.inVoltage[2]);

	s += F("\nU вых.макс: ");
	s += String(mainStats.outVoltage[0]);

	s += F("\nU вых.сред: ");
	s += String(mainStats.outVoltage[1]);

	s += F("\nU вых.мин : ");
	s += String(mainStats.outVoltage[2]);
	s += F("V");

	s += F("\nI макс, А : ");
	s += String(mainStats.outCurrent[0], 1);
	s += F("A");

	s += F("\nI сред, А : ");
	s += String(mainStats.outCurrent[1], 1);
	s += F("A");

	s += F("\nP макс,kVA: ");
	s += String(maxPwr,1);

	s += F("\nP сред,kVA: ");
	s += String(avgPwr,1);

	s += F("\nСобытия: ");
	s += errorsToStr(mainStats.boardEvents, EVENTS_SHORT);
	
	mainStats.Str = s;
}

void Board::tick() {
	getData();
}

void Board::detach() {
	if (!startFlag) return;
	_board_addr = 0;
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
	if (brd.size() == max) return brd.size();
	for (uint8_t addr = 1; addr < 128; addr++) {				//проходимся по возможным адресам
		if (Board::isBoard(addr)) {				//если на этом адресе есть плата, и кол-во плат < макс
			bool reserved = false;	
			for (uint8_t i = 0; i < brd.size(); i++) {				//проходимся по уже существующим платам
				if (brd[i].getAddress() == addr) {						//если эта плата уже имеет этот адрес
					reserved = true;									//то отмечаем как зарезервировано
				}
			}
			if (!reserved) brd.emplace_back(addr);					//если не зарезервировано, то создаем новую плату с этим адресом
		}

	}
	return brd.size();
}



