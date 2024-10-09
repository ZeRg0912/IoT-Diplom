#include "SmartHome.h"

SmartHome::SmartHome() {
  Settings.night_light_sensor       = false;
  Settings.night_light_duration     = 10;
  Settings.day_light_sensor         = false;
  Settings.day_light_duration       = 10;

  Settings.alarm_time.tm_hour       = 0;
  Settings.alarm_time.tm_min        = 0;
  Settings.alarm_time.tm_sec        = 0;

  Settings.current_time.tm_year     = 0;
  Settings.current_time.tm_mon      = 0;
  Settings.current_time.tm_mday     = 0;

  Settings.alarm                    = false;
  Settings.alarm_duration           = 10;

  Settings.SSID                     = BASE_SSID;
  Settings.password                 = BASE_PASSWORD;

  num_symbols                       = sizeof(symbols) - 1;

  timer_lastUpPress                 = 0;
  timer_lastDownPress               = 0;
  timer_lastEnterPress              = 0;
  timer_lastBackPress               = 0;
  timer_lastLeftPress               = 0;
  timer_lastRightPress              = 0;
  timer_menuAFK                     = 0;
  timer_auto_off                    = 0;
  timer_current_time                = 0;
  timer_alarm                       = 0;
  timer_blink_alarm                 = 0;
  timer_blink_char                  = 0;
  timer_debounce                    = 0;

  day_light_interrupt               = false;
  day_light                         = false;
  night_light                       = false;
  sensor_trig                       = false;
  alarm_trig                        = false;

  last_up_state                     = LOW;
  last_down_state                   = LOW;
  last_enter_state                  = LOW;
  last_back_state                   = LOW;
  last_left_state                   = LOW;
  last_right_state                  = LOW;    

  menu_index                        = 0;
  current_sub_menu                  = -1;
  SSID_char_index                   = 0;
  password_char_index               = 0;
  current_time_char_index           = 0;
  alarm_time_char_index             = 0;
  old_time                          = -1;  
}

// Buttons
void SmartHome::DayLightInterrupt() {
  if (millis() - timer_debounce >= DEBOUNCE_DELAY) {
    if (digitalRead(BUTTON_DAY_LIGHT_1_PIN) == HIGH || digitalRead(BUTTON_DAY_LIGHT_2_PIN) == HIGH) {
      day_light_interrupt = true;
    }
    timer_debounce = millis();
  }
}

