#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <data.h>
#include <EEManager.h>
#include <LittleFS.h>
#include <GyverPortal.h>
#include <custom_elements.h>

//------------Prototypes--------------//
void connectingInit();
void memoryInit();
void LED_switch(bool state);
void LED_blink(uint16_t period_on, uint16_t period_off);
bool board_state_toStr(int16_t board_state, String& board_state_str);
void dataHandler();
uint8_t makeRequestI2C(const uint8_t addr, const int16_t request);
uint8_t readDataAsync(const uint8_t slave_addr);
void flush_tx_buffer();
void flush_rx_buffer();
void memoryTick();
void getWorkTime(String& worktime);
float KVAcalculate(int pwr);




void portalBuild();
void portalActions(GyverPortal &p);
void portalInit();
void portalTick();

GyverPortal ui(&LittleFS);
EEManager memoryWIFI(wifi_settings, 20000);
EEManager memoryTRIMS(gTrimmers, 20000);
EEManager memoryBSETS(gBoardSets, 20000);
//-----------Functions definitions---------------//

void LED_switch(bool state) {
	if (state) {
		digitalWrite(LED_BUILTIN, LOW);
	}
	else {
		digitalWrite(LED_BUILTIN, HIGH);
	}  
}

void LED_blink(uint16_t period_on, uint16_t period_off = 0) {
  	static uint64_t tick = 0;
	static bool led_state = false;
	if (!period_off) {
		if (millis() - tick > period_on) {
		LED_switch(led_state = !led_state);
		tick = millis();
		}
	} else {
		if (millis() - tick > (led_state ? period_on : period_off)) {
		LED_switch(led_state = !led_state);
		tick = millis();
		}
	}
}

