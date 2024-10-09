#include "WiFiManager.h"
#include "SmartHome.h"

extern LiquidCrystal_I2C lcd;
extern SmartHome smartHome;

WiFiManager::WiFiManager() {
    url_time = URL_TIME;
}

void WiFiManager::WiFiSetup(const String& SSID, const String& password) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Init WiFi");

  WiFi.begin(SSID, password);
  uint64_t start_time = millis();
  
  while (WiFi.status() != WL_CONNECTED && millis() - start_time < TIMEOUT_MS) {
      delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
      lcd.setCursor(0, 1);
      lcd.print("Init done!");
  } else {
      lcd.setCursor(0, 1);
      lcd.print("Init failed");
  }

  delay(1000);    
  smartHome.SetTimerMenuAFK();
}

void WiFiManager::GetTime(struct tm& currentTime) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Receiving time");
    http.begin(url_time);
    http.setTimeout(TIMEOUT_MS);
    int httpCode = http.GET();

    if (httpCode > 0) {
        lcd.setCursor(0, 1);
        lcd.print("Done");
        delay(1000);
        String message = http.getString();
        ReceiveMessage(message, currentTime);
    } else {
        lcd.setCursor(0, 1);
        lcd.print("Fail");
        delay(1000);
    }
    http.end();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Can't get time!");
    lcd.setCursor(0, 1);
    lcd.print("Set default");
    smartHome.SetDefaultTime();
    delay(1000);
  }
}

void WiFiManager::ReceiveMessage(const String& message, struct tm& currentTime) {
  StaticJsonDocument<1500> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    smartHome.SetDefaultTime();
    delay(1000);
    return;
  } else {
    const char *datetime = doc["datetime"];
    int year, month, day, hour, minute, second;

    sscanf(datetime, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);

    currentTime.tm_year = year - 1900;
    currentTime.tm_mon  = month - 1;
    currentTime.tm_mday = day;
    currentTime.tm_hour = hour;
    currentTime.tm_min  = minute;
    currentTime.tm_sec  = second;

    time_t now = mktime(&currentTime);
    struct timeval tv = { now, 0 };
    settimeofday(&tv, NULL);
  }
}