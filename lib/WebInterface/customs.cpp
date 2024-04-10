#include "customs.h"

void GP_data_build() {
	GP.GRID_BEGIN();
	if (board.size() == 0) {
		GP.BLOCK_BEGIN(GP_DIV_RAW);
		GP.TITLE("Не обнаружено подключенных плат!");
		M_BOX(
			GP.BUTTON_MINI("rst_btn", "Перезапустить\n систему");
			GP.BUTTON_MINI("scan_btn", "Пересканировать\n платы");
		);
		GP.BLOCK_END();
	} else {
		for (uint8_t i = 0; i < board.size(); i++) {
			GP.BLOCK_BEGIN(GP_THIN, "100%", String("Плата ")+ board[i].getLiteral());
			GP.AREA(String("b_data/")+i, 7, "Загрузка \nданных...");
			GP.AREA(String("b_stat/")+i, 13, "Загрузка \nстатистики...");
			M_BOX(GP_AROUND,
				GP.BUTTON_MINI(String("r_stat/")+i, "Сброс");
				GP.LED(String("b_led/")+i, board[i].isOnline());
			);
			GP.BLOCK_END();
		}
	}
	
	GP.GRID_END();
	if (board.size()) GP.BUTTON_MINI("scan_btn", "Пересканировать\n платы");
}

void GP_target_build() {
	GP.GRID_BEGIN();

		GP.BLOCK_BEGIN(GP_DIV_RAW);//выбор платы
			String list = "";
            for (uint8_t i = 0; i < board.size();i++) {
				list += String("Плата ");
				list += board[i].getLiteral();
				list += "(";
				list += board[i].getAddress();
				list += "),";
			}
			list.remove(list.length() - 1);
			if (board.size() > 0) {
				GP.SELECT("b_sel", list, activeBoard, 0, 0, 1);
			}
			
        GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			if (board.size() > 0) {
				GP.BUTTON("saveall_btn", "Сохранить");
			} else {
				GP.TITLE("Не обнаружено подключенных плат!");
			}
			
			GP.RELOAD("reload");
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);//назначение букв
			for (uint8_t i = 0; i < board.size(); i++) {
				String label = String("Плата ") + board[i].getAddress();
				uint8_t select = 0;
				if (board[i].getLiteral() == 'A') select = 1;
				else if (board[i].getLiteral() == 'B') select = 2;
				else if (board[i].getLiteral() == 'C') select = 3;
				GP.BOX_BEGIN();
				GP.LABEL(label);
				GP.SELECT(String("brdLit/")+i, "Нет,A,B,C", select); 
				GP.BOX_END();
			}
			
		GP.BLOCK_END();

	GP.GRID_END();

}

void GP_mainsets_build(Board &brd) {
	String motTypes_list;
	String tcRatio_list;
	brd.getMotTypesList(motTypes_list, true);
	brd.getTcRatioList(tcRatio_list);

	//------------------------------------------------//
	String title = "Настройка платы ";
	title += (brd.mainSets.Liter > 0 ? String(brd.getLiteral()) : String(brd.getAddress()));
	GP.BLOCK_BEGIN(GP_DIV_RAW, "", title);
	M_BOX(
		GP.BUTTON_MINI("read_btn", "Прочитать из платы"); 
		GP.BUTTON_MINI("save_btn", "Записать в плату")
	);
	M_BOX(GP_EDGES, GP.LABEL("Транзит при перегрузке"); GP.CHECK("mset_transit", brd.mainSets.EnableTransit););
	M_BOX(GP_EDGES, GP.LABEL("Точность/ гистерезис");  GP.NUMBER("mset_prec", "", brd.mainSets.Hysteresis, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Подстройка входа");  GP.NUMBER("mset_tunIn", "", brd.mainSets.TuneInVolt, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Подстройка выхода");  GP.NUMBER("mset_tunOut", "", brd.mainSets.TuneOutVolt, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Целевое напряжение");  GP.NUMBER("mset_targetV", "", brd.mainSets.Target, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Ток выхода макс"); GP.NUMBER("mset_maxcurr", "", brd.mainSets.MaxCurrent, "70px"); );
	M_BOX(GP_EDGES, GP.LABEL("Коэффициент трансформатора");  GP.SELECT("mset_tcratio_idx", tcRatio_list, brd.mainSets.TransRatioIndx););
	M_BOX(GP_EDGES, GP.LABEL("Тип мотора"); GP.SELECT("mset_mottype", motTypes_list, brd.mainSets.MotorType-1););
	
	GP.GRID_BEGIN();
		GP.BUTTON_MINI("rst_btn", "Перезапустить ESP");
		GP.BUTTON_MINI("mset_reboot", "Перезагрузить плату");	
	GP.GRID_END();
	GP.BLOCK_END();
}

