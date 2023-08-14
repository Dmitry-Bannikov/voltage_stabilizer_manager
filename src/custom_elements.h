#pragma once
#include <GyverPortal.h>

void GP_details_start(const String &summary, const String &id);
void GP_details_end();
void GP_hideBlock_start(const String &name);
void GP_hideBlock_end();
void GP_checkBox(const String &label, const String &checkbox_id, const String &div_id, bool state);
String openBlock_script();



void GP_details_start(const String &summary, const String &id = "") {
    //  <details id = ''>
    //      <summary>
    //      some text
    //      </summary>
    //
    String s;
    s += F("<details ");
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

void GP_hideBlock_start(const String &div_id) {
	String s;
	s += F("\n<div ");
	s += F("id='");
	s += div_id;
	s += F("' ");
	s += F(">\n");
	GP.SEND(s);
}

void GP_hideBlock_end() {
	GP.SEND("</div>");
}

void GP_checkBox(const String &label, const String &checkbox_id, const String &div_id, bool state = false) {
	PGM_P st = GP_GREEN;
	bool dis = false;
	GP.LABEL(label);
	String s;
	s += openBlock_script();
	if (st != GP_GREEN) {
		s += F("<style>#__");
		s += checkbox_id;
		s += F(" input:checked+span::before{border-color:");
		s += FPSTR(st);
		s += F(";background-color:");
		s += FPSTR(st);
		s += F("}</style>\n");
	}
	s += F("<label id='__");
	s += checkbox_id;
	s += F("' class='check_c'><input type='checkbox' name='");
	s += checkbox_id;
	s += F("' id='check_");
	s += checkbox_id;
	s += "' ";
	s += F("onclick='toggleDiv(");
	s += div_id;
	s += F(", ");
	s += checkbox_id;
	s += F(")' ");
	if (state)
		s += F("checked ");
	if (dis)
		s += F("disabled ");
	s += F("><span></span></label>\n"
				"<input type='hidden' value='0' name='");
	s += checkbox_id;
	s += "'>\n";
	GP.SEND(s);

}

String openBlock_script() {
	String s;
	s += F("\n<script>\n");
	s += F("\nfunction toggleDiv(div_id, checkbox_id) {\n");
	s += F("var checkbox = document.getElementById(checkbox_id);\n");
	s += F("var div = document.getElementById(div_id);\n");
	s += F("if (checkbox.checked) div.style.display = 'block';\n");
	s += F("else div.style.display = 'none';\n");
	s += F("}\n");
	s += F("</script>\n");
	return s;
}
