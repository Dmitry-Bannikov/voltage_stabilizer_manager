#include "netconnection.h"
#include <common_data.h>

wifisets wifi_settings;
global_vars globalData;
EEManager memoryWIFI(wifi_settings, 20000);



void WifiInit() {
    EEPROM.begin(512);
	memoryWIFI.begin(0, MEMORY_KEY);
	WiFi_Reconnect();
}

void wifi_tick() {
	static uint32_t tmr = 0;
	memoryWIFI.tick();
	if (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA) {
		LED_blink(100, 2000);
	} else if (WiFi.getMode() == WIFI_AP){
		LED_blink(1000);
	}
	if (millis() - tmr > 45000) {
		WiFi_Reconnect();
		tmr = millis();
	}
}

void wifi_updateCFG() {
	memoryWIFI.updateNow();
}

void WiFi_Reconnect() {
	if (wifi_settings.staModeEn) {
		if (strcmp(wifi_settings.staSsid, "") && WiFi.status() != WL_CONNECTED) {
			WiFi.mode(WIFI_STA);
			WiFi.begin(wifi_settings.staSsid, wifi_settings.staPass);
			uint32_t tmr = millis();
			while (WiFi.status() != WL_CONNECTED) {
				LED_blink(250);
				if (millis() - tmr > 30000) break;
			}
		} else {
			wifi_settings.staModeEn = 0;
		} 
	} 
	if (!wifi_settings.staModeEn) {
		if (WiFi.getMode() != WIFI_AP) {
			WiFi.mode(WIFI_AP);
			WiFi.softAP(wifi_settings.apSsid, wifi_settings.apPass);
			delay(1000);
		}
		if (strcmp(wifi_settings.staSsid, "")) {
			wifi_settings.staModeEn = 1;
		}
	}
	if (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA) {
		Serial.print("\nLocal IP: ");
		Serial.println(WiFi.localIP());
	} else {
		Serial.print("\nSoft AP: ");
		Serial.println(WiFi.softAPIP());
	}
}









//=============================