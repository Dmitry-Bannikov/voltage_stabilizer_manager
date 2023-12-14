#include <Board.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <Wire.h>

//==================Public=================//

int8_t Board::StartI2C() {
	/*
	i2c_config_t config = { };
    config.mode = I2C_MODE_MASTER;
    config.sda_io_num = 21;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_io_num = 22;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = 100000; 
	config.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
	if (i2c_param_config(I2C_NUM_0, &config) != ESP_OK) return 1;
    if (i2c_driver_install(I2C_NUM_0, config.mode, 0, 0, 0) != ESP_OK) return 2;
	i2c_set_timeout(I2C_NUM_0, 1200000);
	*/
	
	Wire.begin();
	Wire.setTimeOut(100);
	
	return 0;
}

int8_t Board::StopI2C() {
    if (i2c_driver_delete(I2C_NUM_0)) return 1;
    gpio_reset_pin(GPIO_NUM_21);
    gpio_reset_pin(GPIO_NUM_22);
	gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL<<21) | (1ULL<<22);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    if (gpio_config(&io_conf)) return 2;
	return 0;
}


bool Board::attach(const uint8_t addr) {
	if (addr == 0 || addr > 127) return false;
	if (addr != _board_addr)
		_board_addr = addr;
	startFlag = true;
	return isOnline();
}

bool Board::isBoard(uint8_t addr) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t error = i2c_master_cmd_begin(0, cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);
	if (error) return false;
	uint8_t txbuf[sizeof(_txbuffer)] = {0x20, 0};
	uint8_t rxbuf[sizeof(_rxbuffer)] = {0};
	esp_err_t ret = i2c_master_write_read_device(0, addr, txbuf, sizeof(txbuf), rxbuf, sizeof(rxbuf), pdMS_TO_TICKS(20));
	if (ret != ESP_OK) return false;
	if (rxbuf[0] == 0x20 && rxbuf[1] == 0xF0) return true;
	return false;
}

void Board::waitForReady() {
	//delay(5);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_stop(cmd);
    esp_err_t error = i2c_master_cmd_begin(0, cmd, pdMS_TO_TICKS(25));
    i2c_cmd_link_delete(cmd);
}

