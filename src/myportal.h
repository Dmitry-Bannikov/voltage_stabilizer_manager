#pragma once

#include <customs.h>
#include <LittleFS.h>
#include <service.h>


void portalBuild();
void portalActions();
void portalInit();
void portalTick();
void createUpdateList(String &list);
uint8_t formsHandler();
void clicksHandler();
void updatesHandler(uint8_t &txSuccess);

GyverPortal ui(&LittleFS);


void portalBuild() {
  //------------------------------------------//
	GP.BUILD_BEGIN(900);
	GP.ONLINE_CHECK(5000);
	GP.THEME(GP_LIGHT);
	String update = "";
	createUpdateList(update);
	GP.UPDATE(update);
	GP.ALERT("setsalt");
	GP.RELOAD("reload");
	GP.GRID_RESPONSIVE(770); // Отключение респонза при узком экране
	GP.PAGE_TITLE("stab_manager");
	if (!wifi_settings.staModeEn) {
		GP.TITLE("Менеджер плат(AP)");
	} else {
		GP.TITLE("Менеджер плат (STA)");
	}
	GP.NAV_TABS_LINKS("/,/brdcfg,/addcfg,/wificfg", "Главная,Настройки платы,Доп. настройки,Настройка подключения");
	if (ui.uri("/")) {
		GP.HR();
		GP.BLOCK_BEGIN(GP_THIN, "", "Основные данные и статистика");
			GP_data_build();
		GP.BLOCK_END();
	}

	if(ui.uri("/brdcfg")) {
		GP.BLOCK_BEGIN(GP_THIN, "", "Основные настройки");
		GP_target_build();
		if (board.size()) {
			GP_mainsets_build(board[activeBoard]);
		}
		GP.BLOCK_END();
	}

	if(ui.uri("/addcfg")) {
		GP.BLOCK_BEGIN(GP_THIN, "", "Дополнительные настройки");
		if (board.size()) {
			GP_addsets_build(board[activeBoard]);
		}
		GP.BLOCK_END();
	}
	
	if(ui.uri("/wificfg")) {
		GP.BLOCK_BEGIN(GP_THIN, "", "Сетевые настройки");
		GP.FORM_BEGIN("/wificfg");
		GP.GRID_BEGIN();
			GP.BLOCK_BEGIN(GP_DIV_RAW, "", "Настройка точки дступа");
				GP.TEXT("apSsid", "Login AP", wifi_settings.apSsid, "", 20);
				GP.PASS_EYE("apPass", "Password AP", wifi_settings.apPass, "", 20);
			GP.BLOCK_END();

			GP.BLOCK_BEGIN(GP_DIV_RAW);
				GP.SUBMIT_MINI("SUBMIT & RESTART");
			GP.BLOCK_END();
			
			GP.BLOCK_BEGIN(GP_DIV_RAW, "", "Подключение к WIFI");
				GP.TEXT("staSsid", "Login STA", wifi_settings.staSsid, "", 20);
				GP.PASS_EYE("staPass","Password STA", wifi_settings.staPass, "", 20);
			GP.BLOCK_END();
		GP.GRID_END();
		GP.BUTTON_LINK("/ota_update", "Обновление прошивки");
		GP.FORM_END();
		GP.BLOCK_END();
	}
	GP.BUILD_END();
}

void portalActions() {
	static uint8_t txSuccess = 0;
	uint8_t temp = formsHandler();
	if (temp == 1 || temp == 2) {
		txSuccess = temp;
	}
	
	updatesHandler(txSuccess);
	clicksHandler();	
}

void portalInit() {
	// Пытаемся подключиться к роутеру
	if (wifi_settings.staModeEn) {
		WiFi.mode(WIFI_STA);
		WiFi.begin(wifi_settings.staSsid, wifi_settings.staPass);
		int attemptCount = 0;
		while (WiFi.status() != WL_CONNECTED) {
			LED_blink(100);
			if (++attemptCount == 10) {
				LED_switch(0);
				wifi_settings.staModeEn = 0;  //переключаемся на режим точки доступа
				memoryWIFI.updateNow();       //сохраняемся
				ESP.restart();                //перезапускаем есп
				return;
			}
			delay(1000);
		}
	} 
	// Иначе создаем свою сеть
	else {
		WiFi.mode(WIFI_AP);
		WiFi.softAP(wifi_settings.apSsid, wifi_settings.apPass);
		delay(1000);
	}
	ui.attachBuild(portalBuild);
	ui.attach(portalActions);
	ui.start("stab_stm32");
	Serial.println(WiFi.localIP());
	ui.enableOTA("admin", "012343210");
}

