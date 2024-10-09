#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include "ConfigMap.h"
#include <time.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class TimeManager {
public:
    TimeManager();
    void SetDefaultTime(struct tm& currentTime);  
    void SetTime(struct tm& newTime);            
    void GetCurrentTime(struct tm& currentTime);
    void FormatTime(char* buffer, const struct tm& time);
};

#endif