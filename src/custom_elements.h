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