#pragma once

//================= Библиотеки ==================//
#include "data.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <GyverPortal.h>

GyverPortal ui(&LittleFS);

void build() {
  GP.BUILD_BEGIN(600);
  GP.THEME(GP_LIGHT);
  GP.GRID_RESPONSIVE(650); // Отключение респонза при узком экране
  GP.PAGE_TITLE("stab_manager");
  if (networkConnectionMode == NET_MODE_AP) {
     GP.LABEL(GP.ICON_FILE("/ICONS/wifi.svg") + "WIFI Board Manager (AP)");
    //GP.TITLE(GP.ICON_FILE("/ICONS/wifi.svg") + "WIFI Board Manager");
  } else {
     GP.LABEL(GP.ICON_FILE("/ICONS/wifi.svg") + "WIFI Board Manager (STA)");
    //GP.TITLE(GP.ICON_FILE("/ICONS/wifi.svg") + "WIFI Board Manager");
  }
  GP.HR();
  GP.NAV_TABS_LINKS("/,/sets,/wifi", "Home,Board Settings,WiFi Settings");
  GP.BREAK();
  GP.FORM_BEGIN("/cfg"); // Начало формы
  M_BLOCK(               // Общий блок-колонка для WiFi
      GP.SUBMIT("SUBMIT SETTINGS"); // Кнопка отправки формы
      M_BLOCK_TAB( // Конфиг для AP режима -> текстбоксы (логин + пароль)
          "AP-Mode", // Имя + тип DIV
          GP.TEXT("apSsid", "Login", wifi_settings.apSsid, "", 20);
          GP.BREAK();
          GP.TEXT("apPass", "Password", wifi_settings.apPass, "", 20);
          GP.BREAK(););
      M_BLOCK_TAB( // Конфиг для STA режима -> текстбоксы (логин + пароль)
          "STA-Mode", // Имя + тип DIV
          GP.TEXT("staSsid", "Login", wifi_settings.staSsid, "", 20);
          GP.BREAK();
          GP.TEXT("staPass", "Password", wifi_settings.staPass, "", 20);
          GP.BREAK(); 
          M_BOX(
            GP_CENTER, GP.LABEL("STA Enable");
            GP.SWITCH("staEn", wifi_settings.staModeEn);
          ););     
  );
  GP.FORM_END();
  M_BLOCK_TAB(           // Блок с OTA-апдейтом
      "ESP UPDATE",      // Имя + тип DIV
      GP.OTA_FIRMWARE(); // Кнопка с OTA начинкой
  );
  GP.BUILD_END(); // Конец билда страницы
}

void actions(GyverPortal &p) {
  if (p.form("/cfg")) { // Если есть сабмит формы - копируем все в переменные
    p.copyStr("apSsid", wifi_settings.apSsid);
    p.copyStr("apPass", wifi_settings.apPass);
    p.copyStr("staSsid", wifi_settings.staSsid);
    p.copyStr("staPass", wifi_settings.staPass);
    p.copyBool("staEn", wifi_settings.staModeEn);
    memoryWIFI.updateNow();
    ESP.restart();
  }
}

void hub_init() {
  static uint32_t connection_timer = 0;
  if (wifi_settings.staModeEn) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_settings.staSsid, wifi_settings.staPass);
    connection_timer = millis();
    while (WiFi.status() != WL_CONNECTED) {
      pinMode(LED_BUILTIN, OUTPUT);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      if (millis() - connection_timer >= 10000) {
        wifi_settings.staModeEn = 0;
        memoryWIFI.updateNow();
        ESP.restart();
      }
      yield();
    }
    Serial.println();
    Serial.println(WiFi.localIP());
    Serial.println();
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(LED_BUILTIN, INPUT);
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
  LittleFS.begin();
  ui.enableOTA();
}

void foo() {
  // somecode
}