#pragma once
#include <GyverPortal.h>
#include <data.h>
//#include <service.h>

void GP_details_start(const String &summary, const String &id);
void GP_details_end();
void GP_build_data();
void GP_build_stats();
void GP_target_set();
void GP_container_row();
void GP_container_column(const String& title = "");
void GP_block_begin(const String& width = "");
void GP_block_end();
void GP_title(const String& text);



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

void GP_build_data() {
	for (int i = 0; i < gNumBoards; i++) {
		String text = "Board #";
		text += String(i2c_boards_addrs[i], HEX);
		GP.AREA(String("b_data/") + i, 6, text, "50");
	}
}

void GP_build_stats() {
	for (int i = 0; i < gNumBoards; i++) {
		String text = "Board #";
		text += String(i2c_boards_addrs[i], HEX);
		GP.AREA(String("b_stat/") + i, 10, text, "50");
	}
}

void GP_target_set() {
    GP_container_column("Set target board:");
    GP_container_row();
    for (int i = 0; i < gNumBoards; i++) {
        targetBoard[i] = 1;
        GP_container_column();
        GP.LABEL(String("Board 0x") + i2c_boards_addrs[i]);
        GP.CHECK(String("btarget/") + i, targetBoard[i]);
        GP_block_end();
    }
    GP_container_column();
    GP.SUBMIT_MINI("Save/Write");
	GP.BUTTON_MINI("btn1", "Read Sets");
	GP.RELOAD_CLICK("btn1");
    GP_block_end();

    GP_block_end();//row
    GP_block_end();//column
}

void GP_container_row() {
    String s;
    s += F("\n<div style='display: flex; flex-direction: row; flex-wrap: nowrap;'>\n");
    GP.SEND(s);
}

void GP_block_end() {
    GP.BLOCK_END();
}

void GP_container_column(const String& title) {
    String s;
    s += F("\n<div style='display: flex; flex-direction: column; ");
    s += F("margin: 0 auto;");
    s += F("max-width: max-content; align-items: center; ");
    s += F("justify-content: space-around; padding: 1%;'>\n");
    if (title.length()) {
        s += F("\n<h2>");
        s += title;
        s += F("</h2>");
    }
    GP.SEND(s);
}

void GP_title(const String& text) {
    String s;
    s += F("<h2>");
    s += text;
    s += F("</h2>");
    GP.SEND(s);
}

void GP_block_begin(const String& width) {
    String s;
    s += F("\n<div ");
    if (width.length()) {
        s += F("style='width: ");
        s += width;
        s += F(";'");
    }
    s += F(">\n");
}