bool board_state_toStr(int16_t board_state, String& board_state_str) {
	if (board_state < 1) {
		board_state_str = "Board State OK";
		return false;
	}
	String s = "";
	for (uint8_t i = 0; i < 16; i++) {
		if (board_state & (1<<i)) {
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
	board_state_str = s;
	return true;
}

void dataHandler() {
	static uint32_t tick = 0;
	static uint8_t mode = 0;
	if (millis() -  tick >= 1000) {
		if (mode == 0) {
		makeRequestI2C(I2C_BOARD_ADDR_1, I2C_SLAVE_SEND_DATA);
		} else if (mode == 1) {
		makeRequestI2C(I2C_BOARD_ADDR_1, I2C_SLAVE_SEND_STAT);
		}
		
		readDataAsync(I2C_BOARD_ADDR_1);
		board_state_toStr(gData_stat, gData_stat_str);
		getWorkTime(workTime_str);
		mode = (mode < 1 ? mode+1 : 0);
		tick = millis();
	}
}

void getWorkTime(String& worktime) {
	int hours = gStat_workTime_m/60;
	int minutes = gStat_workTime_m % 60;
	int days = gStat_workTime_d;
	String s = "";
	s += String(days);
	s += "d, ";
	s += String(hours);
	s += "h, ";
	s += String(minutes);
	s += "m";
	worktime = s;
}

float KVAcalculate(int pwr) {
  	return (float)(pwr/PWR_ACCURACY/1000.0);
}

uint8_t makeRequestI2C(const uint8_t addr, const int16_t request) {
	flush_tx_buffer();
	*i2c_master_tx_buffer = request;
	if (request == I2C_SLAVE_GET_PARAMS) {
		uint8_t size = sizeof(gTrimmers) / sizeof(*gTrimmers);
		for (uint8_t i = 0; i < size; i++)
		{
		*(i2c_master_tx_buffer + i + 1) = *(gTrimmers + i);
		}
	}
	else if (request == I2C_SLAVE_GET_BSETS) {
		uint8_t size = sizeof(gBoardSets) / sizeof(*gBoardSets);
		for (uint8_t i = 0; i < size; i++)
		{
		*(i2c_master_tx_buffer + i + 1) = *(gBoardSets + i);
		}
	}
	Wire.beginTransmission(addr);
	Wire.write((uint8_t*)i2c_master_tx_buffer, sizeof(i2c_master_tx_buffer));       //передаем
	return Wire.endTransmission();
}


//-----------Функция чтения данных-------------------------------//
uint8_t readDataAsync(const uint8_t slave_addr) {
	uint8_t res = 0;
	flush_rx_buffer();
	Wire.requestFrom(slave_addr, sizeof(i2c_master_rx_buffer));
	uint8_t* p = reinterpret_cast<uint8_t*>(i2c_master_rx_buffer);
	if (Wire.available()) {
		Wire.readBytes(p, sizeof(i2c_master_rx_buffer));
	} else {
		return 1;
	}
	if (*i2c_master_rx_buffer == I2C_MASTER_GET_PARAMS) {
		int size = sizeof(gTrimmers)/sizeof(*gTrimmers);
		for (int i = 0; i < size; i++) {
		*(gTrimmers + i) = *(i2c_master_rx_buffer + i + 1);
		}
	}
	else if (*i2c_master_rx_buffer == I2C_MASTER_GET_DATA) {
		int size = sizeof(gData)/sizeof(*gData);
		for (int i = 0; i < size; i++) {
		*(gData + i) = *(i2c_master_rx_buffer + i + 1);
		}
	}
	else if (*i2c_master_rx_buffer == I2C_MASTER_GET_STAT) {
		int size = sizeof(gStatis)/sizeof(*gStatis);
		for (int i = 0; i < size; i++) {
		*(gStatis + i) = *(i2c_master_rx_buffer + i + 1);
		}
	}
	else {
		res = 1;
	}
	return res;
}

void flush_tx_buffer() {
  	memset(i2c_master_tx_buffer, 0, sizeof(i2c_master_tx_buffer));
}

void flush_rx_buffer() {
  	memset(i2c_master_rx_buffer, 0, sizeof(i2c_master_rx_buffer));
}

void connectingInit() {
	Wire.begin();
	Wire.setTimeOut(500);
	delay(10);
	Serial.begin(115200);
	delay(10);
	Serial.println();
	Serial.println("Wait for i2c connect.");
	int attemptCount = 0;
	makeRequestI2C(I2C_BOARD_ADDR_1, I2C_SLAVE_SEND_PARAMS);
	while(readDataAsync(I2C_BOARD_ADDR_1)) {
		if (++attemptCount == 5) {
		Serial.println();
		Serial.println("i2c connection failure!");
		break;
		}
		Serial.print(".");
		delay(1000);
	}
}

void memoryInit() {
	pinMode(LED_BUILTIN, OUTPUT);     // Пара подмигиваний
	LED_switch(1);
	delay(30);
	LED_switch(0);
	EEPROM.begin(512);
	memoryWIFI.begin(0, 127);
	memoryTRIMS.begin(100, MEMORY_KEY);
	memoryBSETS.begin(200, MEMORY_KEY);
}

void memoryTick() {
	memoryWIFI.tick();
	memoryTRIMS.tick();
	memoryBSETS.tick();
}

void connectionTick() {
	static uint32_t tmr = 0;
	static uint8_t mode = 0;
	if (millis() - tmr >= 1000) {
		mode = (mode < 2 ? mode + 1 : 0);
	}
}









//=====================PORTAL FUNCTIONS===============================//

void portalBuild() {
  //------------------------------------------//
	GP.BUILD_BEGIN(600);
	GP.THEME(GP_LIGHT);
	GP.UPDATE("inV,outV,outC,bState,fullPwr,staEn,maxFP,minFP,avgFP,workTime,prec,vtuneIn,vtuneOut,targetV,mot_type,rel_set,tcRatio");

	GP.GRID_RESPONSIVE(650); // Отключение респонза при узком экране
	GP.PAGE_TITLE("stab_manager");
	if (!wifi_settings.staModeEn) {
		GP.TITLE(GP.ICON_FILE("/ICONS/wifi.svg") + "WIFI Board Manager (AP)");
	} else {
		GP.TITLE(GP.ICON_FILE("/ICONS/wifi.svg") + "WIFI Board Manager (STA)");
	}
	GP.HR();
	GP.NAV_TABS("Home,Board Settings,WiFi Settings");
	GP.BREAK();
	
	GP.NAV_BLOCK_BEGIN();
		GP.TITLE("Board Data");
		GP.HR();
		M_BOX(GP.LABEL("Input Voltage, V");    GP.NUMBER("inV", "", 0, "", true);     );
		M_BOX(GP.LABEL("Output Voltage, V");   GP.NUMBER("outV", "", 0, "", true);   );
		M_BOX(GP.LABEL("Output Current, A");   GP.NUMBER_F("outC", "", 0, 2, "", true);     );
		M_BOX(GP.LABEL("Full Power, kVA");  GP.NUMBER_F("fullPwr", "", 0, 3, "", true); );
		M_BOX(GP.LABEL("Board State");      GP.TEXT("bState", "", gData_stat_str, "", 20);   );
		GP.HR();
		GP_details_start("Board Statistics");
		M_BOX(GP.LABEL("Max Power per Minute, kVA");   GP.NUMBER_F("maxFP", "", 0, 3, "", true);   );
		M_BOX(GP.LABEL("Min Power per Minute, kVA");   GP.NUMBER_F("minFP", "", 0, 3, "", true);   );
		M_BOX(GP.LABEL("Avg Power per Minute, kVA");   GP.NUMBER_F("avgFP", "", 0, 3, "", true);   );
		M_BOX(GP.LABEL("Work Time");    GP.TEXT("workTime", "", workTime_str, "", 15);     );
		GP_details_end();
	GP.NAV_BLOCK_END();

	GP.NAV_BLOCK_BEGIN();
		GP.TITLE("Board Settings");
		GP.HR();
		GP.FORM_BEGIN("brdcfg");
		M_BOX(
		GP.SUBMIT("Save/Write Settings");
		GP.BUTTON("btn1", "Read Settings");
		);
		M_BOX(
			GP_CENTER, GP.LABEL("Ignore board settings");
			GP.CHECK("ignoreSets", gTrim_ignoreSets);
		);
		M_BOX(GP.LABEL("Precision/ Hysterezis");    GP.NUMBER("prec", "", gTrim_precision);  );
		M_BOX(GP.LABEL("Tune Voltage Input");       GP.NUMBER("vtuneIn", "", gTrim_tuneIn);     );
		M_BOX(GP.LABEL("Tune Voltage Output");      GP.NUMBER("vtuneOut", "", gTrim_tuneOut);    );
		M_BOX(GP.LABEL("Target Voltage");           GP.NUMBER("targetV", "", gTrim_targetVolt);   );
		M_BOX(GP.LABEL("Motor Type");               GP.SELECT("mot_type", "TYPE_1,TYPE_2,TYPE_3,TYPE_4", gTrim_motType); );
		M_BOX(GP.LABEL("Relay Behavior");           GP.SELECT("rel_set", "OFF,ON,NO_OFF", gTrim_relSet);     );
		M_BOX(GP.LABEL("TC Ratio");                 GP.NUMBER("tcRatio", "", gTrim_tcRatio);   );
		GP.BREAK();
		GP_details_start("Additional Settings");
		M_BOX(GP.LABEL("Max Voltage Add");    		GP.NUMBER("vmaxterm", "", gBSets_vMaxTerm);  );
		M_BOX(GP.LABEL("Min Voltage Add");       	GP.NUMBER("vminterm", "", gBSets_vMinTerm);  );
		M_BOX(GP.LABEL("Emergency tOFF (ms)");      GP.NUMBER("emerToff", "", gBSets_emergToff); );
		M_BOX(GP.LABEL("Emergency tON (ms)");       GP.NUMBER("emerTon",  "", gBSets_emergTon);  );
		M_BOX(GP.LABEL("Motor 1 coeff");            GP.NUMBER("mot1koef", "", gBSets_mot1koef);  );
		M_BOX(GP.LABEL("Motor 2 coeff");           	GP.NUMBER("mot2koef", "", gBSets_mot2koef);  );
		M_BOX(GP.LABEL("Motor 3 coeff");            GP.NUMBER("mot2koef", "", gBSets_mot3koef);  );
		M_BOX(GP.LABEL("Motor 4 coeff");            GP.NUMBER("mot3koef", "", gBSets_mot4koef);  );
		GP_details_end();
		GP.HR();
		GP.FORM_END();
	GP.NAV_BLOCK_END();

	GP.NAV_BLOCK_BEGIN();
		GP.TITLE("Connection Config");
		GP.HR();
		GP.FORM_BEGIN("/netcfg");
		GP.SUBMIT("SUBMIT & RESTART"); 
		M_BLOCK_TAB( 
		"AP-Mode config", 
		GP.TEXT("apSsid", "Login AP", wifi_settings.apSsid, "", 20);
		GP.TEXT("apPass", "Password AP", wifi_settings.apPass, "", 20);
		);
		GP_details_start("STA-Mode config");
		GP.TEXT("staSsid", "Login STA", wifi_settings.staSsid, "", 20);
		GP.TEXT("staPass", "Password STA", wifi_settings.staPass, "", 20);
		GP_details_end();
		GP.FORM_END();
		M_BLOCK_TAB(           // Блок с OTA-апдейтом
		"ESP UPDATE",      // Имя + тип DIV
		GP.OTA_FIRMWARE(); // Кнопка с OTA начинкой
		);
	GP.NAV_BLOCK_END();
	GP.BUILD_END();

}

void portalActions(GyverPortal &p) {

	if (ui.clickUp("btn1")) {
		gTrim_ignoreSets = 0;
		makeRequestI2C(I2C_BOARD_ADDR_1, I2C_SLAVE_SEND_PARAMS);
		uint8_t attempts = 0;
		uint8_t error = 1;
		while (attempts < 5 || error) {
			error = readDataAsync(I2C_BOARD_ADDR_1);
			attempts++;
		}
	}

	if (ui.update()) {
		ui.updateInt("inV", gData_input);
		ui.updateInt("outV", gData_output);
		ui.updateFloat("outC", (float)(gData_load/PWR_ACCURACY));
		ui.updateFloat("fullPwr", KVAcalculate(gData_fullpwr));
		ui.updateFloat("maxFP", KVAcalculate(gStat_FullP_max));
		ui.updateFloat("avgFP", KVAcalculate(gStat_FullP_avg));
		ui.updateString("bState", gData_stat_str);
		ui.updateString("workTime", workTime_str);
	}

	if (p.form("/netcfg")) { // Если есть сабмит формы - копируем все в переменные
		p.copyStr("apSsid", wifi_settings.apSsid);
		p.copyStr("apPass", wifi_settings.apPass);
		p.copyStr("staSsid", wifi_settings.staSsid);
		p.copyStr("staPass", wifi_settings.staPass);
		String s = wifi_settings.staSsid;
		if (s.length() > 1) {
		wifi_settings.staModeEn = 1;
		} else {
		wifi_settings.staModeEn = 0;
		}
		LED_switch(1);
		memoryWIFI.updateNow();
		delay(1000);
		ESP.restart();
	}

	if (p.form("/brdcfg")) {
		p.copyBool("ignoreSets", gTrim_ignoreSets);
		p.copyInt("prec", 		 gTrim_precision);
		p.copyInt("tuneIn", 	 gTrim_tuneIn);
		p.copyInt("tuneOut", 	 gTrim_tuneOut);
		p.copyInt("targetV", 	 gTrim_targetVolt);
		p.copyInt("mot_type", 	 gTrim_motType);
		p.copyInt("rel_set", 	 gTrim_relSet);
		p.copyInt("tcRatio", 	 gTrim_tcRatio);

		p.copyInt("vmaxterm", gBSets_vMaxTerm);  
		p.copyInt("vminterm", gBSets_vMinTerm); 
		p.copyInt("emerToff", gBSets_emergToff); 
		p.copyInt("emerTon",  gBSets_emergTon);  
		p.copyInt("mot1koef", gBSets_mot1koef);  
		p.copyInt("mot2koef", gBSets_mot2koef);  
		p.copyInt("mot2koef", gBSets_mot3koef);  
		p.copyInt("mot3koef", gBSets_mot4koef);  

		LED_switch(1);
		memoryTRIMS.update();
		uint8_t attempts = 0;
		uint8_t error = 1;
		while (attempts < 5 || error) {
			uint8_t error1 = makeRequestI2C(I2C_BOARD_ADDR_1, I2C_SLAVE_GET_PARAMS);
			uint8_t error2 = makeRequestI2C(I2C_BOARD_ADDR_1, I2C_SLAVE_GET_BSETS);
			if (error1 == 0 && error2 == 0) error =  0;
			attempts++;
		}
		LED_switch(0);
	}
}

void portalInit() {
	static uint32_t connection_timer = 0;
	// Пытаемся подключиться к роутеру
	if (wifi_settings.staModeEn) {
		WiFi.mode(WIFI_STA);
		WiFi.begin(wifi_settings.staSsid, wifi_settings.staPass);
		Serial.println();
		Serial.print("Try to connect network.");
		int attemptCount = 0;
		while (WiFi.status() != WL_CONNECTED) {
		LED_blink(100);
		Serial.print(".");
		if (++attemptCount == 10) {
			Serial.println();
			Serial.println("Net connection failure! Restart with AP mode.");
			LED_switch(0);
			wifi_settings.staModeEn = 0;  //переключаемся на режим точки доступа
			memoryWIFI.updateNow();       //сохраняемся
			ESP.restart();                //перезапускаем есп
			return;
		}
		delay(1000);
		}
		Serial.println("Net connection success: ");
		Serial.print(WiFi.localIP());
		Serial.println();
		networkConnectionMode = NET_MODE_STA;
	} 
	// Иначе создаем свою сеть
	else {
		WiFi.mode(WIFI_AP);
		WiFi.softAP(wifi_settings.apSsid, wifi_settings.apPass);
		networkConnectionMode = NET_MODE_AP;
		Serial.println();
		Serial.print("WiFi AP mode started:");
		Serial.println(WiFi.softAPIP());
	}
	ui.attachBuild(portalBuild);
	ui.attach(portalActions);
	ui.start();
	ui.enableOTA();
	if (!LittleFS.begin()) Serial.println("FS Error");
	ui.downloadAuto(true);
}

void portalTick() {
	ui.tick();
	if (WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_MODE_AP) {
		LED_blink(1500);
	}
	else {
		LED_blink(50, 1000);
	}
}











































//---functions