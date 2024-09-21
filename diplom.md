#include "WiFi.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define MAIN_LIGHT_BUTTON_1_PIN           2
#define MAIN_LIGHT_BUTTON_2_PIN           4
#define UP_BUTTON_PIN                     14
#define DOWN_BUTTON_PIN                   12
#define ENTER_BUTTON_PIN                  27
#define BACK_BUTTON_PIN                   26

#define MAIN_LIGHT_LED_PIN                19
#define NIGHT_LIGHT_LED_PIN               18
#define ALARM_LIGHT_LED_PIN               5
#define MOTION_SENSOR_PIN                 13

#define DEBOUNCE_DELAY                    25
#define TIMEOUT_MS                        5000

volatile unsigned long debounce_timer   = 0;

volatile bool main_light_interrupt      = false;
volatile bool up_button_interrupt       = false;
volatile bool down_button_interrupt     = false;
volatile bool enter_button_interrupt    = false;
volatile bool back_button_interrupt    = false;

bool main_light                         = false;
bool night_light                        = false;
bool auto_light_off                     = false;

int light_duration                      = 10;
unsigned long light_timer               = 0;

String ssid                             = "Wokwi-GUEST";
String pass                             = "";
String url_time                         = "http://worldtimeapi.org/api/timezone/Europe/Moscow";

struct tm stored_time;
unsigned long current_time_timer        = 0;

void IRAM_ATTR MainLightInterrupt() {
  if (millis() - debounce_timer >= DEBOUNCE_DELAY) {
    if (digitalRead(MAIN_LIGHT_BUTTON_1_PIN) == HIGH || digitalRead(MAIN_LIGHT_BUTTON_2_PIN) == HIGH) {
      main_light_interrupt = true;
    }
    debounce_timer = millis();
  }
}

void IRAM_ATTR UpInterrupt() {
  if (millis() - debounce_timer >= DEBOUNCE_DELAY) {
    if (digitalRead(UP_BUTTON_PIN) == HIGH) {
      up_button_interrupt = true;
    }
    debounce_timer = millis();
  }
}

void IRAM_ATTR DownInterrupt() {
  if (millis() - debounce_timer >= DEBOUNCE_DELAY) {
    if (digitalRead(DOWN_BUTTON_PIN) == HIGH) {
      down_button_interrupt = true;
    }
    debounce_timer = millis();
  }
}

void IRAM_ATTR EnterInterrupt() {
  if (millis() - debounce_timer >= DEBOUNCE_DELAY) {
    if (digitalRead(ENTER_BUTTON_PIN) == HIGH) {
      enter_button_interrupt = true;
    }
    debounce_timer = millis();
  }
}

void IRAM_ATTR BackInterrupt() {
  if (millis() - debounce_timer >= DEBOUNCE_DELAY) {
    if (digitalRead(BACK_BUTTON_PIN) == HIGH) {
      back_button_interrupt = true;
    }
    debounce_timer = millis();
  }
}

void ButtonsMainLight() {
  if (main_light_interrupt && digitalRead(MAIN_LIGHT_BUTTON_1_PIN) == LOW && digitalRead(MAIN_LIGHT_BUTTON_2_PIN) == LOW) {
    delay(DEBOUNCE_DELAY);
    if (main_light_interrupt && digitalRead(MAIN_LIGHT_BUTTON_1_PIN) == LOW && digitalRead(MAIN_LIGHT_BUTTON_2_PIN) == LOW) {
      main_light = !main_light;
      digitalWrite(MAIN_LIGHT_LED_PIN, main_light ? HIGH : LOW);
      main_light_interrupt = false;
    }
  }
}

void UpButton() {
  if (up_button_interrupt && digitalRead(UP_BUTTON_PIN) == LOW) {
    delay(DEBOUNCE_DELAY);
    if (up_button_interrupt && digitalRead(UP_BUTTON_PIN) == LOW) {
      Serial.println("UP CLICKED");
      up_button_interrupt = false;
    }
  }
}

void DownButton() {
  if (down_button_interrupt && digitalRead(DOWN_BUTTON_PIN) == LOW) {
    delay(DEBOUNCE_DELAY);
    if (down_button_interrupt && digitalRead(DOWN_BUTTON_PIN) == LOW) {
      Serial.println("DOWN CLICKED");
      down_button_interrupt = false;
    }
  }
}

void EnterButton() {
  if (enter_button_interrupt && digitalRead(ENTER_BUTTON_PIN) == LOW) {
    delay(DEBOUNCE_DELAY);
    if (enter_button_interrupt && digitalRead(ENTER_BUTTON_PIN) == LOW) {
      Serial.println("ENTER CLICKED");
      enter_button_interrupt = false;
    }
  }
}

void BackButton() {
  if (back_button_interrupt && digitalRead(BACK_BUTTON_PIN) == LOW) {
    delay(DEBOUNCE_DELAY);
    if (back_button_interrupt && digitalRead(BACK_BUTTON_PIN) == LOW) {
      Serial.println("BACK CLICKED");
      back_button_interrupt = false;
    }
  }
}

void WiFiSetup() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Init WiFi...");
  WiFi.begin(ssid, pass);
  unsigned long start_time = millis();
  
  while (WiFi.status() != WL_CONNECTED && millis() - start_time < (TIMEOUT_MS)) {
    delay(500);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 1);
    lcd.print("Init done!");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Connect failed");
  }
  delay(1000);
}

