#include "devices.h"
#include "nlohmann/json.hpp"
#include <common_data.h>
using json = nlohmann::ordered_json;
using string = std::string;
std::vector<device> Devices;
user User;
device LocalDevice;
EEManager memoryOwner(User);
EEManager memoryDevices(LocalDevice, 23000);




void Devices_Init() {
	memoryOwner.begin(200, 4);
	LocalDevice.setParameters("Стабилизатор 1", "stab_brd", String(Board_SN).c_str(), User.Email, "/dashboard", "onreg", "0", "13");
	Devices.emplace_back(LocalDevice);
	memoryDevices.begin(memoryOwner.nextAddr() + 1, 4);	//вспоминаем данные об устройствах
	delay(10);
}

void Devices_Tick() {
	uint32_t tmr = 0;
	if (millis() - tmr > 120000) {
		if (Devices.size()) {
			if (Device_Get(0, DEV_SN) != S(Board_SN)) return;
			Device_Set(0, DEV_ISACT, String(millis()/60000));
		}
	}
	memoryDevices.tick();
}


//==============USER FUNCTIONS==================//
void User_AddOrUpdate(
	const char *name, 
	const char *email, 
	const char *pass,
	const char *code, 
	const char *status,
	const char *timezone
	) 
{
    if (strcmp(name, "")) strlcpy(User.Name, name, 32);
	if (strcmp(email, "")) strlcpy(User.Email, email, 32);
	if (strcmp(pass, "")) strlcpy(User.Pass, pass, 32);
	if (strcmp(code, "")) strlcpy(User.Code, code, 32);
	if (strcmp(status, "")) strlcpy(User.Status, status, 32);
	if (strcmp(timezone, "")) strlcpy(User.Timezone, timezone, 7);
}

String User_Get(uint8_t param) {
	String result = "_no_owner";
	char get[32];
	switch (param) {
	case OWN_NAME:
		strcpy(get, User.Name);
		break;
	case OWN_EMAIL:
		strcpy(get, User.Email);
		break;
	case OWN_PASS:
		strcpy(get, User.Pass);
		break;
	case OWN_CODE:
		strcpy(get, User.Code);
		break;
	case OWN_STATUS:
		strcpy(get, User.Status);
		break;
	case OWN_TIMEZONE:
		strcpy(get, User.Timezone);
		break;
	default:
		strcpy(get, "_null");
		break;
	}
	result = get;
	return result;
}

bool User_Set(uint8_t param, const String &data) {
	if (data.length() > 31) return false;
	const char* set = data.c_str();
	switch (param) {
	case OWN_NAME:
		strlcpy(User.Name, set, 32);
		break;
	case OWN_EMAIL:
		strlcpy(User.Email, set, 64);
		break;
	case OWN_PASS:
		strlcpy(User.Pass, set, 32);
		break;
	case OWN_CODE:
		strlcpy(User.Code, set, 32);
		break;
	case OWN_STATUS:
		strlcpy(User.Status, set, 32);
		break;
	case OWN_TIMEZONE:
		strlcpy(User.Timezone, set, 7);
		break;
	default:
		break;
	}
	return true;
}

std::string User_getJson() {
	json ownJson;
	ownJson["Name"] = User.Name;
	ownJson["Email"] = User.Email;
	ownJson["Pass"] = User.Pass;
	ownJson["Code"] = User.Code;
	ownJson["Status"] = User.Status;
	ownJson["Timezone"] = User.Timezone;
	return ownJson.dump();
}

void User_setJson(const char* json_c) {
	std::string jsonString = std::string(json_c);
	auto j = json::parse(jsonString);
	j["Name"].is_null() ? strcpy(User.Name, User.Name) : j.at("Name").get_to(User.Name);
	j["Email"].is_null() ? strcpy(User.Email, User.Email) : j.at("Email").get_to(User.Email);
	j["Pass"].is_null() ? strcpy(User.Pass, User.Pass) : j.at("Pass").get_to(User.Pass);
	j["Code"].is_null() ? strcpy(User.Code, User.Code) : j.at("Code").get_to(User.Code);
	j["Status"].is_null() ? strcpy(User.Status, User.Status) : j.at("Status").get_to(User.Status);
	j["Timezone"].is_null() ? strcpy(User.Timezone, User.Timezone) : j.at("Status").get_to(User.Status);
}

void User_Save() {
	memoryOwner.updateNow();
}

//==============DEVICE FUNCTIONS==================//

void Device_AddOrUpdate(
	const char *name, 
	const char *type, 
	const char *serial_n,
	const char *owner, 
	const char *page, 
	const char *status, 
	const char *is_active,
	const char *timezone
	) 
{
	uint8_t count = Devices.size();
    int num = -1;
    num = Device_FindIndxFromSN(serial_n); //ищем устройство по его серийнику
	if (num == -1) { //если устройства с таким сер. нет, то добавляем в список
		Devices.emplace_back();
        num = Devices.size() - 1;
		if (num < 0) return;
	}
	if (strcmp(name, "")) strlcpy(Devices[num].Name, name, 32); //если имя не пустое...
	if (strcmp(type, "")) strlcpy(Devices[num].Type, type, 32) ;
	if (strcmp(serial_n, "")) strlcpy(Devices[num].SN, serial_n, 32);
	if (strcmp(owner, "")) strlcpy(Devices[num].Email, owner, 64);
	if (strcmp(page, "")) strlcpy(Devices[num].Page, page, 32);
	if (strcmp(status, "")) strlcpy(Devices[num].Status, status, 32);
	if (strcmp(is_active, "")) strlcpy(Devices[num].IsActive, is_active, 32);
	if (strcmp(timezone, "")) strlcpy(Devices[num].TimeZone, timezone, 32);
	if (num == 0 && S(serial_n).toInt() == Board_SN) {
		LocalDevice = Devices[0];
	}
}

