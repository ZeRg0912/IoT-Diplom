#include "WiFi.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>

#define BUTTON_DAY_LIGHT_1_PIN            33
#define BUTTON_DAY_LIGHT_2_PIN            32
#define BUTTON_UP_PIN                     14
#define BUTTON_DOWN_PIN                   12
#define BUTTON_ENTER_PIN                  26
#define BUTTON_BACK_PIN                   25
#define BUTTON_LEFT_PIN                   13
#define BUTTON_RIGHT_PIN                  27

#define DAY_LIGHT_LED_PIN                 19
#define NIGHT_LIGHT_LED_PIN               18
#define ALARM_LIGHT_LED_PIN               5
#define MOTION_SENSOR_PIN                 34

#define DEBOUNCE_DELAY                    50
#define MAX_ROW_LENGHT                    16
#define MAX_ROWS                          2
#define MENU_ITEMS                        10
#define DEBOUNCE_DELAY                    25
#define TIMEOUT_MS                        10000
#define AFK_DURATION                      30000

LiquidCrystal_I2C lcd(0x27, MAX_ROW_LENGHT, MAX_ROWS);

unsigned long timer_lastUpPress         = 0;
unsigned long timer_lastDownPress       = 0;
unsigned long timer_lastEnterPress      = 0;
unsigned long timer_lastBackPress       = 0;
unsigned long timer_lastLeftPress       = 0;
unsigned long timer_lastRightPress      = 0;
unsigned long timer_menuAFK             = 0;
unsigned long timer_auto_off            = 0;
unsigned long timer_current_time        = 0;
unsigned long timer_alarm               = 0;
unsigned long timer_blink_alarm         = 0;
unsigned long timer_blink_char          = 0;

// Для работы с прерывнаиями и кнопками основного света
volatile unsigned long timer_debounce   = 0;
volatile bool day_light_interrupt       = false;

bool day_light                          = false;
bool night_light                        = false;
bool sensor_trig                        = false;
bool alarm_trig                         = false;

// Для фиксации предыдущих состояний кнопок
bool last_up_state                      = LOW;
bool last_down_state                    = LOW;
bool last_enter_state                   = LOW;
bool last_back_state                    = LOW;
bool last_left_state                    = LOW;
bool last_right_state                   = LOW;

// Работа с меню настроек
enum MenuState { TIME, MAIN_MENU, SUB_MENU };
MenuState current_menu = TIME;

int menu_index                          = 0;
int current_sub_menu                    = -1;
int SSID_char_index                     = 0;
int password_char_index                 = 0;
int current_time_char_index             = 0;
int alarm_time_char_index               = 0;

int old_time                            = -1;

// Переменные для пунктов меню
struct SmartHomeSettings {
  bool    night_light_sensor            = false;
  int     night_light_duration          = 10;
  bool    day_light_sensor              = false;
  int     day_light_duration            = 10;
  struct  tm alarm_time;
  bool    alarm                         = false;
  int     alarm_duration                = 10;
  String  SSID                          = "Wokwi-GUEST";
  String  password                      = "";
  struct  tm current_time;
};

SmartHomeSettings Settings;

// Массив символов для выбора
char symbols[] = " AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789";
int num_symbols = sizeof(symbols) - 1;
// Строка адреса для получения времени
String url_time = "http://worldtimeapi.org/api/timezone/Europe/Moscow";


void IRAM_ATTR DayLightInterrupt() {
  if (millis() - timer_debounce >= DEBOUNCE_DELAY) {
    if (digitalRead(BUTTON_DAY_LIGHT_1_PIN) == HIGH || digitalRead(BUTTON_DAY_LIGHT_2_PIN) == HIGH) {
      day_light_interrupt = true;
    }
    timer_debounce = millis();
  }
}

