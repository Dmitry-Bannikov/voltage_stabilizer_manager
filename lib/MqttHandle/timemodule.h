#pragma once

#include <string>


void Time_begin(int offset);
std::string Time_getCurrent();
void Time_setOffset(int offset);
int Time_syncTZ();


