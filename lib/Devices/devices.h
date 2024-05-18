#pragma once
#include <Arduino.h>
#include <EEManager.h> 
#include <nlohmann/json.hpp>

#define MEM_KEY		127

#define DEV_NAME	0
#define DEV_TYPE	1
#define DEV_EMAIL	2
#define DEV_PAGE	3
#define DEV_STATUS	4
#define DEV_SN		5
#define DEV_ISACT	6

#define	OWN_NAME		0
#define OWN_EMAIL		1
#define OWN_PASS		2
#define OWN_CODE		3
#define OWN_STATUS		4
#define OWN_TIMEZONE	5

struct user {
	char Name[32] = "";	//имя пользователя
	char Email[32] = "";	//емайл пользователя
	char Pass[32] = "";	//пароль пользователя
	char Code[32] = "";	//код подтверждения от сервера
	char Status[32] = "";	//статус пользователя
	char Timezone[10] = "GMT+3";
	void setParameters(const char *name, const char *email, const char *pass, const char *code, const char* status, const char* t_zone) {
		strlcpy(Name, name, 32);
		strlcpy(Email, email, 32);
		strlcpy(Pass, pass, 32);
		strlcpy(Code, code, 32);
		strlcpy(Status, status, 32);
		strlcpy(Timezone, t_zone, 10);
	}
};

struct device {
	char Name[32] = ""; //имя устройства
	char Type[32] = ""; //тип устройства (стаб/мультиметр и тд)
	char SN[32] = "";  //серийник устройства
	char Email[32] = ""; // е-майл владельца
	char Page[32] = ""; //веб адрес устройства (присваивается сервером)
	char Status[32] = "";
	char IsActive[32] = ""; //активно ли сейчас устройство (передает ли данные)

	void setParameters(const char *name, const char *type, const char *sn, const char *owner,
						const char *page, const char *status, const char *is_act) {
		strlcpy(Name, name, 32);
		strlcpy(Type, type, 32);
		strlcpy(Email, owner, 32);
		strlcpy(Page, page, 32);
		strlcpy(Status, status, 32);
		strlcpy(SN, sn, 32);
		strlcpy(IsActive, is_act, 32);
	}
};

void Devices_Init();
void User_AddOrUpdate(
	const char *name = "", 
	const char *email = "", 
	const char *pass = "",
	const char *code = "", 
	const char *status = "",
	const char *timezone = ""
	);
void Device_AddOrUpdate(
	const char *name, 
	const char *type, 
	const char *serial_n,
	const char *owner = "", 
	const char *page = "", 
	const char *status = "", 
	const char *is_active = ""
	);
void Device_CreateList(const char *json);
void Device_Delete(int indx);
int Device_FindIndxFromSN(const char *sn);
int Device_Size();

void Device_Save();
void User_Save();

std::string User_getJson();
void User_setJson(const char* json_c);
String User_Get(uint8_t param);
bool User_Set(uint8_t param, const String &data);

std::string Device_getJson();
void Device_setJson(const char* json_c);
String Device_Get(uint8_t indx, uint8_t param);
bool Device_Set(uint8_t indx, uint8_t param, const String &data);









//========================================