void UpPressed() {
  timer_menuAFK = millis();
  if (current_menu == MAIN_MENU) {
    menu_index = (menu_index - 1 + MENU_ITEMS) % MENU_ITEMS;
    ShowMainMenu();
  } else if (current_menu == SUB_MENU) {
    if (current_sub_menu == 0) {
      Settings.night_light_sensor = !Settings.night_light_sensor;
    }

    else if (current_sub_menu == 1) {
      if (Settings.night_light_duration < 60) Settings.night_light_duration++;
    }

    else if (current_sub_menu == 2) {
      Settings.day_light_sensor = !Settings.day_light_sensor;
    }

    else if (current_sub_menu == 3) {
      if (Settings.day_light_duration < 600) Settings.day_light_duration++;
    }

    // Работа с будильником
    else if (current_sub_menu == 4) {  
      // Десятки часов
      if (alarm_time_char_index == 0) {  
        if (Settings.alarm_time.tm_hour < 20) Settings.alarm_time.tm_hour += 10;
        else Settings.alarm_time.tm_hour %= 10;  
      } 
      // Единицы часов
      else if (alarm_time_char_index == 1) { 
        if (Settings.alarm_time.tm_hour % 10 < 9) Settings.alarm_time.tm_hour++;
        if (Settings.alarm_time.tm_hour > 23) Settings.alarm_time.tm_hour = 0;
      } 
      // Десятки минут
      else if (alarm_time_char_index == 3) {  
        int units = Settings.alarm_time.tm_min % 10;
        if (Settings.alarm_time.tm_min < 50) Settings.alarm_time.tm_min = ((Settings.alarm_time.tm_min / 10 + 1) * 10) + units;
        else Settings.alarm_time.tm_min = units;  
      } 
      // Единицы минут
      else if (alarm_time_char_index == 4) {  
        if (Settings.alarm_time.tm_min % 10 < 9) Settings.alarm_time.tm_min++;
        else Settings.alarm_time.tm_min -= 9;  
      } 
      // Десятки секунд
      else if (alarm_time_char_index == 6) {  
        int units = Settings.alarm_time.tm_sec % 10;
        if (Settings.alarm_time.tm_sec < 50) Settings.alarm_time.tm_sec = ((Settings.alarm_time.tm_sec / 10 + 1) * 10) + units;
        else Settings.alarm_time.tm_sec = units;  
      } 
      // Единицы секунд
      else if (alarm_time_char_index == 7) { 
        if (Settings.alarm_time.tm_sec % 10 < 9) Settings.alarm_time.tm_sec++;
        else Settings.alarm_time.tm_sec -= 9;  
      }
    }

    else if (current_sub_menu == 5) {
      Settings.alarm = !Settings.alarm;
    }

    else if (current_sub_menu == 6) {
      if (Settings.alarm_duration < 600) Settings.alarm_duration++;
    }

    else if (current_sub_menu == 7) {
      int symbol_index = Findsymbol_index(Settings.SSID[SSID_char_index]);
      symbol_index = (symbol_index + 1) % num_symbols;
      Settings.SSID[SSID_char_index] = symbols[symbol_index];
    }

    else if (current_sub_menu == 8) {
      int symbol_index = Findsymbol_index(Settings.password[password_char_index]);
      symbol_index = (symbol_index + 1) % num_symbols;
      Settings.password[password_char_index] = symbols[symbol_index];
    }

    // Работа с текущим временем
    else if (current_sub_menu == 9) {  
      // Десятки часов
      if (current_time_char_index == 0) {  
        if (Settings.current_time.tm_hour < 20) Settings.current_time.tm_hour += 10;
        else Settings.current_time.tm_hour %= 10;  
      } 
      // Единицы часов
      else if (current_time_char_index == 1) {  
        if (Settings.current_time.tm_hour % 10 < 9) Settings.current_time.tm_hour++;
        if (Settings.current_time.tm_hour > 23) Settings.current_time.tm_hour = 0;
      } 
      // Десятки минут
      else if (current_time_char_index == 3) {  
        int units = Settings.current_time.tm_min % 10;
        if (Settings.current_time.tm_min < 50) Settings.current_time.tm_min = ((Settings.current_time.tm_min / 10 + 1) * 10) + units;
        else Settings.current_time.tm_min = units;  
      } 
      // Единицы минут
      else if (current_time_char_index == 4) {  
        if (Settings.current_time.tm_min % 10 < 9) Settings.current_time.tm_min++;
        else Settings.current_time.tm_min -= 9;  
      } 
      // Десятки секунд
      else if (current_time_char_index == 6) {  
        int units = Settings.current_time.tm_sec % 10;
        if (Settings.current_time.tm_sec < 50) Settings.current_time.tm_sec = ((Settings.current_time.tm_sec / 10 + 1) * 10) + units;
        else Settings.current_time.tm_sec = units;  
      } 
      // Единицы секунд
      else if (current_time_char_index == 7) {  
        if (Settings.current_time.tm_sec % 10 < 9) Settings.current_time.tm_sec++;
        else Settings.current_time.tm_sec -= 9;  
      }
    }

    ShowSubMenu();
  }
}

