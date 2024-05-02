#include "customs.h"
#include "devices.h"

void GP_data_build() {
	GP.GRID_BEGIN();
	if (board.size() == 0) {
		GP.BLOCK_BEGIN(GP_DIV_RAW);
		GP.TITLE("Не обнаружено подключенных плат!");
		M_BOX(
			GP.BUTTON_MINI("btnbtn_sys_rescan_sys_reboot", "Перезапустить\n систему");
			GP.BUTTON_MINI("", "Пересканировать\n платы");
		);
		GP.BLOCK_END();
	} else {
		for (uint8_t i = 0; i < board.size(); i++) {
			GP.BLOCK_BEGIN(GP_THIN, "100%", String("Плата ")+ board[i].getLiteral());
			GP.AREA(String("b_data/")+i, 7, "Загрузка \nданных...");
			GP.AREA(String("b_stat/")+i, 13, "Загрузка \nстатистики...");
			M_BOX(GP_AROUND,
				GP.BUTTON_MINI(String("btn_brd_rst/")+i, "Сброс");
				GP.LED(String("b_led/")+i, board[i].isOnline());
			);
			GP.BLOCK_END();
		}
	}
	
	GP.GRID_END();
	if (board.size()) GP.BUTTON_MINI("btn_sys_reboot", "Пересканировать\n платы");
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
				String label = "Плата " + board[i].getAddress();
				uint8_t select = 0;
				if (board[i].getLiteral() == 'A') select = 1;
				else if (board[i].getLiteral() == 'B') select = 2;
				else if (board[i].getLiteral() == 'C') select = 3;
				GP.BOX_BEGIN();
				GP.LABEL(label);
				GP.SELECT("btn_brd_lit/"+i, "Нет,A,B,C", select); 
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

void GP_SUBMIT_MINI_LINK(const String &text, const String &link, PGM_P st, const String &cls) {
    *_GPP += F("<input type='submit' onclick='openNewTab()' value='");
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

void GP_CreateDevicesList() {
	for (uint8_t i = 0; i < Device_GetAmount(); i++) {
		GP_DeviceInfo(
			Device_Get(i, DEV_NAME).c_str(), 
			Device_Get(i, DEV_TYPE).c_str(),
			Device_Get(i, DEV_PAGE).c_str(),
			Device_Get(i, DEV_ISACT).c_str(),
			i
		);
	}
	
}

void GP_DeviceInfo(const char *name, const char *type, const char *page, const char *is_active, int num) {
	GP.GRID_BEGIN();
		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.TEXT("dev_name/" + String(num), "Name", name, "", 20);
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.TEXT("dev_type/" + String(num), "Type", type, "", 20);
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.LABEL(atoi(is_active) ? "В работе" : "Отключен");
			GP.LED("", atoi(is_active) ? true : false);
			GP.NUMBER("", "Hours", atoi(is_active), "20px");
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.BUTTON_MINI("dev_edit_btn_" + String(num), "Изменить");
			GP.BUTTON_MINI("dev_delete_btn_" + String(num), "Удалить");
			GP.BUTTON_MINI_LINK(page, "Открыть");
		GP.BLOCK_END();
	GP.GRID_END();
}
































//========================================


