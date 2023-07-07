#pragma once
#include <GyverPortal.h>

void GP_HIDE_BLOCK_SCRIPT(const String &checkbox_id, const String &div_id) {
  String s;
  s += F("<script>\n");
  s += F("function toggleDiv() {\n");
  s += F("var checkbox = document.getElementById('");
  s += checkbox_id;
  s += F("');\n");
  s += F("var div = document.getElementById('");
  s += div_id;
  s += F("');\n");
  s += F("if (checkbox.checked) div.style.display = 'block';\n");
  s += F("else div.style.display = 'none';\n");
  s += F("}\n");
  s += F("</script>\n");
  GP.SEND(s);
}

void GP_DIV_START(const String &id = "", bool visible = true) {
  String s;
  s += F("<div ");
  if (id.length()) {
    s += F("id='");
    s += id;
    s += F("' ");
  }
  if (!visible) {
    s += F(" style='display: none;' ");
  }
  s += F(">");
  GP.SEND(s);
}

void GP_DIV_END() {
    GP.SEND("</div>");
}

void GP_CHECK_FUNC(const String &name, bool state = 0, const String &myFoo = "",
                PGM_P st = GP_GREEN, bool dis = false) {
  String s;
  if (st != GP_GREEN) {
    s += F("<style>#__");
    s += name;
    s += F(" input:checked+span::before{border-color:");
    s += FPSTR(st);
    s += F(";background-color:");
    s += FPSTR(st);
    s += F("}</style>\n");
  }
  s += F("<label id='__");
  s += name;
  s += F("' class='check_c'><input type='checkbox' name='");
  s += name;
  s += F("' id='check_");
  s += name;
  s += "' ";
  if (myFoo.length()) {
    s += F("onclick='");
    s += myFoo;
    s += F("' ");
  }
  if (state)
    s += F("checked ");
  if (dis)
    s += F("disabled ");
  s += F("><span></span></label>\n"
             "<input type='hidden' value='0' name='");
  s += name;
  s += "'>\n";
  GP.SEND(s);
}