void DownPressed() {
  timer_menuAFK = millis();
  if (current_menu == MAIN_MENU) {
    menu_index = (menu_index + 1) % MENU_ITEMS;
    ShowMainMenu();
  } else if (current_menu == SUB_MENU) {
    if (current_sub_menu == 0) {
      Settings.night_light_sensor = !Settings.night_light_sensor;
    }

    else if (current_sub_menu == 1) {
      if (Settings.night_light_duration > 10) Settings.night_light_duration--;
    }

    else if (current_sub_menu == 2) {
      Settings.day_light_sensor = !Settings.day_light_sensor;
    }

    else if (current_sub_menu == 3) {
      if (Settings.day_light_duration > 10) Settings.day_light_duration--;
    }

    // Работа с будильником
    else if (current_sub_menu == 4) {  
      // Десятки часов
      if (alarm_time_char_index == 0) {  
        if (Settings.alarm_time.tm_hour >= 10) Settings.alarm_time.tm_hour -= 10;
        else Settings.alarm_time.tm_hour %= 10;  
      } 
      // Единицы часов
      else if (alarm_time_char_index == 1) {  
        if (Settings.alarm_time.tm_hour % 10 > 0) Settings.alarm_time.tm_hour--;
        if (Settings.alarm_time.tm_hour < 0) Settings.alarm_time.tm_hour = 23;
      } 
      // Десятки минут
      else if (alarm_time_char_index == 3) {  
        int units = Settings.alarm_time.tm_min % 10;
        if (Settings.alarm_time.tm_min >= 10) Settings.alarm_time.tm_min = ((Settings.alarm_time.tm_min / 10 - 1) * 10) + units;
        else Settings.alarm_time.tm_min = 50 + units;  
      } 
      // Единицы минут
      else if (alarm_time_char_index == 4) {  
        if (Settings.alarm_time.tm_min % 10 > 0) Settings.alarm_time.tm_min--;
        else Settings.alarm_time.tm_min += 9;  
      } 
      // Десятки секунд
      else if (alarm_time_char_index == 6) {  
        int units = Settings.alarm_time.tm_sec % 10;
        if (Settings.alarm_time.tm_sec >= 10) Settings.alarm_time.tm_sec = ((Settings.alarm_time.tm_sec / 10 - 1) * 10) + units;
        else Settings.alarm_time.tm_sec = 50 + units;  
      } 
      // Единицы секунд
      else if (alarm_time_char_index == 7) {  
        if (Settings.alarm_time.tm_sec % 10 > 0) Settings.alarm_time.tm_sec--;
        else Settings.alarm_time.tm_sec += 9;  
      }
    }

    else if (current_sub_menu == 5) {
      Settings.alarm = !Settings.alarm;
    }

    else if (current_sub_menu == 6) {
      if (Settings.alarm_duration > 10) Settings.alarm_duration--;
    }

    else if (current_sub_menu == 7) {
      int symbol_index = Findsymbol_index(Settings.SSID[SSID_char_index]);
      symbol_index = (symbol_index - 1 + num_symbols) % num_symbols;
      Settings.SSID[SSID_char_index] = symbols[symbol_index];
    }

    else if (current_sub_menu == 8) {
      int symbol_index = Findsymbol_index(Settings.password[password_char_index]);
      symbol_index = (symbol_index - 1 + num_symbols) % num_symbols;
      Settings.password[password_char_index] = symbols[symbol_index];
    }

    // Работа с текущим временем
    else if (current_sub_menu == 9) {
      // Десятки часов
      if (current_time_char_index == 0) {  
        if (Settings.current_time.tm_hour >= 10) Settings.current_time.tm_hour -= 10;
        else Settings.current_time.tm_hour %= 10;  
      } 
      // Единицы часов
      else if (current_time_char_index == 1) {  
        if (Settings.current_time.tm_hour % 10 > 0) Settings.current_time.tm_hour--;
        if (Settings.current_time.tm_hour < 0) Settings.current_time.tm_hour = 23;
      } 
      // Десятки минут
      else if (current_time_char_index == 3) { 
        int units = Settings.current_time.tm_min % 10;
        if (Settings.current_time.tm_min >= 10) Settings.current_time.tm_min = ((Settings.current_time.tm_min / 10 - 1) * 10) + units;
        else Settings.current_time.tm_min = 50 + units;  
      } 
      // Единицы минут
      else if (current_time_char_index == 4) {  
        if (Settings.current_time.tm_min % 10 > 0) Settings.current_time.tm_min--;
        else Settings.current_time.tm_min += 9;  
      } 
      // Десятки секунд
      else if (current_time_char_index == 6) {  
        int units = Settings.current_time.tm_sec % 10;
        if (Settings.current_time.tm_sec >= 10) Settings.current_time.tm_sec = ((Settings.current_time.tm_sec / 10 - 1) * 10) + units;
        else Settings.current_time.tm_sec = 50 + units;  
      } 
      // Единицы секунд
      else if (current_time_char_index == 7) {  
        if (Settings.current_time.tm_sec % 10 > 0) Settings.current_time.tm_sec--;
        else Settings.current_time.tm_sec += 9;  
      }
    }

    ShowSubMenu();
  }
}

