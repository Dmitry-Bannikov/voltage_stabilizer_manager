#pragma once
#include <GyverPortal.h>
#include <data.h>
//#include <service.h>


void GP_data_build();
void GP_target_build();
void GP_mainsets_build(Board &brd);
void GP_addsets_build(Board &brd);

void GP_details_start(const String &summary, const String &id);
void GP_details_end();



void GP_details_start(const String &summary, const String &id = "") {
    String s;
    s += F("<details style='margin: 1%; padding: 1%;'");
    if (id != "") {
        s += F("id = '");
        s += F("' ");
    }
    s += F(">\n");
    s += F("<summary>\n");
    s += summary;
    s += F("\n</summary>");
    GP.SEND(s);
}

void GP_details_end() {
    String s;
    s += F("\n</details>");
    GP.SEND(s);
}

void GP_data_build() {
	GP.GRID_BEGIN();
	if (board.size() == 0) {
		GP.BLOCK_BEGIN(GP_DIV_RAW);
		GP.TITLE("Не обнаружено подключенных плат!");
		M_BOX(
			GP.BUTTON("rst_btn", "Перезапустить\n систему");
			GP.BUTTON("scan_btn", "Пересканировать\n платы");
		);
		GP.BLOCK_END();
	} else {
		for (uint8_t i = 0; i < board.size(); i++) {
			GP.BLOCK_BEGIN(GP_DIV_RAW, "100%");
			GP.AREA(String("b_data/")+i, 6, "Загрузка \nданных...");
			GP.AREA(String("b_stat/")+i, 7, "Загрузка \nстатистики...");
			GP.BUTTON_MINI(String("r_stat/")+i, "Сброс");
			GP.BLOCK_END();
		}
	}
	
	GP.GRID_END();
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
				GP.BUTTON("svlit_btn", "Сохранить");
			} else {
				GP.TITLE("Не обнаружено подключенных плат!");
			}
			
			GP.RELOAD("reload");
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);//назначение букв
			for (uint8_t i = 0; i < board.size(); i++) {
				String label = String("Плата ") + board[i].getAddress();
				uint8_t select = 0;
				if (board[i].getLiteral() == "A") select = 1;
				else if (board[i].getLiteral() == "B") select = 2;
				else if (board[i].getLiteral() == "C") select = 3;
				GP.BOX_BEGIN();
				GP.LABEL(label);
				GP.SELECT(String("brdLit/")+i, "Нет,A,B,C", select); 
				GP.BOX_END();
			}
			
		GP.BLOCK_END();

	GP.GRID_END();

}

void GP_mainsets_build(Board &brd) {
	//String targetV_list = brd.getTargetVList();
	//String tcRatio_list = brd.getTcRatioList();
	//String motKoef_list = brd.getMotKoefList();
	//String maxCurr_list = brd.getMaxCurrList();
	//------------------------------------------------//
	String title = "Настройка платы ";
	title += (brd.mainSets.liter > 0 ? brd.getLiteral() : String(brd.getAddress()));
	GP.BLOCK_BEGIN(GP_DIV_RAW, "", title);
	M_BOX(
		GP.BUTTON_MINI("rboard_btn", "Прочитать из платы"); 
		GP.BUTTON_MINI("wset_btn", "Записать в плату")
	);
	M_BOX(GP_EDGES, GP.LABEL("Транзит при перегрузке"); GP.CHECK("aset_transit", brd.addSets.overloadTransit));
	M_BOX(GP_EDGES, GP.LABEL("Точность/ гистерезис");  GP.NUMBER("mset_prec", "", brd.mainSets.precision, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Подстройка входа");  GP.NUMBER("mset_tunIn", "", brd.mainSets.tuneInVolt, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Подстройка выхода");  GP.NUMBER("mset_tunOut", "", brd.mainSets.tuneOutVolt, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Целевое напряжение");  GP.NUMBER("mset_targetV", "", brd.mainSets.targetVoltage, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Коэффициент трансформатора");  GP.SELECT("mset_tcratio_idx", "25,40,50,60,80,100", brd.mainSets.transRatioIndx));
	M_BOX(GP_EDGES, GP.LABEL("Тип мотора"); GP.SELECT("mset_mottype", "20,90,150,200", brd.mainSets.motorType) );
	M_BOX(GP_EDGES, GP.LABEL("Ток выхода макс"); GP.NUMBER("mset_maxcurr", "", brd.mainSets.maxCurrent, "100px") );
	GP.GRID_BEGIN();
		GP.BUTTON_MINI("mset_disreg", "Переключить регуляцию");
		GP.BUTTON_MINI("rst_btn", "Перезапустить ESP");
		GP.BUTTON_MINI("mset_reboot", "Перезагрузить плату");	
	GP.GRID_END();
	GP.BLOCK_END();
}

void GP_addsets_build(Board &brd) {
	String title = "Настройка платы ";
	title += (brd.mainSets.liter > 0 ? brd.getLiteral() : String(brd.getAddress()));
	GP.BLOCK_BEGIN(GP_DIV_RAW, "", title);
	
	M_BOX(GP_EDGES, GP.LABEL("Максимальное напряжение");  GP.NUMBER("aset_maxV", "", brd.addSets.maxVolt, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Минимальное напряжение");  GP.NUMBER("aset_minV", "", brd.addSets.minVolt, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Время отключения, мс");  GP.NUMBER("aset_toff", "", brd.addSets.emergencyTOFF, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Время включения, мс");  GP.NUMBER("aset_ton", "", brd.addSets.emergencyTON, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Отключить регуляцию"); GP.CHECK("aset_disreg", (bool)(brd.addSets.Switches[SW_REGDIS])));
	M_BOX(GP_EDGES, GP.LABEL("Включить алярм"); GP.CHECK("aset_alarm", (bool)(brd.addSets.Switches[SW_ALARM])));
	//M_BOX(GP_EDGES, GP.LABEL("Коэф. моторов, %"); GP.TEXT("aset_motpwm", "30,100,150 и тд", brd.getMotKoefList(), "200px"); );
	//M_BOX(GP_EDGES, GP.LABEL("Максимальные токи, A"); GP.TEXT("aset_maxcurr", "35,40,50 и тд", brd.getMaxCurrList(), "200px"); );
	//M_BOX(GP_EDGES, GP.LABEL("Трансформаторы тока, X/5"); GP.TEXT("aset_tcratio", "40,50,60 и тд", brd.getTcRatioList(), "200px"); );
	
	GP.BLOCK_END();
}















