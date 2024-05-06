#include "webinterface.h" 
#include "netconnection.h"
#include "customs.h"
#include "common_data.h"
#include "devices.h"



int actionDevice = -1;

void portalBuild() {
  //------------------------------------------//
	GP.BUILD_BEGIN(900);
	GP.ONLINE_CHECK(6000);
	GP.THEME(GP_LIGHT);
	String update = "";
	createUpdateList(update);
	GP.UPDATE(update);
	GP.ALERT("setsalt");
	GP.RELOAD("reload");
	GP.GRID_RESPONSIVE(850); // Отключение респонза при узком экране
	GP.PAGE_TITLE("stab_manager");
	if (!wifi_settings.staModeEn) {
		GP.TITLE("Менеджер плат(AP)");
	} else {
		GP.TITLE("Менеджер плат (STA)");
	}
	GP.NAV_TABS_LINKS("/,/dashboard,/brdcfg,/wificfg", "Главная,Мониторинг,Настройки платы,Настройка подключения");
	
	if(ui.uri("/")) {
		GP.BLOCK_BEGIN(GP_THIN, "", "Регистрация пользователя");
		GP_OwnerEdit_build();
		GP.BLOCK_END();
		GP.BLOCK_BEGIN(GP_THIN, "", "Мои устройства");
		GP_CreateDevicesList();
		GP.BLOCK_END();
	} 
	
	if (ui.uri("/dashboard")) {
		dataReqDelay = false;
		GP.HR();
		GP.LABEL("Соединение по MQTT");
		GP.LED("mqttConnected_led", mqttConnected);
		GP.BLOCK_BEGIN(GP_DIV_RAW, "", "Основные данные и статистика");
			GP_data_build();
		GP.BLOCK_END();
	} else {
		dataReqDelay = true;
	}

	if(ui.uri("/brdcfg")) {
		GP.BLOCK_BEGIN(GP_THIN, "", "Настройки стабилизаторов");
		GP_target_build();
		if (board.size()) {
			GP_mainsets_build(board[activeBoard]);
			GP_addsets_build(board[activeBoard]);
		}
		GP.BLOCK_END();
	}
	if(ui.uri("/wificfg")) {
		GP_wificonnection_build();
	}
	GP.BUILD_END();
}

void portalActions() {
	formsHandler();	
	clicksHandler();
	updatesHandler();
	ActionsDevice_handler();
}

void portalInit() {
	ui.attachBuild(portalBuild);
	ui.attach(portalActions);
	ui.start(globalData.webInterfaceDNS);
	ui.enableOTA("admin", "1234");
	ui.onlineTimeout(5000);
}

void portalTick() {
	ui.tick();
}

//===================================================================================



//===================================================================================

void createUpdateList(String &list) {
	for (uint8_t i = 0; i < board.size(); i++) {
		list += String("fld_data/")+i;
		list += ",";
		list += String("fld_stat/")+i;
		list += ",";
		list += String("fld_online/")+i;
		list += ",";
	}
	list += "setsalt,reload,btn_brd_outsgn,mqttConnected_led,fld_set_CKoef";
}

void formsHandler() {
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
		LED_blink(1);
		wifi_updateCFG();
		WiFi_Reconnect();
		LED_blink(0);
	}

}

void clicksHandler() {
	if (ui.clickSub("btn_")) {
		buttons_handler();
	}
	if (ui.clickSub("fld_")) {
		fields_handler();
	}
	
}

void updatesHandler() {
	if (!ui.update()) return;
	ui.updateBool("btn_brd_outsgn", board[activeBoard].addSets.Switches[SW_OUTSIGN]);
	ui.updateBool("mqttConnected_led", mqttConnected);
	if (ui.update("setsalt")) {	//вызов алерта
		if (requestResult == 2)
		{
			ui.answer("Выполнено!");
		}
		requestResult = 0;
	}
	if (ui.update("reload") && webRefresh) { //перезагрузка страницы
		webRefresh = false;
		ui.answer(1);
	}
	for (uint8_t i = 0; i < board.size(); i++) {
		String dataStr, statStr;
		board[i].getDataStr(dataStr);
		board[i].getStatisStr(statStr);
		ui.updateString(String("fld_data/") + i, dataStr);
		ui.updateString(String("fld_stat/") + i, statStr);
		ui.updateBool(String("fld_online/") + i, board[i].isOnline());
	}
	ui.updateFloat("fld_set_CKoef", board[activeBoard].CurrClbrtKoeff, 2);
	
}