void Device_Delete(int indx) {
	if (indx >= Devices.size()) return;
    Devices.erase(Devices.begin() + indx);
}

int Device_FindIndxFromSN(const char *sn) {
    uint8_t count = Devices.size();
    for (uint8_t i = 0; i < count; i++) {
        if (!strcmp(Devices[i].SN, sn)) {
            return i;
        }
    }
    return -1;
}

int Device_Size() {
	return Devices.size();
}

String Device_Get(uint8_t indx, uint8_t param) {
	String result = "_no_device";

	if (indx >= Devices.size() || Devices.size() < 1) return result;
	char get[32];
	switch (param) {
	case DEV_NAME:
		strcpy(get, Devices[indx].Name);
		break;
	case DEV_TYPE:
		strcpy(get, Devices[indx].Type);
		break;
	case DEV_EMAIL:
		strcpy(get, Devices[indx].Email);
		break;
	case DEV_PAGE:
		strcpy(get, Devices[indx].Page);
		break;
	case DEV_STATUS:
		strcpy(get, Devices[indx].Status);
		break;
	case DEV_SN:
		strcpy(get, Devices[indx].SN);
		break;
	case DEV_ISACT:
		strcpy(get, Devices[indx].IsActive);
		break;
	case DEV_TIMEZONE:
		strcpy(get, Devices[indx].TimeZone);
		break;
	default:
		strcpy(get, "_invalid");
		break;
	}
	result = get;
	return result;
}

bool Device_Set(uint8_t indx, uint8_t param, const String &data) {
	if (indx >= Devices.size() || Devices.size() < 1 || data.length() > 31) return false;
	const char* set = data.c_str();
	switch (param) {
	case DEV_NAME:
		strlcpy(Devices[indx].Name, set, 32);
		break;
	case DEV_TYPE:
		strlcpy(Devices[indx].Type, set, 32);
		break;
	case DEV_EMAIL:
		strlcpy(Devices[indx].Email, set, 64);
		break;
	case DEV_PAGE:
		strlcpy(Devices[indx].Page, set, 32);
		break;
	case DEV_STATUS:
		strlcpy(Devices[indx].Status, set, 32);
		break;
	case DEV_SN:
		strlcpy(Devices[indx].SN, set, 32);
		break;
	case DEV_ISACT:
		strlcpy(Devices[indx].IsActive, set, 32);
		break;
	case DEV_TIMEZONE:
		strlcpy(Devices[indx].TimeZone, set, 32);
		break;
	default:
		break;
	}
	return true;
}

std::string Device_getJson() {
    json devicesJson;
    for (const auto& dev : Devices) {
        json devJson;
        devJson["Name"] = dev.Name;
        devJson["Type"] = dev.Type;
        devJson["Email"] = dev.Email;
        devJson["Page"] = dev.Page;
        devJson["Status"] = dev.Status;
        devJson["SN"] = dev.SN;
        devJson["IsActive"] = dev.IsActive;
        devicesJson.push_back(devJson);
    }

    return devicesJson.dump();
}

void Device_setJson(const char* json_c) {
	string jsonString = string(json_c);
    json devicesJson = json::parse(jsonString);

    Devices.clear(); // Очищаем вектор, чтобы заполнить его заново

    for (const auto& devJson : devicesJson) {
        device dev;
		devJson["Name"].is_null() ? strcpy(dev.Name, "") : devJson.at("Name").get_to(dev.Name);
        devJson["Type"].is_null() ? strcpy(dev.Type, "") : devJson.at("Type").get_to(dev.Type);
		devJson["SN"].is_null() ? strcpy(dev.SN, "") : devJson.at("SN").get_to(dev.SN);
        devJson["Email"].is_null() ? strcpy(dev.Email, "") : devJson.at("Email").get_to(dev.Email);
        devJson["Page"].is_null() ? strcpy(dev.Page, "") : devJson.at("Page").get_to(dev.Page);
        devJson["Status"].is_null() ? strcpy(dev.Status, "") : devJson.at("Status").get_to(dev.Status);
        devJson["IsActive"].is_null() ? strcpy(dev.IsActive, "") : devJson.at("IsActive").get_to(dev.IsActive);
		devJson["TimeZone"].is_null() ? strcpy(dev.TimeZone, "") : devJson.at("TimeZone").get_to(dev.TimeZone);

        // Проверяем, совпадает ли Email владельца с Email устройства
        if (std::string(dev.Email) == std::string(User.Email)) {
            Device_AddOrUpdate(dev.Name, dev.Type, dev.SN, dev.Email, dev.Page, dev.Status, dev.Type);
        }
    }
}

void Device_Save() {
	Serial.println(LocalDevice.Name);
	memoryDevices.updateNow();
}

uint32_t hashEmail(const char* str) {
	uint32_t hash = 0;
    uint32_t prime = 31;

    while (*str) {
        hash = hash * prime + static_cast<uint32_t>(*str);
        str++;
    }

    return hash;
}













//======================================