bool Board::isOnline() {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_board_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t error = i2c_master_cmd_begin(0, cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);
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
	memset(_txbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_REQUEST_DATA;
	memset(_rxbuffer, 0, sizeof(_rxbuffer));
	esp_err_t ret = i2c_master_write_read_device(0, _board_addr, _txbuffer, sizeof(_txbuffer), _rxbuffer, sizeof(_rxbuffer), pdMS_TO_TICKS(100));
	if (ret != ESP_OK) return 3;
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
	memset(_txbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_REQUEST_MAINSETS;
	memset(_rxbuffer, 0, sizeof(_rxbuffer));
	esp_err_t ret = i2c_master_write_read_device(0, _board_addr, _txbuffer, sizeof(_txbuffer), _rxbuffer, sizeof(_rxbuffer), pdMS_TO_TICKS(100));
	if (ret != ESP_OK) return 3;
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
		(disconn < 5) ? (disconn++) : (disconn = 5, _disconnected = 1);
	} else {
		disconn = 0;
		_disconnected = 0;
	}
	return error;
}

uint8_t Board::sendMainSets(uint8_t attempts) {
	if (!startFlag) return 1;
	Board::waitForReady();
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_MAINSETS_START;
	mainSets.packData();
	addSets.packData();
	memcpy(_txbuffer+1, mainSets.buffer, mainSets.structSize);
	memcpy(_txbuffer+1 + mainSets.structSize, addSets.buffer, addSets.structSize);
  	esp_err_t ret = i2c_master_write_to_device(0, _board_addr, _txbuffer, sizeof(_txbuffer), pdMS_TO_TICKS(100));
	if (ret != ESP_OK) return 2;
	return 0;
}

uint8_t Board::sendCommand(uint8_t command, uint8_t value) {
	if (!startFlag) return 1;
	Board::waitForReady();
	addSets.Switches[command] = value;
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_SWITCHES_START;
	memcpy(_txbuffer+1, addSets.Switches, sizeof(addSets.Switches));
	esp_err_t ret = i2c_master_write_to_device(0, _board_addr, _txbuffer, sizeof(_txbuffer), pdMS_TO_TICKS(100));
	if (ret != ESP_OK) return 2;
	return 0;
}

uint8_t Board::sendCommand(uint8_t* command) {
	if (!startFlag) return 1;
	Board::waitForReady();
	memset(_rxbuffer, 0, sizeof(_txbuffer));
	*_txbuffer = I2C_SWITCHES_START;
	memcpy(_txbuffer+1, command, 8);
	esp_err_t ret = i2c_master_write_to_device(0, _board_addr, _txbuffer, sizeof(_txbuffer), pdMS_TO_TICKS(100));
	if (ret != ESP_OK) return 2;
	return 0;
}

void Board::getDataStr() {
	float full_pwr = mainData.Power/mainData.cosfi/1000.0;
	String s = "";
	s += F(" Данные ");

	s += F("\nU вход   : ");
	s += String(mainData.Uin);
	s += F("V");

	s += F("\nU выход  : ");
	s += String(mainData.Uout);
	s += F("V");

	s += F("\nI выход  : ");
	s += String(mainData.Current, 1);
	s += F("A");

	s += F("\nP полн.  : ");
	s += String(full_pwr, 1);
	s += F("kVA");

	//s += F("\nP актив. : ");
	//s += String(mainData.Power/1000.0, 1);
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

	float maxPwr = mainStats.Power[0]/1000.0;
	float avgPwr = mainStats.Power[1]/1000.0;
	String s = "";
	s += F(" Cтатистика ");
	s += F("\nt работы:");
	s += getWorkTime(mainStats.workTimeMins);
	
	s += F("\nU вх.макс : ");
	s += String(mainStats.Uin[0]);

	s += F("\nU вх.сред : ");
	s += String(mainStats.Uin[1]);

	s += F("\nU вх.мин  : ");
	s += String(mainStats.Uin[2]);

	s += F("\nU вых.макс: ");
	s += String(mainStats.Uout[0]);

	s += F("\nU вых.сред: ");
	s += String(mainStats.Uout[1]);

	s += F("\nU вых.мин : ");
	s += String(mainStats.Uout[2]);
	

	s += F("\nI макс, А : ");
	s += String(mainStats.Current[0], 1);
	

	s += F("\nI сред, А : ");
	s += String(mainStats.Current[1], 1);
	

	s += F("\nP макс,kVA: ");
	s += String(maxPwr,1);

	s += F("\nP сред,kVA: ");
	s += String(avgPwr,1);

	s += F("\nСобытия: ");
	s += errorsToStr(mainStats.boardEvents, EVENTS_SHORT);
	
	mainStats.Str = s;
}





String Board::getJsonData() {
	char json[200];
	float maxPwr = mainStats.Power[0]/1000.0;
	float avgPwr = mainStats.Power[1]/1000.0;

	sprintf(json, "{"
					"\"Uin\":\"%d\",\"Uout\":\"%d\",\"I\":\"%.1f\",\"P\":\"%.1f\","
					"\"Uin_avg\":\"%d\",\"Uout_avg\":\"%d\",\"I_avg\":\"%.1f\",\"P_avg\":\"%.1f\","
					"\"Uin_max\":\"%d\",\"Uout_max\":\"%d\",\"I_max\":\"%.1f\",\"P_max\":\"%.1f\","
					"\"work_h\":\"%d\""
					"}", mainData.Uin, mainData.Uout, mainData.Current, mainData.Power,
						mainStats.Uin[1],mainStats.Uout[1],mainStats.Current[1],avgPwr,
						mainStats.Uin[0],mainStats.Uout[0],mainStats.Current[0],maxPwr,
						mainStats.workTimeMins/60
					);
	String data = String(json);
	return data;
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
			if (!reserved) {
				brd.emplace_back(addr);					//если не зарезервировано, то создаем новую плату с этим адресом
				//return 1;//test
			}
		}

	}
	return brd.size();
}



