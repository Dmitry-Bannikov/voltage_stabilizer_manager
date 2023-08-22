#pragma once

#include <customs.h>
#include <LittleFS.h>
#include <service.h>


void portalBuild();
void portalActions(GyverPortal &p);
void portalInit();
void portalTick();

GyverPortal ui(&LittleFS);


void portalBuild() {
  //------------------------------------------//
	GP.BUILD_BEGIN(850);
	GP.THEME(GP_LIGHT);
	String update;
	for (int i = 0; i2c_boards_addrs[i] != 0; i++) {
		update += "b_data/";
		update += i;
		update += ",";
	}
	for (int i = 0; i2c_boards_addrs[i] != 0; i++) {
		update += "b_stat/";
		update += i;
		update += ",";
	}
	GP.UPDATE(update);

	GP.GRID_RESPONSIVE(550); // Отключение респонза при узком экране
	GP.PAGE_TITLE("stab_manager");
	if (!wifi_settings.staModeEn) {
		GP.TITLE("WIFI Board Manager (AP)");
	} else {
		GP.TITLE("WIFI Board Manager (STA)");
	}
	GP.HR();
	GP.NAV_TABS("Home,Board Settings,WiFi Settings");
	GP.BREAK();
	
	GP.NAV_BLOCK_BEGIN();
		GP.TITLE("Board Data and Stats");
		GP.HR();
		GP.BLOCK_BEGIN("Board Data");
		GP_build_data();
		GP.BLOCK_END();
		GP.HR();
		GP.BLOCK_BEGIN("Board Stats");
		GP_build_stats();
		GP.BLOCK_END();
	GP.NAV_BLOCK_END();

	GP.NAV_BLOCK_BEGIN();
		GP.TITLE("Board Settings");
		GP.HR();
		GP.FORM_BEGIN("brdcfg");
		M_BOX(
			GP.BLOCK_BEGIN();
			GP.SUBMIT("Save/Write Settings");
			GP.BUTTON("btn1", "Read Settings");
			GP.RELOAD_CLICK("btn1");
		);
		M_BOX(
			GP.LABEL("Ignore board settings");
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
		GP.BLOCK_BEGIN();
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
		GP.BLOCK_END();
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
		recall(TRIMS);
	}

	if (ui.update()) {
		if (ui.updateSub("b_data/")) {
			for (int i = 0; i2c_boards_addrs[i] != 0; i++) {
				String name = "b_data/";
				name += i;
				String text;
				board[i].getValuesStr(text);
				ui.updateString(name, text);
			}
		}

		if (ui.updateSub("b_stat/")) {
			for (int i = 0; i2c_boards_addrs[i] != 0; i++) {
				String name = "b_stat/";
				name += i;
				String text;
				board[i].getStatisStr(text);
				ui.updateString(name, text);
			}
		}
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
		remember(WIFI);
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
		remember(TRIMS);
		remember(BSETS);
		uint8_t attempts = 0;
		uint8_t success = 0;
		while (attempts < 5 || !success) {
			for (int i = 0; i2c_boards_addrs[i] != 0; i++) {
				bool res1 = board[i].sendTrimmers(gTrimmers);
				bool res2 = board[i].sendBSets(gBoardSets);
				if (res1 && res2) success =  1;
				else success = 0;
			}
			attempts++;
		}
		LED_switch(0);
	}
}

void portalInit() {
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
	} 
	// Иначе создаем свою сеть
	else {
		WiFi.mode(WIFI_AP);
		WiFi.softAP(wifi_settings.apSsid, wifi_settings.apPass);
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
}



















