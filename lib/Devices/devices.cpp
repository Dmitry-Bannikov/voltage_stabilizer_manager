#include "devices.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;
using string = std::string;
std::vector<device> Devices;
owner DeviceOwner;

EEManager memoryOwner(DeviceOwner);
EEManager memoryDevices(Devices);

void Devices_Init() {
	memoryOwner.begin(100, MEM_KEY);
	memoryDevices.begin(memoryOwner.nextAddr(), MEM_KEY);
}


void Device_AddOrUpdate(
	const char *name, 
	const char *type, 
	const char *serial_n,
	const char *owner, 
	const char *page, 
	const char *status, 
	const char *is_active
	) 
{
	uint8_t count = Devices.size();
    int isExists = -1;
    isExists = Device_FindIndxFromSN(serial_n); //ищем устройство по его серийнику
    if (isExists != -1) {   //если устройство с таким серийником есть
        Devices[isExists].setParameters(
            name, type, serial_n,
            owner == "" ? Devices[isExists].Email : owner, 
            page == "" ? Devices[isExists].Page : page,
            status == "" ? Devices[isExists].Status : status,
			is_active == "" ? Devices[isExists].IsActive : is_active
        );
    } else {                //иначе добавляем в конец списка
        Devices.emplace_back();
        int8_t num = Devices.size() - 1;
        if (num >= 0) {
            Devices[num].setParameters(
                name, type, serial_n, 
                owner, page,
                status, is_active
            );
        }
    }

}

void Device_CreateList(const char *json) {

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
		strlcpy(Devices[indx].Email, set, 32);
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
	default:
		break;
	}
	return true;
}

String Owner_Get(uint8_t param) {
	String result = "_no_owner";
	char get[32];
	switch (param) {
	case OWN_NAME:
		strcpy(get, DeviceOwner.Name);
		break;
	case OWN_EMAIL:
		strcpy(get, DeviceOwner.Email);
		break;
	case OWN_PASS:
		strcpy(get, DeviceOwner.Pass);
		break;
	case OWN_CODE:
		strcpy(get, DeviceOwner.Code);
		break;
	case OWN_STATUS:
		strcpy(get, DeviceOwner.Status);
		break;
	default:
		strcpy(get, "_invalid");
		break;
	}
	result = get;
	return result;
}

bool Owner_Set(uint8_t param, const String &data) {
	if (data.length() > 31) return false;
	const char* set = data.c_str();
	switch (param) {
	case OWN_NAME:
		strlcpy(DeviceOwner.Name, set, 32);
		break;
	case OWN_EMAIL:
		strlcpy(DeviceOwner.Email, set, 32);
		break;
	case OWN_PASS:
		strlcpy(DeviceOwner.Pass, set, 32);
		break;
	case OWN_CODE:
		strlcpy(DeviceOwner.Code, set, 32);
		break;
	case OWN_STATUS:
		strlcpy(DeviceOwner.Status, set, 32);
		break;
	default:
		break;
	}
	return true;
}

void Device_Save() {
	memoryDevices.updateNow();
}

void Device_Add() {
	Devices.emplace_back();
}

std::string Owner_getJson() {
	json ownJson;
	ownJson["Name"] = DeviceOwner.Name;
	ownJson["Email"] = DeviceOwner.Email;
	ownJson["Pass"] = DeviceOwner.Pass;
	ownJson["Code"] = DeviceOwner.Code;
	ownJson["Status"] = DeviceOwner.Status;
	return ownJson.dump();
}

void Owner_setJson(const char* json_c) {
	std::string jsonString = std::string(json_c);
	try {
    	auto j = json::parse(jsonString);
		if (!j["Name"].is_null()) {
      		strlcpy(DeviceOwner.Name, j["Name"].get<std::string>().c_str(), 32);
		}
		if (!j["Email"].is_null()) {
			strlcpy(DeviceOwner.Email, j["Email"].get<std::string>().c_str(), 32);
		}
		if (!j["Pass"].is_null()) {
			strlcpy(DeviceOwner.Pass, j["Pass"].get<std::string>().c_str(), 32);
		}
		if (!j["Code"].is_null()) {
			strlcpy(DeviceOwner.Code, j["Code"].get<std::string>().c_str(), 32);
		}
		if (!j["Status"].is_null()) {
			strlcpy(DeviceOwner.Status, j["Status"].get<std::string>().c_str(), 32);
		}
  	} catch (const std::exception& e) {
    	Serial.println("Error at json parsing");
  	}
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

void parseJsonToDeviceVector(const char* json_c) {
	string jsonString = string(json_c);
    json devicesJson = json::parse(jsonString);

    Devices.clear(); // Очищаем вектор, чтобы заполнить его заново

    for (const auto& devJson : devicesJson) {
        device dev;
        devJson.at("Name").get_to(dev.Name);
        devJson.at("Type").get_to(dev.Type);
        devJson.at("Email").get_to(dev.Email);
        devJson.at("Page").get_to(dev.Page);
        devJson.at("Status").get_to(dev.Status);
        devJson.at("SN").get_to(dev.SN);
        devJson.at("IsActive").get_to(dev.IsActive);

        // Проверяем, совпадает ли Email владельца с Email устройства
        if (std::string(dev.Email) == std::string(DeviceOwner.Email)) {
            //Device_AddOrUpdate(dev.Name, dev.Type, )
        }
    }
}



















//======================================