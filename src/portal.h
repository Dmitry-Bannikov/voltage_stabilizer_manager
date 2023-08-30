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
	GP.BUILD_BEGIN(900);
	GP.THEME(GP_LIGHT);
	String update;
	for (int i = 0; i < gNumBoards; i++) {
		update += "b_data/";
		update += i;
		update += ",";
	}
	for (int i = 0; i < gNumBoards; i++) {
		update += "b_stat/";
		update += i;
		update += ",";
	}
	Serial.println(update);
	GP.UPDATE(update);

	GP.GRID_RESPONSIVE(600); // Отключение респонза при узком экране
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
		if (gNumBoards == 0) {
			M_BOX(GP_AROUND,
				GP.LABEL("Плата не отвечает!");
				GP.BUTTON("btn2", "Restart Board");
			);
		}
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
			GP.SUBMIT("Save/Write Settings");
			
			GP.BUTTON("btn1", "Read Settings");
			GP.RELOAD_CLICK("btn1");
		);
		M_BOX(GP_AROUND, GP.LABEL("Ignore board settings");	GP.CHECK ("trim/0", 	gTrimmers[0]);	);
		M_BOX(GP_AROUND, GP.LABEL("Precision/ Hysterezis");    GP.NUMBER("trim/1", "", gTrimmers[1], "20"); );
		M_BOX(GP_AROUND, GP.LABEL("Tune Voltage Input");       GP.NUMBER("trim/2", "", gTrimmers[2], "20");	);
		M_BOX(GP_AROUND, GP.LABEL("Tune Voltage Output");      GP.NUMBER("trim/3", "", gTrimmers[3], "20");	);
		M_BOX(GP_AROUND, GP.LABEL("Target Voltage");           GP.NUMBER("trim/4", "", gTrimmers[4], "20");	);
		M_BOX(GP_AROUND, GP.LABEL("TC Ratio");                 GP.NUMBER("trim/7", "", gTrimmers[7], "20");	);
		M_BOX(GP_AROUND, GP.LABEL("Relay Behavior");           GP.SELECT("trim/5", "OFF,ON,NO_OFF", gTrimmers[5]);     				);
		M_BOX(GP_AROUND, GP.LABEL("Motor Type");               GP.SELECT("trim/6", "TYPE_1,TYPE_2,TYPE_3,TYPE_4", gTrimmers[6]); 	);
		
		GP.BREAK();
		
		GP_details_start("Additional Settings");
		GP.BLOCK_BEGIN();
		M_BOX(GP_AROUND, GP.LABEL("Max Voltage Add");		GP.NUMBER("bset/1", "", gBoardSets[1]);  );
		M_BOX(GP_AROUND, GP.LABEL("Min Voltage Add");		GP.NUMBER("bset/0", "", gBoardSets[0]);  );
		M_BOX(GP_AROUND, GP.LABEL("Emergency tOFF (ms)");	GP.NUMBER("bset/2", "", gBoardSets[2]); );
		M_BOX(GP_AROUND, GP.LABEL("Emergency tON (ms)");	GP.NUMBER("bset/3", "", gBoardSets[3]);  );
		M_BOX(GP_AROUND, GP.LABEL("Motor 1 coeff");		GP.NUMBER("bset/4", "", gBoardSets[4]);  );
		M_BOX(GP_AROUND, GP.LABEL("Motor 2 coeff");		GP.NUMBER("bset/5", "", gBoardSets[5]);  );
		M_BOX(GP_AROUND, GP.LABEL("Motor 3 coeff");		GP.NUMBER("bset/6", "", gBoardSets[6]);  );
		M_BOX(GP_AROUND, GP.LABEL("Motor 4 coeff");		GP.NUMBER("bset/7", "", gBoardSets[7]);  );
		GP.BLOCK_END();
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
		recall(TRIMS);
	}
	if (ui.clickUp("btn2")) {
		ESP.restart();
	}


	if (ui.update()) {
		for (int i = 0; i < gNumBoards; i++) {
			String data_name = "b_data/";
			String stat_name = "b_stat/";
			data_name += i;
			stat_name += i;
			ui.updateString(data_name, gBoard_data[i]);
			ui.updateString(stat_name, gBoard_stat[i]);
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
		p.copyBool("trim/0", gTrimmers[0]);
		for (int i = 1; i < 8; i++) {
			String name = "trim/";
			name += i;
			p.copyInt(name, gTrimmers[i]);
		}
		for (int i = 0; i < 8; i++) {
			String name = "bset/";
			name += i;
			p.copyInt(name, gBoardSets[i]);
		} 
		LED_switch(1);
		remember(TRIMS);
		remember(BSETS);
		uint8_t attempts = 0;
		uint8_t success = 0;
		while (!success) {
			for (int i = 0; i < gNumBoards; i++) {
				uint8_t res1 = board[i].sendTrimmers(gTrimmers);
				uint8_t res2 = board[i].sendBSets(gBoardSets);
				if (!res1 && !res2) success =  1;
				else success = 0;
			}
			attempts++;
			if (attempts == 5) break;
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
	static uint32_t tmr = 0;
	if (millis() - tmr > 1000) {
		for (int i = 0; i < gNumBoards; i++)
		{
			board[i].getDataStr(gBoard_data[i]);
			board[i].getStatisStr(gBoard_stat[i]);
			boards_worktime[i] = board[i].getWorkTime();
			//board[i].tick();
		}
		tmr = millis();
	}
	
	ui.tick();
}



















