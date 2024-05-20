#include "customs.h"
#include "devices.h"
#include "netconnection.h"

void GP_data_build() {
	GP.GRID_BEGIN();
	if (board.size() == 0) {
		GP.BLOCK_BEGIN(GP_DIV_RAW);
		GP.TITLE("Не обнаружено подключенных плат!");
		M_BOX(
			GP.BUTTON_MINI("btn_sys_reboot", "Перезапустить\n систему");
			GP.BUTTON_MINI("btn_sys_rescan", "Пересканировать\n платы");
		);
		GP.BLOCK_END();
	} else {
		for (uint8_t i = 0; i < board.size(); i++) {
			GP.BLOCK_BEGIN(GP_THIN, "100%", String("Плата ")+ board[i].getLiteral());
			GP.AREA(String("fld_data/")+i, 7, "Загрузка \nданных...", "", true);
			GP.AREA(String("fld_stat/")+i, 10, "Загрузка \nстатистики...", "", true);
			M_BOX(GP_AROUND,
				GP.BUTTON_MINI(String("btn_brd_rst/")+i, "Сброс");
				GP.LED(String("fld_online/")+i, board[i].isOnline());
			);
			GP.BLOCK_END();
		}
	}
	
	GP.GRID_END();
	if (board.size()) GP.BUTTON_MINI("btn_sys_rescan", "Пересканировать\n платы");
}

void GP_target_build() {
	GP.GRID_BEGIN();

		GP.BLOCK_BEGIN(GP_DIV_RAW);//выбор платы
			String list = "";
            for (uint8_t i = 0; i < board.size();i++) {	//формиуем список для селектора
				list += String("Плата ");
				list += board[i].getLiteral();
				list += "(";
				list += board[i].getAddress();
				list += "),";
			}
			list.remove(list.length() - 1);
			if (board.size() > 0) {
				GP.SELECT("btn_brd_active", list, activeBoard);
			}
        GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			if (board.size() > 0) {
				GP.BUTTON("btn_brd_saveall", "Сохранить");
			} else {
				GP.TITLE("Не обнаружено подключенных плат!");
			}
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);//назначение букв
			for (uint8_t i = 0; i < board.size(); i++) {
				String label = "Плата " + String(board[i].getAddress());
				uint8_t select = 0;
				if (board[i].getLiteral() == 'A') select = 1;
				else if (board[i].getLiteral() == 'B') select = 2;
				else if (board[i].getLiteral() == 'C') select = 3;
				GP.BOX_BEGIN();
				GP.LABEL(label);
				GP.SELECT(String("btn_brd_lit/")+i, "Нет,A,B,C", select); 
				GP.BOX_END();
			}
		GP.BLOCK_END();

	GP.GRID_END();

}

void GP_mainsets_build(Board &brd) {
	String motTypes_list;
	String tcRatio_list;
	brd.getMotKoefsList(motTypes_list, true);
	brd.getTcRatioList(tcRatio_list);

	//------------------------------------------------//
	String title = "Настройка платы ";
	title += (brd.mainSets.Liter > 0 ? String(brd.getLiteral()) : String(brd.getAddress()));
	GP.BLOCK_BEGIN(GP_DIV_RAW, "", title);
	GP.RELOAD("reload");
	M_BOX(
		GP.BUTTON_MINI("btn_brd_read", "Прочитать из платы"); 
		GP.BUTTON_MINI("btn_brd_write", "Записать в плату")
	);
	M_BOX(GP_EDGES, GP.LABEL("Транзит при перегрузке"); GP.CHECK("fld_set_transit", brd.mainSets.EnableTransit););
	M_BOX(GP_EDGES, GP.LABEL("Точность/ гистерезис");  GP.NUMBER("fld_set_prec", "", brd.mainSets.Hysteresis, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Подстройка входа");  GP.NUMBER("fld_set_tunIn", "", brd.mainSets.TuneInVolt, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Подстройка выхода");  GP.NUMBER("fld_set_tunOut", "", brd.mainSets.TuneOutVolt, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Целевое напряжение");  GP.NUMBER("fld_set_targetV", "", brd.mainSets.Target, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Ток выхода макс"); GP.NUMBER("fld_set_maxcurr", "", brd.mainSets.MaxCurrent, "70px"); );
	M_BOX(GP_EDGES, GP.LABEL("Коэффициент трансформатора");  GP.SELECT("fld_set_trIndx", tcRatio_list, brd.mainSets.TransRatioIndx););
	M_BOX(GP_EDGES, GP.LABEL("Тип мотора"); GP.SELECT("fld_set_mottype", motTypes_list, brd.mainSets.MotorType-1););
	
	GP.GRID_BEGIN();
		GP.BUTTON_MINI("btn_sys_reboot", "Перезапустить ESP");
		GP.BUTTON_MINI("btn_brd_reboot", "Перезагрузить плату");	
	GP.GRID_END();
	GP.BLOCK_END();
}