void LeftPressed() {
  timer_menuAFK = millis();
  if (current_sub_menu == 4) {
    if (alarm_time_char_index == 1) alarm_time_char_index = 0;
    else if (alarm_time_char_index == 3) alarm_time_char_index = 1;
    else if (alarm_time_char_index == 4) alarm_time_char_index = 3;
    else if (alarm_time_char_index == 6) alarm_time_char_index = 4;
    else if (alarm_time_char_index == 7) alarm_time_char_index = 6;
  }

  if (current_sub_menu == 7 && SSID_char_index > 0) {
    SSID_char_index--;
  }

  if (current_sub_menu == 8 && password_char_index > 0) {
    password_char_index--;
  }

  if (current_sub_menu == 9) {
    if (current_time_char_index == 1) current_time_char_index = 0;
    else if (current_time_char_index == 3) current_time_char_index = 1;
    else if (current_time_char_index == 4) current_time_char_index = 3;
    else if (current_time_char_index == 6) current_time_char_index = 4;
    else if (current_time_char_index == 7) current_time_char_index = 6;
  }


  ShowSubMenu();
}

void RightPressed() {
  timer_menuAFK = millis();
  if (current_sub_menu == 4) {
    if (alarm_time_char_index == 0) alarm_time_char_index = 1;
    else if (alarm_time_char_index == 1) alarm_time_char_index = 3;
    else if (alarm_time_char_index == 3) alarm_time_char_index = 4;
    else if (alarm_time_char_index == 4) alarm_time_char_index = 6;
    else if (alarm_time_char_index == 6) alarm_time_char_index = 7;
  }

  if (current_sub_menu == 7) {
    if (SSID_char_index == Settings.SSID.length() - 1 && Settings.SSID.length() < MAX_ROW_LENGHT) {
      Settings.SSID += " ";
    }
    if (SSID_char_index < Settings.SSID.length() - 1) {
      SSID_char_index++;
    }
  }

  if (current_sub_menu == 8) {
    if (password_char_index == Settings.password.length() - 1 && Settings.password.length() < MAX_ROW_LENGHT) {
      Settings.password += " ";
    }
    if (password_char_index < Settings.password.length() - 1) {
      password_char_index++;
    }
  }

  if (current_sub_menu == 9) {
    if (current_time_char_index == 0) current_time_char_index = 1;
    else if (current_time_char_index == 1) current_time_char_index = 3;
    else if (current_time_char_index == 3) current_time_char_index = 4;
    else if (current_time_char_index == 4) current_time_char_index = 6;
    else if (current_time_char_index == 6) current_time_char_index = 7;
  }

  ShowSubMenu();
}

void EnterPressed() {
  timer_menuAFK = millis();
  if (current_menu == TIME) {
    current_menu = MAIN_MENU;
    ShowMainMenu();
  } else if (current_menu == MAIN_MENU) {
    current_menu = SUB_MENU;
    current_sub_menu = menu_index;
    ShowSubMenu();
  } else if (current_menu == SUB_MENU && current_sub_menu == 4) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm time set!");
    delay(500);
    current_menu = MAIN_MENU;
    ShowMainMenu();
  } else if (current_menu == SUB_MENU && current_sub_menu == 7) {
    RemoveSpaces(Settings.SSID);
    RemoveSpaces(Settings.password);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SSID set!");
    delay(500);
    WiFiSetup();
    GetTime();
    current_menu = MAIN_MENU;
    ShowMainMenu();
  } else if (current_menu == SUB_MENU && current_sub_menu == 8) {
    RemoveSpaces(Settings.SSID);
    RemoveSpaces(Settings.password);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Password set!");
    delay(500);
    WiFiSetup();
    GetTime();
    current_menu = MAIN_MENU;
    ShowMainMenu();
  } else if (current_menu == SUB_MENU && current_sub_menu == 9) {
    time_t new_time = mktime(&Settings.current_time);
    struct timeval tv = { new_time, 0 };
    settimeofday(&tv, NULL);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time set!");
    delay(1000);
    current_menu = MAIN_MENU;
    ShowMainMenu();
  } else if (current_menu == SUB_MENU) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Value saved!");
    delay(500);
    current_menu = MAIN_MENU;
    ShowMainMenu();
  }
}

