#pragma once
#include <GyverPortal.h>
#include <data.h>
//#include <service.h>

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

void GP_build_data() {
	for (int i = 0; i2c_boards_addrs[i] != 0; i++) {
		String text = "Board #";
		text += String(i2c_boards_addrs[i]);
		GP.AREA(String("b_stat/") + i, 5, text, "30");
	}
}

void GP_build_stats() {
	for (int i = 0; i2c_boards_addrs[i] != 0; i++) {
		String text = "Board #";
		text += String(i2c_boards_addrs[i]);
		GP.AREA(String("b_stat/") + i, 5, text, "30");
	}
}