void GetTime() {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Receiving time...");
    http.begin(url_time);
    http.setTimeout(TIMEOUT_MS);
    int httpCode = http.GET();

    if (httpCode > 0) {
      lcd.setCursor(0, 1);
      lcd.print("Done");
      delay(1000);
      String message = http.getString();
      ReceiveMessage(message);
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Timeout error");
      delay(1000);
      DisplayCurrentTime();
    }
    http.end();
  } else {
    DisplayCurrentTime();
  }
}

void ReceiveMessage(String message) {
  StaticJsonDocument<1500> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    SetDefaultTime();
    DisplayCurrentTime();
    delay(1000);    
    return;
  } else {
    const char *datetime = doc["datetime"];
    int year, month, day, hour, minute, second;

    sscanf(datetime, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);

    stored_time.tm_year = year - 1900;
    stored_time.tm_mon  = month - 1;
    stored_time.tm_mday = day;
    stored_time.tm_hour = hour;
    stored_time.tm_min  = minute;
    stored_time.tm_sec  = second;

    time_t now = mktime(&stored_time);
    struct timeval tv = { now, 0 };    
    settimeofday(&tv, NULL);

    DisplayCurrentTime();
  }
}

void SetDefaultTime() {
  stored_time.tm_year = 2000;
  stored_time.tm_mon  = 1;
  stored_time.tm_mday = 1;
  stored_time.tm_hour = 0;
  stored_time.tm_min  = 0;
  stored_time.tm_sec  = 0;
  time_t now = mktime(&stored_time);
  struct timeval tv = { now, 0 };
  settimeofday(&tv, NULL);
}

void DisplayCurrentTime() {
  if (millis() - current_time_timer >= 1000) {
    if (!getLocalTime(&stored_time)) {
      lcd.setCursor(0, 0);
      lcd.print("Failed to obtain");
      lcd.setCursor(0, 1);
      lcd.print("local time");
      delay(1000);
      return;
    }

    lcd.setCursor(0, 0);
    lcd.print("Time:           ");
    lcd.setCursor(0, 1);
    lcd.printf("%02d:%02d:%02d        ", stored_time.tm_hour, stored_time.tm_min, stored_time.tm_sec);
    current_time_timer = millis();
  }
}

void CheckMotionSensor() {
  if (digitalRead(MOTION_SENSOR_PIN) == HIGH) {
    auto_light_off = true;
    // Day light
    if ((stored_time.tm_hour >= 7) && (stored_time.tm_hour <= 22 && stored_time.tm_min <= 59 && stored_time.tm_sec <= 59)) {   
      main_light = true;
      digitalWrite(MAIN_LIGHT_LED_PIN, main_light);   
    }
    // Night light
    else {
      night_light = true;
      digitalWrite(NIGHT_LIGHT_LED_PIN, night_light);
    }
  }
}

void LightOffTimer() {
  if (auto_light_off) {
    if (millis() - light_timer >= light_duration * 1000) {
      if (main_light) {
        main_light = false;
        digitalWrite(MAIN_LIGHT_LED_PIN, LOW);
      } else if (night_light) {
        night_light = false;
        digitalWrite(NIGHT_LIGHT_LED_PIN, LOW);
      }
      auto_light_off = false;
      light_timer = millis();
    }
  }
}

void ButtonsControl() {  
  ButtonsMainLight();
  UpButton();
  DownButton();
  BackButton();
  EnterButton();
}

void setup() {
  Serial.begin(115200);

  pinMode(MAIN_LIGHT_BUTTON_1_PIN, INPUT);
  pinMode(MAIN_LIGHT_BUTTON_2_PIN, INPUT);
  pinMode(UP_BUTTON_PIN, INPUT);
  pinMode(DOWN_BUTTON_PIN, INPUT);
  pinMode(ENTER_BUTTON_PIN, INPUT);

  pinMode(MAIN_LIGHT_LED_PIN, OUTPUT);
  pinMode(NIGHT_LIGHT_LED_PIN, OUTPUT);
  pinMode(ALARM_LIGHT_LED_PIN, OUTPUT);
  pinMode(MOTION_SENSOR_PIN, INPUT);

  digitalWrite(MAIN_LIGHT_LED_PIN, LOW);
  digitalWrite(NIGHT_LIGHT_LED_PIN, LOW);
  digitalWrite(ALARM_LIGHT_LED_PIN, LOW);
  
  attachInterrupt(MAIN_LIGHT_BUTTON_1_PIN, MainLightInterrupt, RISING);
  attachInterrupt(MAIN_LIGHT_BUTTON_2_PIN, MainLightInterrupt, RISING);
  attachInterrupt(UP_BUTTON_PIN, UpInterrupt, RISING);
  attachInterrupt(DOWN_BUTTON_PIN, DownInterrupt, RISING);
  attachInterrupt(ENTER_BUTTON_PIN, EnterInterrupt, RISING);
  attachInterrupt(BACK_BUTTON_PIN, BackInterrupt, RISING);

  lcd.init();
  lcd.backlight();

  SetDefaultTime();

  WiFiSetup();
  GetTime();
}

void loop() {
  DisplayCurrentTime();
  ButtonsControl();
  CheckMotionSensor();
  LightOffTimer();
  delay(20);
}
