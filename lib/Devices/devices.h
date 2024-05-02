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

#define	OWNER_NAME		0
#define OWNER_EMAIL		1
#define OWNER_PASS		2
#define OWNER_CODE		3

struct owner {
	char Name[32];
	char Email[32];
	char Pass[32];
	char Code[32];
	void setParameters(const char *name, const char *email, const char *pass, const char *code) {
		strcpy(Name, name);
		strcpy(Email, email);
		strcpy(Pass, pass);
		strcpy(Code, code);
	}
	void Get(uint8_t param, char *get) {
		switch (param) {
		case OWNER_NAME:
			strcpy(get, Name);
			break;
		case OWNER_EMAIL:
			strcpy(get, Email);
			break;
		case OWNER_PASS:
			strcpy(get, Pass);
			break;
		case OWNER_CODE:
			strcpy(get, Code);
			break;
		default:
			break;
		}
	}
	void Set(uint8_t param, char *set) {
		switch (param) {
		case OWNER_NAME:
			strcpy(Name, set);
			break;
		case OWNER_EMAIL:
			strcpy(Email, set);
			break;
		case OWNER_PASS:
			strcpy(Pass, set);
			break;
		case OWNER_CODE:
			strcpy(Code, set);
			break;
		default:
			break;
		}
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

	void setParameters(const char *name, const char *type, const char *owner,
						const char *page, const char *status, const char *sn,
						const char *is_act) {
		strcpy(Name, name);
		strcpy(Type, type);
		strcpy(Email, owner);
		strcpy(Page, page);
		strcpy(Status, status);
		strcpy(SN, sn);
		strcpy(IsActive, is_act);
	}
	void Get(uint8_t param, char *get) {
		switch (param) {
		case DEV_NAME:
			strcpy(get, Name);
			break;
		case DEV_TYPE:
			strcpy(get, Type);
			break;
		case DEV_EMAIL:
			strcpy(get, Email);
			break;
		case DEV_PAGE:
			strcpy(get, Page);
			break;
		case DEV_STATUS:
			strcpy(get, Status);
			break;
		case DEV_SN:
			strcpy(get, SN);
			break;
		case DEV_ISACT:
			strcpy(get, IsActive);
			break;
		default:
			break;
		}
	}
	void Set(uint8_t param, char *set) {
		switch (param) {
		case DEV_NAME:
			strcpy(Name, set);
			break;
		case DEV_TYPE:
			strcpy(Type, set);
			break;
		case DEV_EMAIL:
			strcpy(Email, set);
			break;
		case DEV_PAGE:
			strcpy(Page, set);
			break;
		case DEV_STATUS:
			strcpy(Status, set);
			break;
		case DEV_SN:
			strcpy(SN, set);
			break;
		case DEV_ISACT:
			strcpy(IsActive, set);
			break;
		default:
			break;
		}
	}
};

void Devices_Init();
void Server_Config();
void Device_AddOrUpdate(
	char *name, 
	char *type, 
	char *owner, 
	char *page, 
	char *status, 
	char *serial_n, 
	char *is_active
	);
void Device_CreateList(const char *json);
void Device_Delete(int indx);
int Device_FindIndxFromSN(char *sn);
int Device_GetAmount();
std::string Device_Get(uint8_t indx, uint8_t param);
















//========================================