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

struct owner {
	char Name[32];
	char Email[32];
	char Pass[32];
	char Code[32];
	char Status[32];
	void setParameters(const char *name, const char *email, const char *pass, const char *code, const char* status) {
		strcpy(Name, name);
		strcpy(Email, email);
		strcpy(Pass, pass);
		strcpy(Code, code);
		strcpy(Status, status);
	}
};

struct device {
	char Name[32] = ""; //имя устройства
	char Type[32] = ""; //тип устройства (стаб/мультиметр и тд)
	char Email[32] = ""; // е-майл пользователя
	char Page[32] = ""; //веб адрес устройства (присваивается сервером)
	char Status[32] = "";
	char SN[32] = "";  //серийник устройства
	char IsActive[32] = ""; //активно ли сейчас устройство (передает ли данные)

	void setParameters(const char *name, const char *type, const char *sn, const char *owner,
						const char *page, const char *status, const char *is_act) {
		strcpy(Name, name);
		strcpy(Type, type);
		strcpy(Email, owner);
		strcpy(Page, page);
		strcpy(Status, status);
		strcpy(SN, sn);
		strcpy(IsActive, is_act);
	}
};

void Devices_Init();
void Server_Config();
void Owner_AddOrUpdate(
	const char *name, 
	const char *email, 
	const char *pass,
	const char *code = "", 
	const char *status = ""
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
String Device_Get(uint8_t indx, uint8_t param);
bool Device_Set(uint8_t indx, uint8_t param, const String &data);
String Owner_Get(uint8_t param);
bool Owner_Set(uint8_t param, const String &data);
void Device_Save();













//========================================