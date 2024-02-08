#include <Display.h>

void Meter::begin() {
    Serial2.begin(9600);
}

bool Meter::requestValue(float &value, commands command) {
    uint8_t regAddr = ((uint8_t)(command))*2;
    bool result = false;
    uint8_t buffer[8] = {1,4,0,0x00,0,2,0,0};
    buffer[3] = regAddr;
    getCRC(buffer, sizeof(buffer));
    Serial2.write(buffer, sizeof(buffer));
    uint32_t tmr = millis();
    while(!Serial2.available() || millis() - tmr < 200);
    if (Serial2.available()==9) {
        uint8_t bufferRX[9];
        Serial2.read(bufferRX, sizeof(bufferRX));
        if (bufferRX[0] == 1 && bufferRX[1] == 4) {
            float value_read = *((float*)(bufferRX+3));
            value = reverseBytes(value_read);
            flushSerial();
            result = true;
        } 
    }
    return result;
    
}

void Meter::getCRC(uint8_t* buffer, size_t size) {
    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < size-2; i++) {
        crc ^= buffer[i];
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    // Инверсия битов
    buffer[size-2] = (uint8_t)(crc);
    buffer[size-1] = ((crc & 0xFF00) >> 8);
    crc = ((crc & 0xFF) << 8) | ((crc & 0xFF00) >> 8);
}

void Meter::flushSerial() {
    while (Serial2.available()) {
        uint8_t garbage = Serial2.read();
    }
}

Meter::~Meter() {

}



//