void GP_addsets_build(Board &brd) {
	String motKoefs_list;
	String tcRatio_list;
	brd.getMotTypesList(motKoefs_list, false);
	brd.getTcRatioList(tcRatio_list);

	String title = "Настройка платы ";
	title += (brd.mainSets.Liter > 0 ? String(brd.getLiteral()) : String(brd.getAddress()));
	GP.BLOCK_BEGIN(GP_DIV_RAW, "", title);
	M_BOX(GP_EDGES, GP.LABEL("Коэф. моторов, %"); GP.TEXT("aset_motKoefs", "Через ,", motKoefs_list, "170px"); );
	M_BOX(GP_EDGES, GP.LABEL("Максимальное напряжение");  GP.NUMBER("mset_maxV", "", brd.mainSets.MaxVolt, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Минимальное напряжение");  GP.NUMBER("mset_minV", "", brd.mainSets.MinVolt, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Время отключения, мс");  GP.NUMBER("mset_toff", "", brd.mainSets.EmergencyTOFF, "70px"););
	M_BOX(GP_EDGES, GP.LABEL("Время включения, мс");  GP.NUMBER("mset_ton", "", brd.mainSets.EmergencyTON, "70px"););
	M_BOX(GP_EDGES, 
		GP.LABEL("Калибровочный ток:"); GP.NUMBER_F("mset_CurrClbrValue", "", brd.CurrClbrtValue, 2, "70px");
		GP.LABEL("Коэффициент", "", GP_GRAY); GP.NUMBER_F("mset_CurrClbrKoeff", "", brd.CurrClbrtKoeff, 2, "70px");
	);
	M_BOX(GP_EDGES, GP.LABEL("Внеш. сигнал"); GP.CHECK("outsignal", (bool)(brd.addSets.Switches[SW_OUTSIGN])););
	//GP.TIME("t", ui.getSystemTime());
	
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
    *_GPP += F("function openNewTab() {");
    *_GPP += F("window.open('");
    *_GPP += link;
    *_GPP += F("', '_blank');");
    *_GPP += F("return false;}");
    *_GPP += F("</script>");
    GP.send();
}

void GP_CreateDevicesList() {
	UpdateDevice("tac4300ct", "example@mail.com", "http://stab_webserver.local/dashboard", "/", 4245242, 1234, 25, true);
    UpdateDevice("stab_brd", "example@mail.com", "stabilizer", "/dashboard", Board_SN, 1235, 21, true);
	for (uint8_t i = 0; i < Devices.size(); i++) {
		GP_DeviceInfo(Devices[i], i);
	}
	
}

void GP_DeviceInfo(device & Device, int num) {
    if (!Device.Reg) return;
	GP.GRID_BEGIN();
		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.TEXT("dev_name_" + String(num), "Name", Device.Name, "", 20);
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			if (Device.IsActive) {
				GP.LABEL("В работе");
				GP.LED("", true);
                GP.NUMBER("", "Hours", Device.IsActive, "20px");
			} else {
				GP.LABEL("Отключен");
				GP.LED("", false);
                GP.NUMBER("", "Hours", Device.IsActive, "20px", true);
			}
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.BUTTON_MINI("dev_edit_btn_" + String(num), "Изменить");
			GP.BUTTON_MINI("dev_delete_btn_" + String(num), "Удалить");
			GP.BUTTON_MINI_LINK(Device.Page, "Открыть");
		GP.BLOCK_END();
	GP.GRID_END();
}
































//========================================


