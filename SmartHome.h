#ifndef SMARTHOME_H
#define SMARTHOME_H

#include "ConfigMap.h"
#include "WiFiManager.h"
#include "TimeManager.h"
#include <time.h>
#include <LiquidCrystal_I2C.h> 

extern LiquidCrystal_I2C lcd;

class SmartHome {
public:
  SmartHome();
  
  // Buttons
  void DayLightInterrupt();
  void UpPressed();
  void DownPressed();
  void LeftPressed();
  void RightPressed();
  void EnterPressed();
  void BackPressed();
  void CheckDayLightButtons();
  void CheckLCDButtons();
  
  // Sensor, alarm, and LCD time
  void CheckMotionSensor();
  void CheckAlarmTime();
  void ShowTime();

  // Menus
  void ShowMainMenu();
  void ShowSubMenu();

  //Utils
  void Init();
  void Loop();

  void BlinkingChar();
  int Findsymbol_index(char currentChar);
  void RemoveSpaces(String& str);
  void SetDefaultTime();
  void CheckAFK();
  bool CheckDayTime();
  void SetTimerMenuAFK();

  // WiFi
  void WiFiSetup();
  void GetTime();

private:    
  WiFiManager wifiManager;
  TimeManager timeManager;

  uint64_t timer_lastUpPress;
  uint64_t timer_lastDownPress;
  uint64_t timer_lastEnterPress;
  uint64_t timer_lastBackPress;
  uint64_t timer_lastLeftPress;
  uint64_t timer_lastRightPress;
  uint64_t timer_menuAFK;
  uint64_t timer_auto_off;
  uint64_t timer_current_time;
  uint64_t timer_alarm;
  uint64_t timer_blink_alarm;
  uint64_t timer_blink_char;
  volatile uint64_t timer_debounce;

  volatile bool day_light_interrupt;
  bool day_light;
  bool night_light;
  bool sensor_trig;
  bool alarm_trig;
  bool last_up_state;
  bool last_down_state;
  bool last_enter_state;
  bool last_back_state;
  bool last_left_state;
  bool last_right_state;    

  char symbols[64] = " AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789";
  int num_symbols;

  struct SmartHomeSettings {
      bool    night_light_sensor;
      int     night_light_duration;
      bool    day_light_sensor;
      int     day_light_duration;
      struct  tm alarm_time;
      bool    alarm;
      int     alarm_duration;
      String  SSID;
      String  password;
      struct  tm current_time;
  };
  
  SmartHomeSettings Settings;

  enum MenuState { TIME, MAIN_MENU, SUB_MENU };
  MenuState current_menu;
  int menu_index;
  int current_sub_menu;
  int SSID_char_index;
  int password_char_index;
  int current_time_char_index;
  int alarm_time_char_index;
  int old_time;
  struct tm temp_time;
};

#endif