void buttons_handler() {
	if (ui.clickSub("btn_brd_lit")) {
		for (uint8_t i = 0; i < board.size();i++) {
			uint8_t num = 0;
			if (ui.clickInt(String("btn_brd_lit/")+i, num)) {
				if (num != 0) board[i].setLiteral(64+num);
				else board[i].setLiteral('N');
			}
		}
	}

	if (ui.click("btn_sys_reboot")) 	boardRequest = 1;					//esp restart
	if (ui.click("btn_sys_rescan")) 	boardRequest = 2;					//scan boards
	if (ui.click("btn_brd_saveall")) 	boardRequest = 3;					//save to all boards
	if (ui.clickInt("btn_brd_active", activeBoard)) boardRequest = 4;		//set an active board
	if (ui.clickBool("btn_brd_outsgn", board[activeBoard].addSets.Switches[SW_OUTSIGN])) boardRequest = 5;	//outsignal on active board
	
	if (ui.click("btn_brd_read") ) 		boardRequest = 10 + activeBoard;	//read settings from active board
	if (ui.click("btn_brd_write"))  	boardRequest = 20 + activeBoard; 	//write settings to active board
	if (ui.click("btn_brd_reboot")) 	boardRequest = 30 + activeBoard;	//reboot active board
	if (ui.clickSub("btn_brd_rst"))		boardRequest = 40 + ui.clickNameSub().toInt();	//reset statistic
	if (ui.click("btn_brd_saveCValue"))	boardRequest = 50 + activeBoard;	//submit current calibration
}

void fields_handler() {
	ui.clickBool("fld_set_transit", board[activeBoard].mainSets.EnableTransit);
	ui.clickInt("fld_set_targetV", board[activeBoard].mainSets.Target);
	ui.clickInt("fld_set_prec", board[activeBoard].mainSets.Hysteresis);
	ui.clickInt("fld_set_tunIn", board[activeBoard].mainSets.TuneInVolt);
	ui.clickInt("fld_set_tunOut", board[activeBoard].mainSets.TuneOutVolt);
	ui.clickInt("fld_set_tcratio_idx", board[activeBoard].mainSets.TransRatioIndx);
	
	ui.clickInt("fld_set_maxcurr", board[activeBoard].mainSets.MaxCurrent);
	ui.clickInt("fld_set_maxV", board[activeBoard].mainSets.MaxVolt);
	ui.clickInt("fld_set_minV", board[activeBoard].mainSets.MinVolt);
	ui.clickInt("fld_set_toff", board[activeBoard].mainSets.EmergencyTOFF);
	ui.clickInt("fld_set_ton", board[activeBoard].mainSets.EmergencyTON);
	ui.clickFloat("fld_set_CValue", board[activeBoard].CurrClbrtValue);
	if (ui.click("fld_set_mottype")) board[activeBoard].mainSets.MotorType = ui.getInt()+1;
	if (ui.click("fld_set_motKoefs")) board[activeBoard].setMotKoefsList(ui.getString());
		
}

void ActionsDevice_handler() {
	static String name = "";
	static String type = "";
	static String sn = "";
	int dev = -1;
	
	
	for (uint8_t i = 0; i <= Device_Size(); i++) {
		ui.clickString(String("dev_name/")+ i, name);
		ui.clickString(String("dev_type/")+ i, type);
		ui.clickString(String("dev_sn/")+ i, sn);
	}
	
	if (ui.clickSub("dev_btn_edit")) {
		dev = ui.clickNameSub().toInt();
		//name = ui.getString(String("dev_name/")+dev);
		//type = ui.getString(String("dev_type/")+dev);
		if (sn == "") sn = Device_Get(dev, DEV_SN);
		if (name != "" && type != "") {
			Device_AddOrUpdate(name.c_str(), type.c_str(), sn.c_str());
			Device_Save();
		}
		webRefresh = true;
	}
	if (ui.clickSub("dev_btn_delete")) {
		dev = ui.clickNameSub().toInt();
		Device_Delete(dev);
		Device_Save();
		webRefresh = true;
	}
	if (ui.click("dev_btn_add")) {
		dev = Device_Size();
		if (sn == "") sn = String(Board_SN);
		if (name != "" && type != "") {
			Device_AddOrUpdate(name.c_str(), type.c_str(), sn.c_str(), "", "/dashboard");
			Device_Save();
		}
		webRefresh = true;
	}

}

void ActionsOwner_handler() {
	
}




















//==================================================