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
void LED_blink(uint16_t period);
bool board_state_toStr(int16_t board_state, String& board_state_str);
void dataHandler();
uint8_t sendParams(const uint8_t slave_addr);
uint8_t requestGetParams(const uint8_t slave_addr);
uint8_t requestGetData(const uint8_t slave_addr);
uint8_t readDataAsync(const uint8_t slave_addr);
void flush_tx_buffer();
void flush_rx_buffer();
void memoryTick();



void portalBuild();
void portalActions(GyverPortal &p);
void portalInit();
void portalTick();

GyverPortal ui(&LittleFS);
EEManager memoryWIFI(wifi_settings, 20000);
EEManager memorySETS(gTrimmers, 20000);
//-----------Functions definitions---------------//

void LED_switch(bool state) {
  if (state) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
  }  
}

void LED_blink(uint16_t period) {
  static uint64_t tick = 0;
  static bool led_state = false;
  if (millis() - tick > period) {
    LED_switch(led_state = !led_state);
    tick = millis();
  }
}

void LED_blink(uint16_t period_on, uint16_t period_off) {
  static uint64_t tick = 0;
  static bool led_state = false;
  if (millis() - tick > (led_state?period_on:period_off)) {
    LED_switch(led_state = !led_state);
    tick = millis();
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
    s.remove(s.length() - 1);
  }
  board_state_str = s;
  return true;
}

void dataHandler() {
  static uint32_t tick = 0;
  if (millis() -  tick >= 1000) {
    requestGetData(I2C_BOARD_ADDR_1);
    readDataAsync(I2C_BOARD_ADDR_1);
    board_state_toStr(gData_stat, gData_stat_str);
    tick = millis();
  }
}

uint8_t sendParams(const uint8_t slave_addr) {
  flush_tx_buffer();
  *i2c_master_tx_buffer = I2C_SLAVE_GET_PARAMS;
  uint8_t size = sizeof(gTrimmers)/sizeof(gTrimmers[0]);
  for (uint8_t i = 0; i < size; i++) {
    *(i2c_master_tx_buffer + i + 1) = *(gTrimmers + i);
  }
  Wire.beginTransmission(slave_addr);
  Wire.write((uint8_t*)i2c_master_tx_buffer, sizeof(i2c_master_tx_buffer));
  uint8_t res = Wire.endTransmission();
  return res;
}

//-----------Запрос на получение настроек платы------------------//
uint8_t requestGetParams(const uint8_t slave_addr) {
  flush_tx_buffer();                                                              //очищаем буфер передачи
  *i2c_master_tx_buffer = I2C_SLAVE_SEND_PARAMS;                                  //передаем команду отправить параметры
  Wire.beginTransmission(slave_addr);                                             //начинаем передачу
  Wire.write((uint8_t*)i2c_master_tx_buffer, sizeof(i2c_master_tx_buffer));       //передаем
  Wire.endTransmission();
  uint8_t res = Wire.requestFrom(slave_addr, sizeof(i2c_master_rx_buffer));                                           //заканчиваем передачу
  #ifdef SERIAL_DEBUG
  Serial.println();
  Serial.print("Request Get parameters: ");
  Serial.println(res);
  #endif
  return res;
}

//-----------Запрос на получение данных--------------------------//
uint8_t requestGetData(const uint8_t slave_addr) {
  flush_tx_buffer();
  *i2c_master_tx_buffer = I2C_SLAVE_SEND_DATA;
  Wire.beginTransmission(slave_addr);
  Wire.write((uint8_t*)i2c_master_tx_buffer, sizeof(i2c_master_tx_buffer));
  Wire.endTransmission();
  uint8_t res = Wire.requestFrom(slave_addr, sizeof(i2c_master_rx_buffer));
  return res;
}