void SmartHome::UpPressed() {
  int symbol_index;
  timer_menuAFK = millis();

  switch (current_menu) {
    case MAIN_MENU:
      menu_index = (menu_index - 1 + MENU_ITEMS) % MENU_ITEMS;
      ShowMainMenu();
      break;

    case SUB_MENU:
      switch (current_sub_menu) {
        case 0:
          Settings.night_light_sensor = !Settings.night_light_sensor;
          ShowSubMenu();
          break;

        case 1:
          if (Settings.night_light_duration < MAX_NIGHT_DURATION) {
            Settings.night_light_duration++;
          }
          ShowSubMenu();
          break;

        case 2:
          Settings.day_light_sensor = !Settings.day_light_sensor;
          ShowSubMenu();
          break;

        case 3:
          if (Settings.day_light_duration < MAX_MAIN_DURATION) {
            Settings.day_light_duration++;
          }
          ShowSubMenu();
          break;

        case 4:
          switch (alarm_time_char_index) {
            case 0:  // десятки часов
              if (Settings.alarm_time.tm_hour < 20) {
                Settings.alarm_time.tm_hour += 10;
                if (Settings.alarm_time.tm_hour > 23) {
                  Settings.alarm_time.tm_hour = 23;  // Ограничение 23 часа
                }
              }
              break;

            case 1:  // единицы часов
              if (Settings.alarm_time.tm_hour >= 20) {
                if (Settings.alarm_time.tm_hour % 10 < 3) {
                  Settings.alarm_time.tm_hour++;
                } else {
                  Settings.alarm_time.tm_hour = 23;  // Ограничение 23 часа
                }
              } else {
                if (Settings.alarm_time.tm_hour % 10 < 9) {
                  Settings.alarm_time.tm_hour++;
                } else {
                  Settings.alarm_time.tm_hour -= 9;
                }
              }
              break;

            case 3:  // десятки минут
              if (Settings.alarm_time.tm_min < 50) {
                Settings.alarm_time.tm_min += 10;
              } else {
                Settings.alarm_time.tm_min = 0;  // Циклический переход на 0
              }
              break;

            case 4:  // единицы минут
              if (Settings.alarm_time.tm_min % 10 < 9) {
                Settings.alarm_time.tm_min++;
              } else {
                Settings.alarm_time.tm_min -= 9;
              }
              break;

            case 6:  // десятки секунд
              if (Settings.alarm_time.tm_sec < 50) {
                Settings.alarm_time.tm_sec += 10;
              } else {
                Settings.alarm_time.tm_sec = 0;  // Циклический переход на 0
              }
              break;

            case 7:  // единицы секунд
              if (Settings.alarm_time.tm_sec % 10 < 9) {
                Settings.alarm_time.tm_sec++;
              } else {
                Settings.alarm_time.tm_sec -= 9;
              }
              break;
          }
          ShowSubMenu();
          break;

        case 5:
          Settings.alarm = !Settings.alarm;
          ShowSubMenu();
          break;

        case 6:
          if (Settings.alarm_duration < MAX_ALARM_DURATION) {
            Settings.alarm_duration++;
          }
          ShowSubMenu();
          break;

        case 7:
          symbol_index = Findsymbol_index(Settings.SSID[SSID_char_index]);
          symbol_index = (symbol_index + 1) % num_symbols;
          Settings.SSID[SSID_char_index] = symbols[symbol_index];
          ShowSubMenu();
          break;

        case 8:
          symbol_index = Findsymbol_index(Settings.password[password_char_index]);
          symbol_index = (symbol_index + 1) % num_symbols;
          Settings.password[password_char_index] = symbols[symbol_index];
          ShowSubMenu();
          break;

        case 9:
          switch (current_time_char_index) {
            case 0:  // десятки часов
              if (temp_time.tm_hour < 20) {
                temp_time.tm_hour += 10;
                if (temp_time.tm_hour > 23) {
                  temp_time.tm_hour = 23;  // Ограничение 23 часа
                }
              }
              break;

            case 1:  // единицы часов
              if (temp_time.tm_hour >= 20) {
                if (temp_time.tm_hour % 10 < 3) {
                  temp_time.tm_hour++;
                } else {
                  temp_time.tm_hour = 23;  // Ограничение 23 часа
                }
              } else {
                if (temp_time.tm_hour % 10 < 9) {
                  temp_time.tm_hour++;
                } else {
                  temp_time.tm_hour -= 9;
                }
              }
              break;

            case 3:  // десятки минут
              if (temp_time.tm_min < 50) {
                temp_time.tm_min += 10;
              } else {
                temp_time.tm_min = 0;  // Циклический переход на 0
              }
              break;

            case 4:  // единицы минут
              if (temp_time.tm_min % 10 < 9) {
                temp_time.tm_min++;
              } else {
                temp_time.tm_min -= 9;
              }
              break;

            case 6:  // десятки секунд
              if (temp_time.tm_sec < 50) {
                temp_time.tm_sec += 10;
              } else {
                temp_time.tm_sec = 0;  // Циклический переход на 0
              }
              break;

            case 7:  // единицы секунд
              if (temp_time.tm_sec % 10 < 9) {
                temp_time.tm_sec++;
              } else {
                temp_time.tm_sec -= 9;
              }
              break;
          }
          ShowSubMenu();
          break;
      }
  }
}

