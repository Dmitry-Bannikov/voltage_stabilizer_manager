#pragma once

//================= Библиотеки ==================//

#include <Arduino.h>
#include <EEManager.h>
#include <LittleFS.h>
#include <GyverPortal.h>
#include <custom_elements.h>
#include <data.h>
#include <functions.h>

GyverPortal ui(&LittleFS);
EEManager memoryWIFI(wifi_settings, 20000);
EEManager memorySETS(stab_trim_save, 20000);

void build() {

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
    M_BOX(GP.LABEL("Input Voltage");    GP.NUMBER("inV", "", stab_data_toprint.inputVoltage, "", true);     );
    M_BOX(GP.LABEL("Output Voltage");   GP.NUMBER("outV", "", stab_data_toprint.outputVoltage, "", true);   );
    M_BOX(GP.LABEL("Output Current");   GP.NUMBER_F("outC", "", stab_data_toprint.outputCurrent, 2, "", true); );
    M_BOX(GP.LABEL("Board State");      GP.TEXT("bState", "", board_state_str, "", 20);   );
  GP.NAV_BLOCK_END();

  GP.NAV_BLOCK_BEGIN();
    GP.TITLE("Board Settings");
    GP.HR();
    GP.FORM_BEGIN("brdcfg");
    GP.SUBMIT("Save Settings");
    M_BOX(GP.LABEL("Precision/ Hysterezis");    GP.NUMBER("prec", "", stab_trim_save.vprecision);  );
    M_BOX(GP.LABEL("Tune Voltage Input");       GP.NUMBER("vtuneIn", "", stab_trim_save.vtuneIn);     );
    M_BOX(GP.LABEL("Tune Voltage Output");      GP.NUMBER("vtuneOut", "", stab_trim_save.vtuneOut);    );
    M_BOX(GP.LABEL("Const Voltage");            GP.NUMBER("constV", "", stab_trim_save.vconstOut);   );
    M_BOX(GP.LABEL("Motor Type");               GP.SELECT("m_type", "TYPE_1,TYPE_2,TYPE_3,TYPE_4", startpwr_to_motortype(stab_trim_save.startpwr)); );
    M_BOX(GP.LABEL("Relay Behavior");           GP.SELECT("rel_bhvr", "OFF,ON,NO_OFF", stab_trim_save.relBehavior);     );
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

void actions(GyverPortal &p) {

  // if (ui.click()) {
  //   if (ui.click("m_type"))  stab_trim_save.mot_type = ui.getInt("m_type") + 1;
  //   if (ui.click("rel_bhvr"))  stab_trim_save.relBehavior = ui.getInt("rel_bhvr") - 1;
  //   ui.clickInt("constV", stab_trim_save.vconstOut);
  //   ui.clickInt("prec", stab_trim_save.vprecision);
  //   ui.clickInt("vtuneIn", stab_trim_save.vtuneIn);
  //   ui.clickInt("vtuneOut", stab_trim_save.vtuneOut);
  // }


  if (ui.update()) {
    ui.updateInt("inV", stab_data_toprint.inputVoltage);
    ui.updateInt("outV", stab_data_toprint.outputVoltage);
    ui.updateFloat("outC", stab_data_toprint.outputCurrent);
    ui.updateString("bState", board_state_str);
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
    int mot_type;
    p.copyInt("prec", stab_trim_save.vprecision);
    p.copyInt("tuneIn", stab_trim_save.vtuneIn);
    p.copyInt("tuneOut", stab_trim_save.vtuneOut);
    p.copyInt("constV", stab_trim_save.vconstOut);
    p.copyInt("m_type", mot_type);
    p.copyInt("rel_bhvr", stab_trim_save.relBehavior);
    stab_trim_save.startpwr = motortype_to_startpwr(mot_type);
    LED_switch(1);
    memorySETS.update();
    LED_switch(0);
  }
}

void hub_init() {
  static uint32_t connection_timer = 0;
  if (wifi_settings.staModeEn) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_settings.staSsid, wifi_settings.staPass);
    connection_timer = millis();
    while (WiFi.status() != WL_CONNECTED) {
      LED_blink(100);
      if (millis() - connection_timer >= 10000) {
        LED_switch(0);
        wifi_settings.staModeEn = 0;
        memoryWIFI.updateNow();
        ESP.restart();
      }
      yield();
    }
    Serial.println();
    Serial.println(WiFi.localIP());
    Serial.println();
    networkConnectionMode = NET_MODE_STA;
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(wifi_settings.apSsid, wifi_settings.apPass);
    networkConnectionMode = NET_MODE_AP;
    Serial.println();
    Serial.println("WiFi AP mode started");
    Serial.println();
  }
  ui.attachBuild(build);
  ui.attach(actions);
  ui.start();
  ui.enableOTA();
  if (!LittleFS.begin()) Serial.println("FS Error");
  ui.downloadAuto(true);
}