void portalTick() {
	ui.tick();
}



void createUpdateList(String &list) {
	for (uint8_t i = 0; i < board.size(); i++) {
		list += String("b_data/")+i;
		list += ",";
		list += String("b_stat/")+i;
		list += ",";
	}
	list += "setsalt,";
	list += "reload";
}

uint8_t formsHandler() {
	uint8_t saveResult = 0;
	if (ui.form("/wificfg")) { // Если есть сабмит формы - копируем все в переменные
			
		ui.copyStr("apSsid", wifi_settings.apSsid);
		ui.copyStr("apPass", wifi_settings.apPass);
		ui.copyStr("staSsid", wifi_settings.staSsid);
		ui.copyStr("staPass", wifi_settings.staPass);
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

		
		//board[activeBoard].saveSettings();
		//saveResult = 1;
		//if (!board[activeBoard].sendAddSets()) saveResult = 2; 
  return saveResult;
}

void clicksHandler() {
	if (ui.clickUp("rset_btn") || ui.clickUp("rset_btn1")) {
		board[activeBoard].readSettings();
		webRefresh = true;
	}
	if (ui.clickUp("wset_btn") || ui.clickUp("wset_btn1")) {
		board[activeBoard].sendMainSets();
		board[activeBoard].sendAddSets();
		webRefresh = true;
	}
	if (ui.clickUp("rst_btn")) {
		ESP.restart();
	}
	if (ui.clickUp("scan_btn")) {
		scanNewBoards();
		webRefresh = true;
	}
	if (ui.clickUp("svlit_btn")) {
		board[activeBoard].saveSettings();
		webRefresh = true;
	}
	if (ui.clickSub("brdLit")) {
		String brdNum = ui.clickNameSub(1);
		uint8_t num = brdNum.toInt();
		String liter = ui.getString();
		board[num].setLiteral(liter);
	}
	
	ui.clickInt("b_sel", activeBoard);
	ui.clickBool("mset_ignor", board[activeBoard].mainSets.ignoreSetsFlag);
	ui.clickInt("mset_trgtV", board[activeBoard].mainSets.targetVolt);
	ui.clickInt("mset_prec", board[activeBoard].mainSets.precision);
	ui.clickInt("mset_tunIn", board[activeBoard].mainSets.tuneInVolt);
	ui.clickInt("mset_tunOut", board[activeBoard].mainSets.tuneOutVolt);
	ui.clickInt("mset_tratio", board[activeBoard].mainSets.transRatio);
	ui.clickInt("mset_mottype", board[activeBoard].mainSets.motorType);
	ui.clickInt("mset_relset", board[activeBoard].mainSets.relaySet);
		//board[activeBoard].saveSettings();
		//saveResult = 1;
		//if (!board[activeBoard].sendMainSets()) saveResult = 2;				
	ui.clickInt("aset_maxV", board[activeBoard].addSets.maxVoltRelative);
	ui.clickInt("aset_minV", board[activeBoard].addSets.minVoltRelative);
	ui.clickInt("aset_toff", board[activeBoard].addSets.emergencyTOFF);
	ui.clickInt("aset_ton", board[activeBoard].addSets.emergencyTON);
	ui.clickInt("aset_motK_0", board[activeBoard].addSets.motKoef_0);
	ui.clickInt("aset_motK_1", board[activeBoard].addSets.motKoef_1);
	ui.clickInt("aset_motK_2", board[activeBoard].addSets.motKoef_2);
	ui.clickInt("aset_motK_3", board[activeBoard].addSets.motKoef_3);	
}

void updatesHandler(uint8_t &txSuccess) {
	if (!ui.update()) return;
	if (ui.update("reload") && webRefresh) {
		webRefresh = false;
		ui.answer(1);
	}
	if (ui.update("setsalt")) {
		if (txSuccess == 1)
			ui.answer("Настройки сохранены!");
		if (txSuccess == 2) {
			ui.answer("Настройки сохранены и переданы!");
		}
		txSuccess = 0;
	}
	for (uint8_t i = 0; i < board.size(); i++) {
		board[i].getDataStr();
		board[i].getStatisStr();
		ui.updateString(String("b_data/") + i, board[i].mainData.Str);
		ui.updateString(String("b_stat/") + i, board[i].mainStats.Str);
	}
	
}