void SmartHome::DownPressed() {
  int symbol_index;
  timer_menuAFK = millis();

  switch (current_menu) {
    case MAIN_MENU:
      menu_index = (menu_index + 1) % MENU_ITEMS;
      ShowMainMenu();
      break;

    case SUB_MENU:
      switch (current_sub_menu) {
        case 0:
          Settings.night_light_sensor = !Settings.night_light_sensor;
          ShowSubMenu();
          break;

        case 1:
          if (Settings.night_light_duration > MIN_DURATION) {
            Settings.night_light_duration--;
          }
          ShowSubMenu();
          break;

        case 2:
          Settings.day_light_sensor = !Settings.day_light_sensor;
          ShowSubMenu();
          break;

        case 3:
          if (Settings.day_light_duration > MIN_DURATION) {
            Settings.day_light_duration--;
          }
          ShowSubMenu();
          break;

        case 4:
          switch (alarm_time_char_index) {
            case 0:  // десятки часов
              if (Settings.alarm_time.tm_hour >= 10) {
                Settings.alarm_time.tm_hour -= 10;
              } else {
                Settings.alarm_time.tm_hour = 20;  // Циклический переход на 20
              }
              break;

            case 1:  // единицы часов
              if (Settings.alarm_time.tm_hour >= 20) {
                if (Settings.alarm_time.tm_hour % 10 > 0) {
                  Settings.alarm_time.tm_hour--;
                } else {
                  Settings.alarm_time.tm_hour = 23;  // Ограничение 23 часа
                }
              } else {
                if (Settings.alarm_time.tm_hour % 10 > 0) {
                  Settings.alarm_time.tm_hour--;
                } else {
                  Settings.alarm_time.tm_hour += 9;
                }
              }
              break;

            case 3:  // десятки минут
              if (Settings.alarm_time.tm_min >= 10) {
                Settings.alarm_time.tm_min -= 10;
              } else {
                Settings.alarm_time.tm_min = 50;  // Циклический переход на 50
              }
              break;

            case 4:  // единицы минут
              if (Settings.alarm_time.tm_min % 10 > 0) {
                Settings.alarm_time.tm_min--;
              } else {
                Settings.alarm_time.tm_min += 9;
              }
              break;

            case 6:  // десятки секунд
              if (Settings.alarm_time.tm_sec >= 10) {
                Settings.alarm_time.tm_sec -= 10;
              } else {
                Settings.alarm_time.tm_sec = 50;  // Циклический переход на 50
              }
              break;

            case 7:  // единицы секунд
              if (Settings.alarm_time.tm_sec % 10 > 0) {
                Settings.alarm_time.tm_sec--;
              } else {
                Settings.alarm_time.tm_sec += 9;
              }
              break;
          }
          ShowSubMenu();
          break;

        case 5:
          Settings.alarm = !Settings.alarm;
          ShowSubMenu();
          break;

        case 6:
          if (Settings.alarm_duration > MIN_DURATION) {
            Settings.alarm_duration--;
          }
          ShowSubMenu();
          break;

        case 7:
          symbol_index = Findsymbol_index(Settings.SSID[SSID_char_index]);
          symbol_index = (symbol_index - 1 + num_symbols) % num_symbols;
          Settings.SSID[SSID_char_index] = symbols[symbol_index];
          ShowSubMenu();
          break;

        case 8:
          symbol_index = Findsymbol_index(Settings.password[password_char_index]);
          symbol_index = (symbol_index - 1 + num_symbols) % num_symbols;
          Settings.password[password_char_index] = symbols[symbol_index];
          ShowSubMenu();
          break;

        case 9:
          switch (current_time_char_index) {
            case 0:  // десятки часов
              if (temp_time.tm_hour >= 10) {
                temp_time.tm_hour -= 10;
              } else {
                temp_time.tm_hour = 20;  // Циклический переход на 20
              }
              break;

            case 1:  // единицы часов
              if (temp_time.tm_hour >= 20) {
                if (temp_time.tm_hour % 10 > 0) {
                  temp_time.tm_hour--;
                } else {
                  temp_time.tm_hour = 23;  // Ограничение 23 часа
                }
              } else {
                if (temp_time.tm_hour % 10 > 0) {
                  temp_time.tm_hour--;
                } else {
                  temp_time.tm_hour += 9;
                }
              }
              break;

            case 3:  // десятки минут
              if (temp_time.tm_min >= 10) {
                temp_time.tm_min -= 10;
              } else {
                temp_time.tm_min = 50;  // Циклический переход на 50
              }
              break;

            case 4:  // единицы минут
              if (temp_time.tm_min % 10 > 0) {
                temp_time.tm_min--;
              } else {
                temp_time.tm_min += 9;
              }
              break;

            case 6:  // десятки секунд
              if (temp_time.tm_sec >= 10) {
                temp_time.tm_sec -= 10;
              } else {
                temp_time.tm_sec = 50;  // Циклический переход на 50
              }
              break;

            case 7:  // единицы секунд
              if (temp_time.tm_sec % 10 > 0) {
                temp_time.tm_sec--;
              } else {
                temp_time.tm_sec += 9;
              }
              break;
          }
          ShowSubMenu();
          break;
      }
  }
}

