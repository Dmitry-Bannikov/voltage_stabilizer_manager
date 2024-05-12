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






void Owner_AddOrUpdate(
	const char *name, 
	const char *email, 
	const char *pass,
	const char *code, 
	const char *status 
	) 
{
    strcpy(DeviceOwner.Name, name);
	strcpy(DeviceOwner.Email, email);
	strcpy(DeviceOwner.Pass, pass);
	strcpy(DeviceOwner.Code, code);
	strcpy(DeviceOwner.Status, status);
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
	auto j = json::parse(jsonString);
	j["Name"].is_null() ? strcpy(DeviceOwner.Name, "NullName") : j.at("Name").get_to(DeviceOwner.Name);
	j["Email"].is_null() ? strcpy(DeviceOwner.Email, "NullEmail") : j.at("Email").get_to(DeviceOwner.Email);
	j["Pass"].is_null() ? strcpy(DeviceOwner.Pass, "NullPass") : j.at("Pass").get_to(DeviceOwner.Pass);
	j["Code"].is_null() ? strcpy(DeviceOwner.Code, "NullCode") : j.at("Code").get_to(DeviceOwner.Code);
	j["Status"].is_null() ? strcpy(DeviceOwner.Status, "NullStatus") : j.at("Status").get_to(DeviceOwner.Status);
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
		devJson["Name"].is_null() ? strcpy(dev.Name, "Unknown") : devJson.at("Name").get_to(dev.Name);
        devJson["Type"].is_null() ? strcpy(dev.Type, "Unknown") : devJson.at("Type").get_to(dev.Type);
		devJson["SN"].is_null() ? strcpy(dev.SN, "Unknown") : devJson.at("SN").get_to(dev.SN);
        devJson["Email"].is_null() ? strcpy(dev.Email, "") : devJson.at("Email").get_to(dev.Email);
        devJson["Page"].is_null() ? strcpy(dev.Page, "") : devJson.at("Page").get_to(dev.Page);
        devJson["Status"].is_null() ? strcpy(dev.Status, "") : devJson.at("Status").get_to(dev.Status);
        devJson["IsActive"].is_null() ? strcpy(dev.IsActive, "") : devJson.at("IsActive").get_to(dev.IsActive);

        // Проверяем, совпадает ли Email владельца с Email устройства
        if (std::string(dev.Email) == std::string(DeviceOwner.Email)) {
            Device_AddOrUpdate(dev.Name, dev.Type, dev.SN, dev.Email, dev.Page, dev.Status, dev.Type);
        }
    }
}



















//======================================