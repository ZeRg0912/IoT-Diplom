#include "TimeManager.h"
#include "SmartHome.h"

extern LiquidCrystal_I2C lcd;
extern SmartHome smartHome;

TimeManager::TimeManager() {}

void TimeManager::SetDefaultTime(struct tm& currentTime) {
    currentTime.tm_year = 2000;
    currentTime.tm_mon  = 1;           
    currentTime.tm_mday = 1;           
    currentTime.tm_hour = 0;           
    currentTime.tm_min  = 0;
    currentTime.tm_sec  = 0;

    time_t now = mktime(&currentTime);
    struct timeval tv = { now, 0 };
    settimeofday(&tv, NULL);
}

void TimeManager::SetTime(struct tm& newTime) {
    time_t now = mktime(&newTime);
    struct timeval tv = { now, 0 };
    settimeofday(&tv, NULL);
}

void TimeManager::GetCurrentTime(struct tm& currentTime) {
    getLocalTime(&currentTime);
}

void TimeManager::FormatTime(char* buffer, const struct tm& time) {
    sprintf(buffer, "%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);
}