void SmartHome::LeftPressed() {
  timer_menuAFK = millis();

  switch (current_sub_menu) {
    case 4:
      switch(alarm_time_char_index) {
        case 1: alarm_time_char_index = 0; break;
        case 3: alarm_time_char_index = 1; break;
        case 4: alarm_time_char_index = 3; break;
        case 6: alarm_time_char_index = 4; break;
        case 7: alarm_time_char_index = 6; break;
      }
      ShowSubMenu();
      break;

    case 7:
      if (SSID_char_index > 0) SSID_char_index--;
      ShowSubMenu();
      break;

    case 8:
      if (password_char_index > 0) password_char_index--;
      ShowSubMenu();
      break;

    case 9:
      switch(current_time_char_index) {
        case 1: current_time_char_index = 0; break;
        case 3: current_time_char_index = 1; break;
        case 4: current_time_char_index = 3; break;
        case 6: current_time_char_index = 4; break;
        case 7: current_time_char_index = 6; break;
      }
      ShowSubMenu();
      break;
  }
}

void SmartHome::RightPressed() {
  timer_menuAFK = millis();

  switch (current_sub_menu) {
    case 4:
      switch(alarm_time_char_index) {
        case 0: alarm_time_char_index = 1; break;
        case 1: alarm_time_char_index = 3; break;
        case 3: alarm_time_char_index = 4; break;
        case 4: alarm_time_char_index = 6; break;
        case 6: alarm_time_char_index = 7; break;
      }
      ShowSubMenu();
      break;

    case 7:
      if (SSID_char_index == Settings.SSID.length() - 1 && Settings.SSID.length() < MAX_ROW_LENGHT) {
        Settings.SSID += " ";
      }
      if (SSID_char_index < Settings.SSID.length() - 1) SSID_char_index++;
      ShowSubMenu();
      break;

    case 8:
      if (password_char_index == Settings.password.length() - 1 && Settings.password.length() < MAX_ROW_LENGHT) {
        Settings.password += " ";
      }
      if (password_char_index < Settings.password.length() - 1) password_char_index++;
      ShowSubMenu();
      break;

    case 9:
      switch(current_time_char_index) {
        case 0: current_time_char_index = 1; break;
        case 1: current_time_char_index = 3; break;
        case 3: current_time_char_index = 4; break;
        case 4: current_time_char_index = 6; break;
        case 6: current_time_char_index = 7; break;
      }
      ShowSubMenu();
      break;
  }
}