void GP_addsets_build(Board &brd) {
	String motKoefs_list;
	String tcRatio_list;
	brd.getMotKoefsList(motKoefs_list, false);
	brd.getTcRatioList(tcRatio_list);

	String title = "Доп. Настройка платы ";
	title += (brd.mainSets.Liter > 0 ? String(brd.getLiteral()) : String(brd.getAddress()));
	GP.BLOCK_BEGIN(GP_DIV_RAW, "", title);
	M_BOX(GP_EDGES, GP.LABEL("Коэф. моторов, %"); GP.TEXT("fld_set_motKoefs", "Через ,", motKoefs_list, "170px"); );
	M_BOX(GP_EDGES, GP.LABEL("Максимальное напряжение");  GP.NUMBER("fld_set_maxV", "", brd.mainSets.MaxVolt, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Минимальное напряжение");  GP.NUMBER("fld_set_minV", "", brd.mainSets.MinVolt, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Время отключения, мс");  GP.NUMBER("fld_set_toff", "", brd.mainSets.EmergencyTOFF, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Время включения, мс");  GP.NUMBER("fld_set_ton", "", brd.mainSets.EmergencyTON, "70px"););
	M_BOX(GP_EDGES, 
		GP.LABEL("Калибровочный ток:"); GP.NUMBER_F("fld_set_CValue", "", brd.CurrClbrtValue, 2, "70px");
		GP.LABEL("Коэффициент", "", GP_GRAY); GP.NUMBER_F("fld_set_CKoef", "", brd.CurrClbrtKoeff, 2, "70px");
		GP.BUTTON_MINI("btn_brd_saveCValue", "Применить");
	);
	M_BOX(GP_EDGES, GP.LABEL("Внеш. сигнал"); GP.CHECK("btn_brd_outsgn", (bool)(brd.addSets.Switches[SW_OUTSIGN])););

	GP.BLOCK_END();
}

void GP_CreateDevicesList() {
	for (uint8_t i = 0; i < Device_Size(); i++) {
		GP_DeviceInfo(i);
	}
	GP_DeviceInfo(Device_Size());
}


void GP_wificonnection_build() {
	GP.BLOCK_BEGIN(GP_THIN, "", "Сетевые настройки");
	GP.FORM_BEGIN("/wificfg");
		GP.GRID_BEGIN();
			GP.BLOCK_BEGIN(GP_DIV_RAW, "", "Настройка точки дступа");
				GP.TEXT("apSsid", "Login AP", wifi_settings.apSsid, "", 20);
				GP.PASS_EYE("apPass", "Password AP", wifi_settings.apPass, "", 20);
			GP.BLOCK_END();
			GP.BLOCK_BEGIN(GP_DIV_RAW, "", "Подключение к WIFI");
				GP.TEXT("staSsid", "Login STA", wifi_settings.staSsid, "", 20);
				GP.PASS_EYE("staPass","Password STA", wifi_settings.staPass, "", 20);
			GP.BLOCK_END();
			GP.BLOCK_BEGIN(GP_DIV_RAW);
				//String open = "http://" + String(globalData.webInterfaceDNS) + ".local/";
				//GP_SUBMIT_MINI_LINK("Запомнить", open);
				GP.SUBMIT_MINI("Запомнить");
			GP.BLOCK_END();
		GP.GRID_END();
		GP.BUTTON_LINK("/ota_update", "Обновление прошивки");
	GP.FORM_END();
	GP.BLOCK_END();
}

void GP_SUBMIT_MINI_LINK(const String &text, const String &link, PGM_P st, const String &cls) {
    *_GPP += F("<input type='submit' onclick='openNewTab(");
	*_GPP += link;
	*_GPP += F(")' value='");
    *_GPP += text;
    if (st != GP_GREEN) {
        *_GPP += F("' style='background:");
        *_GPP += FPSTR(st);
    }
    if (cls.length()) {
        *_GPP += F("' class='");
        *_GPP += cls;
    }
    *_GPP += F("' >\n");

    *_GPP += F("<script>");
    *_GPP += F("function openNewTab(linkText) {");
    *_GPP += F("var NewTab = window.open('");
    *_GPP += link;
    *_GPP += F("', '_blank');");
	*_GPP += F("NewTab.focus();");
    *_GPP += F("return false;}");
    *_GPP += F("</script>");
    GP.send();
}

void GP_DeviceInfo(int num) {
	bool New = num == Device_Size();
	String name = "";
	String type = "";
	String is_act = "0";
	String page = "";
	String sn = String(Board_SN);
	if (num < Device_Size()) {
		name = Device_Get(num, DEV_NAME);
		type = Device_Get(num, DEV_TYPE);
		is_act = Device_Get(num, DEV_ISACT);
		page = Device_Get(num, DEV_PAGE);
		sn = Device_Get(num, DEV_SN);
	}
	
	
	GP.GRID_BEGIN();
		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.TEXT(String("dev_name/") + num, "Name", name, "", 20);
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.TEXT(String("dev_type/") + num, "Type", type, "", 20);
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.TEXT(String("dev_sn/") + num, "SN", sn, "", 20, "", New ? false : true);
		GP.BLOCK_END();

		if (!New) {
			GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.BOX_BEGIN();
				GP.LED("", is_act.toInt() ? true : false);
				if (is_act.toInt() > 0)
					GP.NUMBER("", "Hours", is_act.toInt(), "30px");
			GP.BOX_END();
			GP.BLOCK_END();
		}
		

		GP.BLOCK_BEGIN(GP_DIV_RAW);
		GP.BOX_BEGIN();
			if (!New) {
				GP.BUTTON_MINI(String("dev_btn_edit/") + num, "Изменить");
				GP.BUTTON_MINI(String("dev_btn_delete/") + num, "Удалить", "", GP_RED);
				GP.BUTTON_MINI_LINK(page, "Открыть");
			} else {
				GP.BUTTON_MINI("dev_btn_add", "Добавить");
			}
		GP.BOX_END();
		GP.BLOCK_END();
	GP.GRID_END();
}

void GP_OwnerEdit_build() {
	String name = User_Get(OWN_NAME);
	String email = User_Get(OWN_EMAIL);
	String pass = User_Get(OWN_PASS);
	String code = User_Get(OWN_CODE);
	String status = User_Get(OWN_STATUS);

	GP.GRID_BEGIN();
		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.TEXT("own_name", "Name", name, "", 20);
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.TEXT("own_email", "Email", email, "", 20);
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.PASS_EYE("own_pass", "Pass", pass, "", 20);
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.TEXT("own_code", "Code", code, "", 20, "", status == "registred" ? true : false);
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
		GP.BOX_BEGIN();
		if (status != "" && status != "on_delete") {
			GP.BUTTON_MINI("own_btn_edit", "Изменить");
			GP.BUTTON_MINI("own_btn_delete", "Удалить", "", GP_RED);
		} else {
			GP.BUTTON_MINI("own_btn_reg", "Зарегистрировать", "", GP_GREEN);
		}
		GP.BOX_END();
		GP.BLOCK_END();
	GP.GRID_END();
}
































//========================================