void BackPressed() {
  timer_menuAFK = millis();
  if (current_menu == SUB_MENU && current_sub_menu == 4) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm time set!");
    delay(500);
    current_menu = MAIN_MENU;
    ShowMainMenu();
  } else if (current_menu == SUB_MENU && current_sub_menu == 9) {
    time_t new_time = mktime(&Settings.current_time);
    struct timeval tv = { new_time, 0 };
    settimeofday(&tv, NULL);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time set!");
    delay(500);
    current_menu = MAIN_MENU;
    ShowMainMenu();
  } else if (current_menu == SUB_MENU) {
    RemoveSpaces(Settings.SSID);
    RemoveSpaces(Settings.password);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Value saved!");
    delay(500);
    current_menu = MAIN_MENU;
    ShowMainMenu();
  } else if (current_menu == MAIN_MENU) {
    current_menu = TIME;
  }
}

void CheckDayLightButtons() {
  if (day_light_interrupt && digitalRead(BUTTON_DAY_LIGHT_1_PIN) == LOW && digitalRead(BUTTON_DAY_LIGHT_2_PIN) == LOW) {
    delay(DEBOUNCE_DELAY);
    if (day_light_interrupt && digitalRead(BUTTON_DAY_LIGHT_1_PIN) == LOW && digitalRead(BUTTON_DAY_LIGHT_2_PIN) == LOW) {
      day_light = !day_light;
      digitalWrite(DAY_LIGHT_LED_PIN, day_light ? HIGH : LOW);
      day_light_interrupt = false;
      if (CheckDayTime()) sensor_trig = false;
    }
  }
}

void CheckAFK() {
  if (current_menu != TIME) {
    if (millis() - timer_menuAFK >= AFK_DURATION) {
      current_menu = TIME;
      timer_menuAFK = millis();
    }
  } else timer_menuAFK = millis();
}

void CheckLCDButtons() {
  CheckAFK();
  BlinkingChar();

  // Чтение текущих состояний кнопок
  bool up_state    = digitalRead(BUTTON_UP_PIN);
  bool down_state  = digitalRead(BUTTON_DOWN_PIN);
  bool enter_state = digitalRead(BUTTON_ENTER_PIN);
  bool back_state  = digitalRead(BUTTON_BACK_PIN);
  bool left_state  = digitalRead(BUTTON_LEFT_PIN);
  bool right_state = digitalRead(BUTTON_RIGHT_PIN);

  // Обработка кнопки "Up"
  if (up_state == LOW && last_up_state == HIGH && (millis() - timer_lastUpPress > DEBOUNCE_DELAY)) {
    timer_lastUpPress = millis();
    UpPressed();
  }
  if (up_state == HIGH && last_up_state == LOW) {
    timer_lastUpPress = millis();
  }
  last_up_state = up_state;

  // Обработка кнопки "Down"
  if (down_state == LOW && last_down_state == HIGH && (millis() - timer_lastDownPress > DEBOUNCE_DELAY)) {
    timer_lastDownPress = millis();
    DownPressed();
  }
  if (down_state == HIGH && last_down_state == LOW) {
    timer_lastDownPress = millis();
  }
  last_down_state = down_state;

  // Обработка кнопки "Enter"
  if (enter_state == LOW && last_enter_state == HIGH && (millis() - timer_lastEnterPress > DEBOUNCE_DELAY)) {
    timer_lastEnterPress = millis();
    EnterPressed();
  }
  if (enter_state == HIGH && last_enter_state == LOW) {
    timer_lastEnterPress = millis();
  }
  last_enter_state = enter_state;

  // Обработка кнопки "Back"
  if (back_state == LOW && last_back_state == HIGH && (millis() - timer_lastBackPress > DEBOUNCE_DELAY)) {
    timer_lastBackPress = millis();
    BackPressed();
  }
  if (back_state == HIGH && last_back_state == LOW) {
    timer_lastBackPress = millis();
  }
  last_back_state = back_state;

  // Обработка кнопки "Left"
  if (left_state == LOW && last_left_state == HIGH && (millis() - timer_lastLeftPress > DEBOUNCE_DELAY)) {
    timer_lastLeftPress = millis();
    LeftPressed();
  }
  if (left_state == HIGH && last_left_state == LOW) {
    timer_lastLeftPress = millis();
  }
  last_left_state = left_state;

  // Обработка кнопки "Right"
  if (right_state == LOW && last_right_state == HIGH && (millis() - timer_lastRightPress > DEBOUNCE_DELAY)) {
    timer_lastRightPress = millis();
    RightPressed();
  }
  if (right_state == HIGH && last_right_state == LOW) {
    timer_lastRightPress = millis();
  }
  last_right_state = right_state;
}