void SmartHome::EnterPressed() {
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
    timeManager.SetTime(temp_time);
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

void SmartHome::BackPressed() {
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

void SmartHome::CheckDayLightButtons() {
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

void SmartHome::CheckLCDButtons() {
  CheckAFK();
  BlinkingChar();

  bool up_state     = digitalRead(BUTTON_UP_PIN);
  bool down_state   = digitalRead(BUTTON_DOWN_PIN);
  bool enter_state  = digitalRead(BUTTON_ENTER_PIN);
  bool back_state   = digitalRead(BUTTON_BACK_PIN);
  bool left_state   = digitalRead(BUTTON_LEFT_PIN);
  bool right_state  = digitalRead(BUTTON_RIGHT_PIN);

  if (current_menu != TIME) {
    if (up_state == LOW && last_up_state == HIGH && (millis() - timer_lastUpPress > DEBOUNCE_DELAY)) {
        timer_lastUpPress = millis();
        UpPressed();
    }
    last_up_state = up_state;

    if (down_state == LOW && last_down_state == HIGH && (millis() - timer_lastDownPress > DEBOUNCE_DELAY)) {
        timer_lastDownPress = millis();
        DownPressed();
    }
    last_down_state = down_state;    

    if (back_state == LOW && last_back_state == HIGH && (millis() - timer_lastBackPress > DEBOUNCE_DELAY)) {
        timer_lastBackPress = millis();
        BackPressed();
    }
    last_back_state = back_state;

    if (left_state == LOW && last_left_state == HIGH && (millis() - timer_lastLeftPress > DEBOUNCE_DELAY)) {
        timer_lastLeftPress = millis();
        LeftPressed();
    }
    last_left_state = left_state;

    if (right_state == LOW && last_right_state == HIGH && (millis() - timer_lastRightPress > DEBOUNCE_DELAY)) {
        timer_lastRightPress = millis();
        RightPressed();
    }
    last_right_state = right_state;    
  }

  if (enter_state == LOW && last_enter_state == HIGH && (millis() - timer_lastEnterPress > DEBOUNCE_DELAY)) {
        timer_lastEnterPress = millis();
        EnterPressed();
  }
  last_enter_state = enter_state;
}

// Menus
void SmartHome::ShowMainMenu() {
  temp_time = Settings.current_time;
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

void SmartHome::ShowSubMenu() {
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
      lcd.printf("%02d:%02d:%02d", temp_time.tm_hour, temp_time.tm_min, temp_time.tm_sec);
      lcd.setCursor(current_time_char_index, 1);
      break;
  }
}

// Sensor, Alarm, LCD time
void SmartHome::CheckMotionSensor() {
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

void SmartHome::CheckAlarmTime() {
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

void SmartHome::ShowTime() {
  timeManager.GetCurrentTime(Settings.current_time);
  if (current_menu == TIME) {
    if (old_time != Settings.current_time.tm_sec) {
      lcd.setCursor(0, 0);
      lcd.print("      Time      ");
      char buffer[9];
      timeManager.FormatTime(buffer, Settings.current_time);
      lcd.setCursor(0, 1);
      lcd.print("    ");
      lcd.print(buffer);
      lcd.print("    ");
      old_time = Settings.current_time.tm_sec;
    }
    menu_index = 0;
  }
}

// WiFi
void SmartHome::WiFiSetup() {
  wifiManager.WiFiSetup(Settings.SSID, Settings.password);
}

void SmartHome::GetTime() {
  wifiManager.GetTime(Settings.current_time);
}

// Utils
void SmartHome::BlinkingChar() {
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
          current_char = '0' + temp_time.tm_hour / 10;
        } 
        // Единицы часов
        else if (char_index == 1) {  
          current_char = '0' + temp_time.tm_hour % 10;
        } 
        // Десятки минут
        else if (char_index == 3) {  
          current_char = '0' + temp_time.tm_min / 10;
        } 
        // Единицы минут
        else if (char_index == 4) {  
          current_char = '0' + temp_time.tm_min % 10;
        } 
        // Десятки секунд
        else if (char_index == 6) {  
          current_char = '0' + temp_time.tm_sec / 10;
        } 
        // Единицы секунд
        else if (char_index == 7) {  
          current_char = '0' + temp_time.tm_sec % 10;
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

int SmartHome::Findsymbol_index(char currentChar) {
  for (int i = 0; i < num_symbols; i++) {
    if (symbols[i] == currentChar) {
      return i;
    }
  }
  return 0;
}

void SmartHome::RemoveSpaces(String& str) {
  while (str.endsWith(" ")) {
    str.remove(str.length() - 1);
  }
  if (str == " ") {
    str = "";
  }
}

void SmartHome::CheckAFK() {
  if (current_menu != TIME) {
    if (millis() - timer_menuAFK >= AFK_DURATION) {
      current_menu = TIME;
      timer_menuAFK = millis();
    }
  } else timer_menuAFK = millis();
}

bool SmartHome::CheckDayTime() {
  if ((Settings.current_time.tm_hour >= 7) && (Settings.current_time.tm_hour <= 22 && Settings.current_time.tm_min <= 59 && Settings.current_time.tm_sec <= 59)) {
    return true;
  } else return false;
}

void SmartHome::SetDefaultTime() {
  timeManager.SetDefaultTime(Settings.current_time);
}


void SmartHome::Init(){  
  SetDefaultTime();
  WiFiSetup();
  GetTime();
}

void SmartHome::Loop(){  
  ShowTime();
  CheckLCDButtons();
  CheckDayLightButtons();
  CheckMotionSensor();
  CheckAlarmTime();
}

void SmartHome::SetTimerMenuAFK(){
  timer_menuAFK = millis();
}