//-----------Функция чтения данных-------------------------------//
uint8_t readDataAsync(const uint8_t slave_addr) {
  uint8_t res = 0;
  flush_rx_buffer();
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
  requestGetParams(I2C_BOARD_ADDR_1);
  while(readDataAsync(I2C_BOARD_ADDR_1)) {
    if (++attemptCount == 5) {
      Serial.println();
      Serial.println("i2c connection failure!");
      break;
    }
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("Target Volt: ");
  Serial.println(gTrim_targetVolt);
}

void memoryInit() {
  pinMode(LED_BUILTIN, OUTPUT);     // Пара подмигиваний
  LED_switch(1);
  delay(30);
  LED_switch(0);
  EEPROM.begin(512);
  memoryWIFI.begin(0, MEMORY_KEY);
  memorySETS.begin(100, MEMORY_KEY);
}

void memoryTick() {
  memoryWIFI.tick();
  memorySETS.tick();
}

void connectionTick() {
  static uint32_t tmr = 0;
  static uint8_t mode = 0;
  if (millis() - tmr >= 1000) {
    mode = (mode < 2 ? mode + 1 : 0);
  }
  if (mode == 0) {
    requestGetData(I2C_BOARD_ADDR_1);
  }
  else {
    readDataAsync(I2C_BOARD_ADDR_1);
  }
}









//=====================PORTAL FUNCTIONS===============================//

void portalBuild() {
  //------------------------------------------//
  GP.BUILD_BEGIN(600);
  GP.THEME(GP_LIGHT);
  GP.UPDATE("inV,outV,outC,bState,staEn");

  GP.GRID_RESPONSIVE(650); // Отключение респонза при узком экране
  GP.PAGE_TITLE("stab_manager");
  if (networkConnectionMode == NET_MODE_AP) {
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
    M_BOX(GP.LABEL("Input Voltage");    GP.NUMBER("inV", "", gData_input, "", true);     );
    M_BOX(GP.LABEL("Output Voltage");   GP.NUMBER("outV", "", gData_output, "", true);   );
    M_BOX(GP.LABEL("Output Current");   GP.NUMBER("outC", "", gData_load, "", true);  );
    M_BOX(GP.LABEL("Board State");      GP.TEXT("bState", "", gData_stat_str, "", 20);   );
  GP.NAV_BLOCK_END();

  GP.NAV_BLOCK_BEGIN();
    GP.TITLE("Board Settings");
    GP.HR();
    GP.FORM_BEGIN("brdcfg");
    GP.SUBMIT("Save Settings");
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
      GP.BREAK(); 
      M_BOX(
        GP_CENTER, GP.LABEL("Use Router Network ");
        GP_CHECK_FUNC("staEn", wifi_settings.staModeEn, "toggleDiv()");
      );
    );
    
    GP_DIV_START("stasets", wifi_settings.staModeEn);
    M_BLOCK_TAB( 
      "STA-Mode config", 
      GP.TEXT("staSsid", "Login STA", wifi_settings.staSsid, "", 20);
      GP.TEXT("staPass", "Password STA", wifi_settings.staPass, "", 20);
    );
    GP_DIV_END();
    GP.FORM_END();
    M_BLOCK_TAB(           // Блок с OTA-апдейтом
      "ESP UPDATE",      // Имя + тип DIV
      GP.OTA_FIRMWARE(); // Кнопка с OTA начинкой
    );
  GP.NAV_BLOCK_END();
  GP_HIDE_BLOCK_SCRIPT("check_staEn", "stasets");
  GP.BUILD_END();

}

void portalActions(GyverPortal &p) {

  // if (ui.click()) {
  //   if (ui.click("m_type"))  stab_trim_save.mot_type = ui.getInt("m_type") + 1;
  //   if (ui.click("rel_bhvr"))  stab_trim_save.relBehavior = ui.getInt("rel_bhvr") - 1;
  //   ui.clickInt("constV", stab_trim_save.vconstOut);
  //   ui.clickInt("prec", stab_trim_save.vprecision);
  //   ui.clickInt("vtuneIn", stab_trim_save.vtuneIn);
  //   ui.clickInt("vtuneOut", stab_trim_save.vtuneOut);
  // }


  if (ui.update()) {
    ui.updateInt("inV", gData_input);
    ui.updateInt("outV", gData_output);
    ui.updateInt("outC", gData_load);
    ui.updateString("bState", gData_stat_str);
  }

  if (p.form("/netcfg")) { // Если есть сабмит формы - копируем все в переменные
    p.copyStr("apSsid", wifi_settings.apSsid);
    p.copyStr("apPass", wifi_settings.apPass);
    p.copyStr("staSsid", wifi_settings.staSsid);
    p.copyStr("staPass", wifi_settings.staPass);
    p.copyBool("staEn", wifi_settings.staModeEn);
    LED_switch(1);
    memoryWIFI.updateNow();
    delay(1000);
    ESP.restart();
  }

  if (p.form("/brdcfg")) {
    p.copyBool("ignoreSets", gTrim_ignoreSets);
    p.copyInt("prec", gTrim_precision);
    p.copyInt("tuneIn", gTrim_tuneIn);
    p.copyInt("tuneOut", gTrim_tuneOut);
    p.copyInt("targetV", gTrim_targetVolt);
    p.copyInt("mot_type", gTrim_motType);
    p.copyInt("rel_set", gTrim_relSet);
    p.copyInt("tcRatio", gTrim_tcRatio);
    LED_switch(1);
    memorySETS.update();
    sendParams(I2C_BOARD_ADDR_1);
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
  if (WiFi.status() == WL_CONNECTED) {
    LED_blink(3000);
  }
  else {
    LED_blink(50, 1000);
  }
}











































//---functions