bool CheckDayTime() {
  if ((Settings.current_time.tm_hour >= 7) && (Settings.current_time.tm_hour <= 22 && Settings.current_time.tm_min <= 59 && Settings.current_time.tm_sec <= 59)) {
    return true;
  } else return false;
}

void CheckMotionSensor() {
  if (digitalRead(MOTION_SENSOR_PIN) == HIGH) {
    sensor_trig = true;

    if (Settings.day_light_sensor && CheckDayTime()) {
      day_light = true;
      digitalWrite(DAY_LIGHT_LED_PIN, day_light);
    } else if (Settings.night_light_sensor && !CheckDayTime()) {
      night_light = true;
      digitalWrite(NIGHT_LIGHT_LED_PIN, night_light);
    }
    timer_auto_off = millis();

  } else if (sensor_trig) {

    if (Settings.day_light_sensor && CheckDayTime()) {
      if (millis() - timer_auto_off >= Settings.day_light_duration * 1000) {
        day_light = false;
        digitalWrite(DAY_LIGHT_LED_PIN, day_light);
        sensor_trig = false;
      }
    }

    if (Settings.night_light_sensor && !CheckDayTime()) {
      if (millis() - timer_auto_off >= Settings.night_light_duration * 1000) {
        night_light = false;
        digitalWrite(NIGHT_LIGHT_LED_PIN, night_light);
        sensor_trig = false;
      }
    }

  }
}

void CheckAlarmTime() {
  if (Settings.alarm) {
    if (Settings.current_time.tm_hour == Settings.alarm_time.tm_hour &&
        Settings.current_time.tm_min == Settings.alarm_time.tm_min &&
        Settings.current_time.tm_sec == Settings.alarm_time.tm_sec) {
      alarm_trig = true;
      digitalWrite(ALARM_LIGHT_LED_PIN, HIGH);
      digitalWrite(DAY_LIGHT_LED_PIN, HIGH);
      timer_alarm = millis();
    }
  }

  if (alarm_trig) {
    if (millis() - timer_blink_alarm <= 500) {
      digitalWrite(ALARM_LIGHT_LED_PIN, HIGH);
    } else if (millis() - timer_blink_alarm > 500 && millis() - timer_blink_alarm <= 1000) {
      digitalWrite(ALARM_LIGHT_LED_PIN, LOW);
    } else timer_blink_alarm = millis();
    if (millis() - timer_alarm >= Settings.alarm_duration * 1000) {
      alarm_trig = false;
      digitalWrite(ALARM_LIGHT_LED_PIN, LOW);
    }
  }
}

void ShowTime() {
  getLocalTime(&Settings.current_time);
  if (current_menu == TIME) {
    if (old_time != Settings.current_time.tm_sec) {
      lcd.setCursor(0, 0);
      lcd.print("      Time      ");
      lcd.setCursor(0, 1);
      lcd.printf("    %02d:%02d:%02d    ", Settings.current_time.tm_hour, Settings.current_time.tm_min, Settings.current_time.tm_sec);
      old_time = Settings.current_time.tm_sec;
    }
    menu_index = 0;
  }
}

void ShowMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Menu:");
  lcd.setCursor(0, 1);
  switch (menu_index) {
    case 0: lcd.print("Night sensor"); break;
    case 1: lcd.print("Night duration"); break;
    case 2: lcd.print("Day sensor"); break;
    case 3: lcd.print("Day duration"); break;
    case 4: lcd.print("Alarm time"); break;
    case 5: lcd.print("Alarm om/off"); break;
    case 6: lcd.print("Alarm duration"); break;
    case 7: lcd.print("SSID"); break;
    case 8: lcd.print("Password"); break;
    case 9: lcd.print("Time edit"); break;
  }
}

void ShowSubMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);

  switch (current_sub_menu) {
    case 0:
      lcd.print("Night sensor");
      break;
    case 1:
      lcd.print("Night duration");
      break;
    case 2:
      lcd.print("Day sensor");
      break;
    case 3:
      lcd.print("Day duration");
      break;
    case 4:
      lcd.print("Alarm time");
      break;
    case 5:
      lcd.print("Alarm on/off");
      break;
    case 6:
      lcd.print("Alarm duration");
      break;
    case 7:
      lcd.print("SSID");
      break;
    case 8:
      lcd.print("Password");
      break;
    case 9:
      lcd.print("Time edit");
      break;
  }
  lcd.print(':');

  lcd.setCursor(0, 1);

  switch (current_sub_menu) {
    case 0:
      lcd.print(Settings.night_light_sensor ? "TRUE" : "FALSE");
      break;
    case 1:
      lcd.print(Settings.night_light_duration);
      break;
    case 2:
      lcd.print(Settings.day_light_sensor ? "TRUE" : "FALSE");
      break;
    case 3:
      lcd.print(Settings.day_light_duration);
      break;
    case 4:
      lcd.printf("%02d:%02d:%02d", Settings.alarm_time.tm_hour, Settings.alarm_time.tm_min, Settings.alarm_time.tm_sec);
      lcd.setCursor(alarm_time_char_index, 1);
      break;
    case 5:
      lcd.print(Settings.alarm ? "TRUE" : "FALSE");
      break;
    case 6:
      lcd.print(Settings.alarm_duration);
      break;
    case 7:
      if (Settings.SSID == "") Settings.SSID = " ";
      lcd.print(Settings.SSID);
      lcd.setCursor(SSID_char_index, 1);
      break;
    case 8:
      if (Settings.password == "") Settings.password = " ";
      lcd.print(Settings.password);
      lcd.setCursor(password_char_index, 1);
      break;
    case 9:
      lcd.printf("%02d:%02d:%02d", Settings.current_time.tm_hour, Settings.current_time.tm_min, Settings.current_time.tm_sec);
      lcd.setCursor(current_time_char_index, 1);
      break;
  }
}

void BlinkingChar() {
  if (current_menu == SUB_MENU && (current_sub_menu == 4 || current_sub_menu == 7 || current_sub_menu == 8 || current_sub_menu == 9)) {
    int char_index;
    char current_char;
    String temp;

    switch (current_sub_menu) {
      case 4:  // Для времени будильника
        char_index = alarm_time_char_index;
        // Десятки часов
        if (char_index == 0) {  
          current_char = '0' + Settings.alarm_time.tm_hour / 10;
        } 
        // Единицы часов
        else if (char_index == 1) {  
          current_char = '0' + Settings.alarm_time.tm_hour % 10;
        } 
        // Десятки минут
        else if (char_index == 3) {  
          current_char = '0' + Settings.alarm_time.tm_min / 10;
        } 
        // Единицы минут
        else if (char_index == 4) {  
          current_char = '0' + Settings.alarm_time.tm_min % 10;
        } 
        // Десятки секунд
        else if (char_index == 6) {  
          current_char = '0' + Settings.alarm_time.tm_sec / 10;
        } 
        // Единицы секунд
        else if (char_index == 7) {  
          current_char = '0' + Settings.alarm_time.tm_sec % 10;
        }
        break;

      case 7:  // Для SSID
        char_index = SSID_char_index;
        current_char = Settings.SSID[SSID_char_index];
        break;

      case 8:  // Для пароля
        char_index = password_char_index;
        current_char = Settings.password[password_char_index];
        break;

      case 9:  // Для текущего времени
        char_index = current_time_char_index;
        // Десятки часов
        if (char_index == 0) {  
          current_char = '0' + Settings.current_time.tm_hour / 10;
        } 
        // Единицы часов
        else if (char_index == 1) {  
          current_char = '0' + Settings.current_time.tm_hour % 10;
        } 
        // Десятки минут
        else if (char_index == 3) {  
          current_char = '0' + Settings.current_time.tm_min / 10;
        } 
        // Единицы минут
        else if (char_index == 4) {  
          current_char = '0' + Settings.current_time.tm_min % 10;
        } 
        // Десятки секунд
        else if (char_index == 6) {  
          current_char = '0' + Settings.current_time.tm_sec / 10;
        } 
        // Единицы секунд
        else if (char_index == 7) {  
          current_char = '0' + Settings.current_time.tm_sec % 10;
        }
        break;
    }

    if (millis() - timer_blink_char <= 500) {
      lcd.setCursor(char_index, 1);
      lcd.print('_');
    } else if (millis() - timer_blink_char > 500 && millis() - timer_blink_char <= 1000) {
      lcd.setCursor(char_index, 1);
      lcd.print(current_char);
    } else timer_blink_char = millis();
  } else {
    alarm_time_char_index   = 0;
    SSID_char_index         = 0;
    password_char_index     = 0;
    current_time_char_index = 0;
  }
}

