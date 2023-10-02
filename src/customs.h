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
			GP.BUTTON("rst_btn", "Перезапустить ESP");
			GP.BUTTON("scan_btn", "Пересканировать платы");
		);
		GP.BLOCK_END();
	} else {
		for (uint8_t i = 0; i < board.size(); i++) {
			GP.BLOCK_BEGIN(GP_DIV_RAW);
			String value;
			if (board[i].getLiteral() == "") {
				value = String(board[i].getAddress());
			} else {
				value = board[i].getLiteral();
			}
			GP.AREA(String("b_data/")+i, 6, String("Плата ") + value, "100px");
			GP.AREA(String("b_stat/")+i, 7, String("Плата ")+ value, "100px");
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
			GP.SELECT("b_sel", list, activeBoard, 0, 0, 1);
        GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);
			GP.BUTTON("svlit_btn", "Сохранить");
			GP.RELOAD("reload");
		GP.BLOCK_END();

		GP.BLOCK_BEGIN(GP_DIV_RAW);//назначение букв
			for (uint8_t i = 0; i < board.size(); i++) {
				String label = "Плата ";
                label += board[i].getAddress();
				GP.BOX_BEGIN();
				GP.LABEL(label);
				GP.TEXT(String("brdLit/")+i, "lit", board[i].getLiteral(), "100px"); 
				GP.BOX_END();
			}
			
		GP.BLOCK_END();

	GP.GRID_END();

}

void GP_mainsets_build(Board &brd) {
	// Формируем список для селектора моторов
	String motorList = "";
	motorList += String("Мотор 0");
	motorList += String(" (") + brd.addSets.motKoef_0 + String("),");
	
	motorList += String("Мотор 1");
	motorList += String(" (") + brd.addSets.motKoef_1 + String("),");
	
	motorList += String("Мотор 2");
	motorList += String(" (") + brd.addSets.motKoef_2 + String("),");
	
	motorList += String("Мотор 3");
	motorList += String(" (") + brd.addSets.motKoef_3 + String("),");

	//------------------------------------------------//
	GP.BLOCK_BEGIN(GP_DIV, "", String("Настройка платы ")+ brd.getLiteral() + String("(") + brd.getAddress() + String(")"));
	M_BOX(GP.BUTTON_MINI("rset_btn", "Прочитать"); GP.BUTTON_MINI("wset_btn", "Записать"));
	M_BOX(GP_EDGES, GP.LABEL("Игнорировать настройки"); GP.CHECK("mset_ignor", brd.mainSets.ignoreSetsFlag));
	M_BOX(GP_EDGES, GP.LABEL("Целевое напряжение");  GP.NUMBER("mset_trgtV", "", brd.mainSets.targetVolt, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Точность/ гистерезис");  GP.NUMBER("mset_prec", "", brd.mainSets.precision, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Подстройка входа");  GP.NUMBER("mset_tunIn", "", brd.mainSets.tuneInVolt, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Подстройка выхода");  GP.NUMBER("mset_tunOut", "", brd.mainSets.tuneOutVolt, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Коэффициент трансформатора");  GP.NUMBER("mset_tratio", "", brd.mainSets.transRatio, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Тип мотора"); GP.SELECT("mset_mottype", motorList, brd.mainSets.motorType) );
	M_BOX(GP_EDGES, GP.LABEL("Тип реле"); GP.SELECT("mset_relset", "Откл,Вкл/откл,Не откл", brd.mainSets.relaySet) );
	GP.BLOCK_END();
}

void GP_addsets_build(Board &brd) {
	GP.BLOCK_BEGIN(GP_DIV_RAW, "", String("Настройка платы ")+ String(brd.getLiteral()) + String("(") + brd.getAddress() + String(")"));
	M_BOX(GP.BUTTON_MINI("rset_btn1", "Прочитать"); GP.BUTTON_MINI("wset_btn1", "Записать"));
	M_BOX(GP_EDGES, GP.LABEL("Максимальное напряжение");  GP.NUMBER("aset_maxV", "", brd.addSets.maxVoltRelative, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Минимальное напряжение");  GP.NUMBER("aset_minV", "", brd.addSets.minVoltRelative, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Время отключения");  GP.NUMBER("aset_toff", "", brd.addSets.emergencyTOFF, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Время включения");  GP.NUMBER("aset_ton", "", brd.addSets.emergencyTON, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Мощность мотора 0");  GP.NUMBER("aset_motK_0", "", brd.addSets.motKoef_0, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Мощность мотора 1");  GP.NUMBER("aset_motK_1", "", brd.addSets.motKoef_1, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Мощность мотора 2");  GP.NUMBER("aset_motK_2", "", brd.addSets.motKoef_2, "100px"));
	M_BOX(GP_EDGES, GP.LABEL("Мощность мотора 3");  GP.NUMBER("aset_motK_3", "", brd.addSets.motKoef_3, "100px"));
	GP.BLOCK_END();
}















