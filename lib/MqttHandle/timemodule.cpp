#include "timemodule.h"
#include <GyverPortal.h>
#include <common_data.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
GPtime t;


void Time_begin(int offset) {
    timeClient.begin();
    timeClient.setTimeOffset(offset); // Установите смещение времени, если необходимо
}

std::string Time_getCurrent() {
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime);

    char buffer[30];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d.000Z",
             ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
             ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    return std::string(buffer);
}


int Time_syncTZ() {
    timeClient.update();
    t = ui.getSystemTime();
    unsigned long epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    if (ptm->tm_year + 1900 < 2000 || (!t.hour && !t.minute && !t.second)) return 13;
    if (t.hour != ptm->tm_hour) {
        int utc = t.hour - ptm->tm_hour;
        timeClient.setTimeOffset(utc*3600);
        Serial.printf("TimeZone: %d\n", utc);
        return utc;
    }
    return 13;
}












//===============================================================================