int Findsymbol_index(char currentChar) {
  for (int i = 0; i < num_symbols; i++) {
    if (symbols[i] == currentChar) {
      return i;
    }
  }
  return 0;
}

void RemoveSpaces(String& str) {
  while (str.endsWith(" ")) {
    str.remove(str.length() - 1);
  }
  if (str == " ") {
    str = "";
  }
}

void WiFiSetup() {  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Init WiFi");

  WiFi.begin(Settings.SSID, Settings.password);
  unsigned long start_time = millis();
  
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
  timer_menuAFK = millis();
}

void GetTime() {
  if ((WiFi.status() == WL_CONNECTED)) {
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
      ReceiveMessage(message);
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
    SetDefaultTime();
    delay(1000);
  }
}

void ReceiveMessage(String message) {
  StaticJsonDocument<1500> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    SetDefaultTime();
    delay(1000);
    return;
  } else {
    const char *datetime = doc["datetime"];
    int year, month, day, hour, minute, second;

    sscanf(datetime, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);

    Settings.current_time.tm_year = year - 1900;
    Settings.current_time.tm_mon  = month - 1;
    Settings.current_time.tm_mday = day;
    Settings.current_time.tm_hour = hour;
    Settings.current_time.tm_min  = minute;
    Settings.current_time.tm_sec  = second;

    time_t now = mktime(&Settings.current_time);
    struct timeval tv = { now, 0 };
    settimeofday(&tv, NULL);
  }
}

void SetDefaultTime() {
  Settings.current_time.tm_year = 2000;
  Settings.current_time.tm_mon  = 1;
  Settings.current_time.tm_mday = 1;
  Settings.current_time.tm_hour = 0;
  Settings.current_time.tm_min  = 0;
  Settings.current_time.tm_sec  = 0;

  time_t now = mktime(&Settings.current_time);
  struct timeval tv = { now, 0 };
  settimeofday(&tv, NULL);
}


void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_DAY_LIGHT_1_PIN, INPUT);
  pinMode(BUTTON_DAY_LIGHT_2_PIN, INPUT);
  pinMode(BUTTON_UP_PIN, INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);
  pinMode(BUTTON_ENTER_PIN, INPUT);
  pinMode(BUTTON_BACK_PIN, INPUT);
  pinMode(BUTTON_LEFT_PIN, INPUT);
  pinMode(BUTTON_RIGHT_PIN, INPUT);
  pinMode(DAY_LIGHT_LED_PIN, OUTPUT);
  pinMode(NIGHT_LIGHT_LED_PIN, OUTPUT);
  pinMode(ALARM_LIGHT_LED_PIN, OUTPUT);
  pinMode(MOTION_SENSOR_PIN, INPUT);

  digitalWrite(DAY_LIGHT_LED_PIN, LOW);
  digitalWrite(NIGHT_LIGHT_LED_PIN, LOW);
  digitalWrite(ALARM_LIGHT_LED_PIN, LOW);

  attachInterrupt(BUTTON_DAY_LIGHT_1_PIN, DayLightInterrupt, RISING);
  attachInterrupt(BUTTON_DAY_LIGHT_2_PIN, DayLightInterrupt, RISING);

  lcd.init();
  lcd.backlight();
  
  Settings.alarm_time.tm_hour = 0;
  Settings.alarm_time.tm_min  = 0;
  Settings.alarm_time.tm_sec  = 0;

  SetDefaultTime();
  WiFiSetup();

  GetTime();
}

void loop() {
  ShowTime();
  CheckLCDButtons();
  CheckDayLightButtons();
  CheckMotionSensor();
  CheckAlarmTime();
  delay(20);
}