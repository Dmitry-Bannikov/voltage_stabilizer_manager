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
	GP.ONLINE_CHECK(5000);
	GP.THEME(GP_LIGHT);
	String update;
	for (int i = 0; i < board.size(); i++) {
		update += "b_data/";
		update += i;
		update += ",";
	}
	for (int i = 0; i < board.size(); i++) {
		update += "b_stat/";
		update += i;
		update += ",";
	}
	update += "alt,";
	Serial.println(update);
	GP.UPDATE(update);
	GP.ALERT("alt");
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
		if (board.size() == 0) {
			M_BOX(GP_CENTER,
				GP.LABEL("Не обнаружено подключенных плат");
				GP.BUTTON_MINI("btn2", "Перезагрузить \nESP");
				GP.BUTTON_MINI("btn3", "Пересканировать");
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
	String num_width = "10%";
	GPalign align = GP_EDGES;
		GP.TITLE("Board Settings");
		GP.BLOCK_BEGIN("80%");
		GP.HR();
		GP.FORM_BEGIN("/brdcfg");
		GP_target_set();
		M_BOX(align, GP.LABEL("Ignore board settings");	   GP.CHECK ("trim/0", 	gTrimmers[0]);	);
		M_BOX(align, GP.LABEL("Precision/ Hysterezis");    GP.NUMBER("trim/1", "", gTrimmers[1], num_width);  	);
		M_BOX(align, GP.LABEL("Tune Voltage Input");       GP.NUMBER("trim/2", "", gTrimmers[2], num_width);	);
		M_BOX(align, GP.LABEL("Tune Voltage Output");      GP.NUMBER("trim/3", "", gTrimmers[3], num_width);	);
		M_BOX(align, GP.LABEL("Target Voltage");           GP.NUMBER("trim/4", "", gTrimmers[4], num_width);	);
		M_BOX(align, GP.LABEL("TC Ratio");                 GP.NUMBER("trim/7", "", gTrimmers[7], num_width);	);
		M_BOX(align, GP.LABEL("Relay Behavior");           GP.SELECT("trim/5", "OFF,ON,NO_OFF", gTrimmers[5]);     				);
		M_BOX(align, GP.LABEL("Motor Type");               GP.SELECT("trim/6", "TYPE_1,TYPE_2,TYPE_3,TYPE_4", gTrimmers[6]); 	);
		
		GP.BREAK();
		GP.HR();
		GP_details_start("Additional Settings");
		GP_block_begin();
		M_BOX(align, GP.LABEL("Max Voltage Add");		GP.NUMBER("bset/1", "", gBoardSets[1], num_width);  );
		M_BOX(align, GP.LABEL("Min Voltage Add");		GP.NUMBER("bset/0", "", gBoardSets[0], num_width);  );
		M_BOX(align, GP.LABEL("Emergency tOFF (ms)");	GP.NUMBER("bset/2", "", gBoardSets[2], num_width); 	);
		M_BOX(align, GP.LABEL("Emergency tON (ms)");	GP.NUMBER("bset/3", "", gBoardSets[3], num_width);  );
		M_BOX(align, GP.LABEL("Motor 1 coeff");			GP.NUMBER("bset/4", "", gBoardSets[4], num_width);  );
		M_BOX(align, GP.LABEL("Motor 2 coeff");			GP.NUMBER("bset/5", "", gBoardSets[5], num_width);  );
		M_BOX(align, GP.LABEL("Motor 3 coeff");			GP.NUMBER("bset/6", "", gBoardSets[6], num_width);  );
		M_BOX(align, GP.LABEL("Motor 4 coeff");			GP.NUMBER("bset/7", "", gBoardSets[7], num_width);  );
		GP_block_end();
		GP_details_end();
		GP.FORM_END();
		
		GP.BLOCK_END();
	GP.NAV_BLOCK_END();

	GP.NAV_BLOCK_BEGIN();
		GP_container_column("WIFI Settings");
		GP.HR();
		GP.FORM_BEGIN("/netcfg");
		GP_container_column();
		GP.SUBMIT_MINI("SUBMIT & RESTART"); 
		GP_details_start("AP-Mode config");
		GP_container_row();
			GP.TEXT("apSsid", "Login AP", wifi_settings.apSsid, "", 20);
			GP.PASS_EYE("apPass", "Password AP", wifi_settings.apPass, "", 20);
		GP_block_end();
		GP_details_end();
		GP.BREAK();
		GP_details_start("STA-Mode config");
		GP_container_row();
			GP.TEXT("staSsid", "Login STA", wifi_settings.staSsid, "", 20);
			GP.PASS_EYE("staPass","Password STA", wifi_settings.staPass, "", 20);
		GP_block_end();
		GP_details_end();
		GP_block_end();
		GP.FORM_END();
		GP.OTA_FIRMWARE(); // Кнопка с OTA начинкой
		GP.BLOCK_END();
	GP.NAV_BLOCK_END();
GP.BUILD_END();

}

void portalActions(GyverPortal &p) {
	if (!espStarted) {
		espStarted = true;
		ui.show();
		
	}
	static uint8_t txSuccess = 0;
	if (ui.clickUp("btn1")) {
		recall(TRIMS);
	}
	if (ui.clickUp("btn2")) {
		ESP.restart();
	}
	if (ui.clickUp("btn3")) {
		scanNewBoards();
	}

	if (ui.update()) {
		if (ui.update("alt")) {
			if (txSuccess == 1)
				ui.answer("Настройки сохранены!");
			if (txSuccess == 2) {
				ui.answer("Настройки сохранены и переданы!");
			}
			txSuccess = 0;
		}
		for (int i = 0; i < board.size(); i++) {
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
		txSuccess = 0;
		//=== Формируем имена для сохранения с формы ===//
		p.copyBool("trim/0", gTrimmers[0]);	
		for (int i = 1; i < 8; i++) {
			p.copyInt(String("trim/")+i, gTrimmers[i]);
		}
		for (int i = 0; i < 8; i++) {
			p.copyInt(String("bset/")+i, gBoardSets[i]);
		}
		for (int i = 0; i < board.size(); i++) {
			p.copyBool(String("btarget/")+i, i2c_active_board[i]);
		}
		//сохраняем в епром
		LED_switch(1);
		remember(TRIMS);
		remember(BSETS);
		txSuccess = 1;
		uint8_t attempts = 0;
		//отправляем на плату
		while (txSuccess != 2) {
			for (int i = 0; i < board.size() || i2c_active_board[i] != 0; i++) {
				uint8_t res1 = board[i].sendTrimmers(gTrimmers);
				uint8_t res2 = board[i].sendBSets(gBoardSets);
				if (!res1 && !res2) txSuccess = 2;
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
	ui.start("stabilizator_stm32");
	ui.enableOTA();
	if (!LittleFS.begin()) Serial.println("FS Error");
	ui.downloadAuto(true);
}

void portalTick() {
	static uint32_t tmr = 0;
	if (millis() - tmr > 1000) {
		for (int i = 0; i < board.size(); i++)
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



















