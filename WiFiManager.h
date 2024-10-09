#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include "ConfigMap.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>

class WiFiManager {
public:
    WiFiManager();
    void WiFiSetup(const String& SSID, const String& password);
    void GetTime(struct tm& currentTime);
    void ReceiveMessage(const String& message, struct tm& currentTime);

private:
    String url_time;
};

#endif