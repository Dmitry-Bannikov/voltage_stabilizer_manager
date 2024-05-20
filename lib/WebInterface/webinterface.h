#pragma once


#include "Arduino.h"

void portalInit();
void portalTick();
void portalBuild();
void portalActions();
void createUpdateList(String &list);
void formsHandler();
void clicksHandler();
void updatesHandler();
void buttons_handler();
void fields_handler();
void ActionsDevice_handler();
void ActionsUser_handler();