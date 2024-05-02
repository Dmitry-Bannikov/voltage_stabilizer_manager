#include "devices.h"


std::vector<device> Devices;
owner DeviceOwner;

EEManager memoryOwner(DeviceOwner);
EEManager memoryDevices(Devices);

void Devices_Init() {
	memoryOwner.begin(100, MEM_KEY);
	memoryDevices.begin(memoryOwner.nextAddr(), MEM_KEY);
}

void Server_Config() {

}

void Device_AddOrUpdate(
	char *name, 
	char *type, 
	char *owner, 
	char *page, 
	char *status, 
	char *serial_n, 
	char *is_active
	) 
{
	uint8_t count = Devices.size();
    int isExists = -1;
    isExists = Device_FindIndxFromSN(serial_n); //ищем устройство по его серийнику
    if (isExists != -1) {   //если устройство с таким серийником есть
        Devices[isExists].setParameters(
            name, type, 
            owner == "" ? Devices[isExists].Email : owner, 
            page == "" ? Devices[isExists].Page : page,
            status == "" ? Devices[isExists].Status : status,
            serial_n == "" ? Devices[isExists].SN : serial_n, 
			is_active == "" ? Devices[isExists].IsActive : is_active
        );
    } else {                //иначе добавляем в конец списка
        Devices.emplace_back();
        int8_t num = Devices.size() - 1;
        if (num >= 0) {
            Devices[isExists].setParameters(
                name, type, 
                owner, page,
                status, serial_n, is_active
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

int Device_FindIndxFromSN(char *sn) {
    uint8_t count = Devices.size();
    for (uint8_t i = 0; i < count; i++) {
        if (!strcmp(Devices[i].SN, sn)) {
            return i;
        }
    }
    return -1;
}

int Device_GetAmount() {
	return Devices.size();
}

std::string Device_Get(uint8_t indx, uint8_t param) {
	std::string result;
	char buffer[32];
	if (indx < Devices.size() && Devices.size() > 0) {
		Devices[indx].Get(param, buffer);
		result = buffer;
	}
	return result;
}























//======================================