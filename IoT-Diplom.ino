#include "ConfigMap.h"
#include <LiquidCrystal_I2C.h>
#include "SmartHome.h"

LiquidCrystal_I2C lcd(0x27, MAX_ROW_LENGHT, MAX_ROWS);
SmartHome smartHome;

void InitPins() {
  pinMode(BUTTON_DAY_LIGHT_1_PIN, INPUT);
  pinMode(BUTTON_DAY_LIGHT_2_PIN, INPUT);
  pinMode(BUTTON_UP_PIN,          INPUT);
  pinMode(BUTTON_DOWN_PIN,        INPUT);
  pinMode(BUTTON_ENTER_PIN,       INPUT);
  pinMode(BUTTON_BACK_PIN,        INPUT);
  pinMode(BUTTON_LEFT_PIN,        INPUT);
  pinMode(BUTTON_RIGHT_PIN,       INPUT);
  pinMode(DAY_LIGHT_LED_PIN,      OUTPUT);
  pinMode(NIGHT_LIGHT_LED_PIN,    OUTPUT);
  pinMode(ALARM_LIGHT_LED_PIN,    OUTPUT);
  pinMode(MOTION_SENSOR_PIN,      INPUT);  

  digitalWrite(DAY_LIGHT_LED_PIN, LOW);
  digitalWrite(NIGHT_LIGHT_LED_PIN, LOW);
  digitalWrite(ALARM_LIGHT_LED_PIN, LOW); 
  
  attachInterrupt(BUTTON_DAY_LIGHT_1_PIN, [](){ smartHome.DayLightInterrupt(); }, RISING);
  attachInterrupt(BUTTON_DAY_LIGHT_2_PIN, [](){ smartHome.DayLightInterrupt(); }, RISING);
}

void setup() {
  InitPins();

  lcd.init();
  lcd.backlight();
  smartHome.Init();
}

void loop() {
  smartHome.Loop();
  delay(20);
}