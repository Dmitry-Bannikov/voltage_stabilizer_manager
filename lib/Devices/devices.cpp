#include "devices.h"


std::vector<device> Devices;
owner DeviceOwner;

EEManager memoryOwner(DeviceOwner);
EEManager memoryDevices(Devices);

void Devices_Init() {
	memoryOwner.begin(100, MEM_KEY);
	memoryDevices.begin(memoryOwner.nextAddr(), MEM_KEY);
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
		strcpy(Devices[indx].Name, set);
		break;
	case DEV_TYPE:
		strcpy(Devices[indx].Type, set);
		break;
	case DEV_EMAIL:
		strcpy(Devices[indx].Email, set);
		break;
	case DEV_PAGE:
		strcpy(Devices[indx].Page, set);
		break;
	case DEV_STATUS:
		strcpy(Devices[indx].Status, set);
		break;
	case DEV_SN:
		strcpy(Devices[indx].SN, set);
		break;
	case DEV_ISACT:
		strcpy(Devices[indx].IsActive, set);
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
		strcpy(DeviceOwner.Name, set);
		break;
	case OWN_EMAIL:
		strcpy(DeviceOwner.Email, set);
		break;
	case OWN_PASS:
		strcpy(DeviceOwner.Pass, set);
		break;
	case OWN_CODE:
		strcpy(DeviceOwner.Code, set);
		break;
	case OWN_STATUS:
		strcpy(DeviceOwner.Status, set);
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

void Owner_Save() {
	memoryOwner.updateNow();
}

























//======================================