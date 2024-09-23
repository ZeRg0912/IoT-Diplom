#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUTTON_MAIN_LIGHT_1_PIN         33
#define BUTTON_MAIN_LIGHT_2_PIN         32
#define BUTTON_UP_PIN                   14
#define BUTTON_DOWN_PIN                 12
#define BUTTON_ENTER_PIN                26
#define BUTTON_BACK_PIN                 25
#define BUTTON_LEFT_PIN                 13
#define BUTTON_RIGHT_PIN                27

#define MAIN_LIGHT_LED_PIN              19
#define NIGHT_LIGHT_LED_PIN             18
#define ALARM_LIGHT_LED_PIN             5
#define MOTION_SENSOR_PIN               34

#define DEBOUNCE_DELAY                  50
#define MAX_ROW_LENGHT                  16
#define MAX_ROWS                        2
#define MENU_ITEMS                      10
#define DEBOUNCE_DELAY                  25
#define TIMEOUT_MS                      5000

LiquidCrystal_I2C lcd(0x27, MAX_ROW_LENGHT, MAX_ROWS);

unsigned long timer_lastUpPress         = 0;
unsigned long timer_lastDownPress       = 0;
unsigned long timer_lastEnterPress      = 0;
unsigned long timer_lastBackPress       = 0;
unsigned long timer_lastLeftPress       = 0; 
unsigned long timer_lastRightPress      = 0;
unsigned long timer_menuAFK             = 0;
unsigned long timer_current_char        = 0;

// Для работы с прерывнаиями и кнопками основного света
volatile unsigned long timer_debounce   = 0;
volatile bool main_light_interrupt      = false;
bool main_light                         = false;

// Для отслеживания состояний кнопок
bool up_state;
bool down_state;
bool enter_state;
bool back_state;
bool left_state;
bool right_state;

// Для фиксации предыдущих состояний кнопок
bool last_up_state     = LOW;
bool last_down_state   = LOW;
bool last_enter_state  = LOW;
bool last_back_state   = LOW;
bool last_left_state   = LOW;
bool last_right_state  = LOW;

// Работа с меню настроек
enum MenuState { TIME, MAIN_MENU, SUB_MENU };
MenuState current_menu = TIME;

int menu_index                  = 0;
int current_sub_menu            = -1;
int SSID_char_index             = 0;
int password_char_index         = 0;

// Переменные для пунктов меню
struct SmartHome {
  bool    night_light               = false;       
  int     night_light_duration      = 10;    
  bool    day_light                 = false;       
  int     day_light_duration        = 10;   
  int     alarm_time                = 0;      
  bool    alarm                     = false;    
  int     alarm_duration            = 10;               
  String  SSID                      = "Wokwi-GUEST";         
  String  password                  = ""; 
  int     current_time              = 0;        
};

SmartHome Settings;

// Массив символов для выбора
char symbols[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
int num_symbols = sizeof(symbols) - 1;
// Строка адреса для получения времени
String url_time = "http://worldtimeapi.org/api/timezone/Europe/Moscow";


void IRAM_ATTR MainLightInterrupt() {
  if (millis() - timer_debounce >= DEBOUNCE_DELAY) {
    if (digitalRead(BUTTON_MAIN_LIGHT_1_PIN) == HIGH || digitalRead(BUTTON_MAIN_LIGHT_2_PIN) == HIGH) {
      main_light_interrupt = true;
    }
    timer_debounce = millis();
  }
}

void CheckMainLightButtons() {
  if (main_light_interrupt && digitalRead(BUTTON_MAIN_LIGHT_1_PIN) == LOW && digitalRead(BUTTON_MAIN_LIGHT_2_PIN) == LOW) {
    delay(DEBOUNCE_DELAY);
    if (main_light_interrupt && digitalRead(BUTTON_MAIN_LIGHT_1_PIN) == LOW && digitalRead(BUTTON_MAIN_LIGHT_2_PIN) == LOW) {
      main_light = !main_light;
      digitalWrite(MAIN_LIGHT_LED_PIN, main_light ? HIGH : LOW);
      main_light_interrupt = false;
    }
  }
}

void CheckAFK() {
  if (current_menu != TIME) {
    if (millis() - timer_menuAFK >= 10000) {
      current_menu = TIME;
      ShowTime();
      timer_menuAFK = millis();
    }
  } else timer_menuAFK = millis();
}

void CheckLCDButtons() {
  CheckAFK();

  // Чтение текущих состояний кнопок
  up_state    = digitalRead(BUTTON_UP_PIN);
  down_state  = digitalRead(BUTTON_DOWN_PIN);
  enter_state = digitalRead(BUTTON_ENTER_PIN);
  back_state  = digitalRead(BUTTON_BACK_PIN);
  left_state  = digitalRead(BUTTON_LEFT_PIN);
  right_state = digitalRead(BUTTON_RIGHT_PIN);

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

void ShowTime() {
  Serial.println(Settings.SSID);
  Serial.println(Settings.password);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome!");
  lcd.setCursor(0, 1);
  lcd.print("Press Enter...");
}

void ShowMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Menu:");
  lcd.setCursor(0, 1);
  switch (menu_index) {
    case 0: lcd.print("Night light sensor"); break;
    case 1: lcd.print("Night light duration"); break;
    case 2: lcd.print("Day light sensor"); break;
    case 3: lcd.print("Day light duration"); break;
    case 4: lcd.print("Alarm time"); break;
    case 5: lcd.print("Alarm om/off"); break;
    case 6: lcd.print("Alarm duration"); break;
    case 7: lcd.print("SSID"); break;
    case 8: lcd.print("Password"); break;
    case 9: lcd.print("Current time edit"); break;
  }
}

void ShowSubMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);

  switch (current_sub_menu) {
    case 0:
      lcd.print("Night light sensor");
      break;
    case 1:
      lcd.print("Night light duration");
      break;
    case 2:
      lcd.print("Day light sensor");
      break;
    case 3:
      lcd.print("Day light duration");
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
      lcd.print("Current time edit");
      break;
  }
  lcd.print(':');
  
  lcd.setCursor(0, 1);
  
  switch (current_sub_menu) {
    case 0:
      lcd.print(Settings.night_light ? "TRUE" : "FALSE");
      break;
    case 1:
      lcd.print(Settings.night_light_duration);
      break;
    case 2:
      lcd.print(Settings.day_light ? "TRUE" : "FALSE");
      break;
    case 3:
      lcd.print(Settings.day_light_duration);
      break;
    case 4:
      lcd.print(Settings.alarm_time);
      break;
    case 5:
      lcd.print(Settings.alarm ? "TRUE" : "FALSE");
      break;
    case 6:
      lcd.print(Settings.alarm_duration);
      break;
    case 7:
      lcd.print(Settings.SSID);
      lcd.setCursor(SSID_char_index, 1);
      break;
    case 8:
      lcd.print(Settings.password);
      lcd.setCursor(password_char_index, 1);
      break;
    case 9:
      lcd.print(Settings.current_time);
      break;
  }
}

void UpPressed() {
  timer_menuAFK = millis();
  if (current_menu == MAIN_MENU) {
    menu_index = (menu_index - 1 + MENU_ITEMS) % MENU_ITEMS;
    ShowMainMenu();
  } else if (current_menu == SUB_MENU) {
    if (current_sub_menu == 0) {
      Settings.night_light = !Settings.night_light;
    }

    else if (current_sub_menu == 1) {
      if (Settings.night_light_duration < 60) Settings.night_light_duration++;
    }  

    else if (current_sub_menu == 2) {
      Settings.day_light = !Settings.day_light;
    }

    else if (current_sub_menu == 3) {
      if (Settings.day_light_duration < 600) Settings.day_light_duration++;
    } 

    else if (current_sub_menu == 4) {
      Settings.alarm_time++;
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

    else if (current_sub_menu == 9) {
      if (Settings.current_time < 24) Settings.current_time++;
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
      Settings.night_light = !Settings.night_light;
    }

    else if (current_sub_menu == 1) {
      if (Settings.night_light_duration > 10) Settings.night_light_duration--;
    }  

    else if (current_sub_menu == 2) {
      Settings.day_light = !Settings.day_light;
    }

    else if (current_sub_menu == 3) {
      if (Settings.day_light_duration > 10) Settings.day_light_duration--;
    } 

    else if (current_sub_menu == 4) {
      Settings.alarm_time--;
    } 

    else if (current_sub_menu == 5) {
      Settings.alarm = !Settings.alarm;
    }

    else if (current_sub_menu == 6) {
      if (Settings.alarm_duration > 10) Settings.alarm_duration--;
    } 

    else if (current_sub_menu == 7) {      
      if (millis() - timer_current_char >= 1000) {
        int symbol_index = Findsymbol_index(Settings.SSID[SSID_char_index]);
        symbol_index = (symbol_index - 1 + num_symbols) % num_symbols;
        Settings.SSID[SSID_char_index] = symbols[symbol_index];
      } else {
        Settings.SSID[SSID_char_index] = " ";
      }
    } 

    else if (current_sub_menu == 8) {
      int symbol_index = Findsymbol_index(Settings.password[password_char_index]);
      symbol_index = (symbol_index - 1 + num_symbols) % num_symbols;
      Settings.password[password_char_index] = symbols[symbol_index];
    }

    else if (current_sub_menu == 9) {
      if (Settings.current_time > 0) Settings.current_time--;
    }  

    ShowSubMenu();
  }
}

void LeftPressed() {
  if (current_sub_menu == 7 && SSID_char_index > 0) {
    SSID_char_index--;
    ShowSubMenu();
  }

  if (current_sub_menu == 8 && password_char_index > 0) {
    password_char_index--;
    ShowSubMenu();
  }
}

void RightPressed() {
  if (current_sub_menu == 7) {
    if (SSID_char_index == Settings.SSID.length() - 1 && Settings.SSID.length() < MAX_ROW_LENGHT) {
      Settings.SSID += " ";
    }
    if (SSID_char_index < Settings.SSID.length() - 1) {
      SSID_char_index++;
    }
    ShowSubMenu();
  }

  if (current_sub_menu == 8) {
    if (password_char_index == Settings.password.length() - 1 && Settings.password.length() < MAX_ROW_LENGHT) {
      Settings.password += " ";
    }
    if (password_char_index < Settings.password.length() - 1) {
      password_char_index++;
    }
    ShowSubMenu();
  }

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
  } else if (current_menu == SUB_MENU) {
    RemoveSpaces(Settings.SSID);
    RemoveSpaces(Settings.password);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Value saved!");
    delay(1000);
    current_menu = MAIN_MENU;
    ShowMainMenu();
  }
}

void BackPressed() {
  timer_menuAFK = millis();
  if (current_menu == SUB_MENU) {
    RemoveSpaces(Settings.SSID);
    RemoveSpaces(Settings.password);
    current_menu = MAIN_MENU;
    ShowMainMenu();
  } else if (current_menu == MAIN_MENU) {
    current_menu = TIME;
    ShowTime();
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
  int last_non_space = str.length() - 1;
  while (last_non_space >= 0 && str[last_non_space] == ' ') {
    last_non_space--;
  }
  str = str.substring(0, last_non_space + 1);
}


void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_MAIN_LIGHT_1_PIN, INPUT);
  pinMode(BUTTON_MAIN_LIGHT_2_PIN, INPUT);
  pinMode(BUTTON_UP_PIN, INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);
  pinMode(BUTTON_ENTER_PIN, INPUT);
  pinMode(BUTTON_BACK_PIN, INPUT);
  pinMode(BUTTON_LEFT_PIN, INPUT);
  pinMode(BUTTON_RIGHT_PIN, INPUT);
  pinMode(MAIN_LIGHT_LED_PIN, OUTPUT);
  pinMode(NIGHT_LIGHT_LED_PIN, OUTPUT);
  pinMode(ALARM_LIGHT_LED_PIN, OUTPUT);
  pinMode(MOTION_SENSOR_PIN, INPUT);
  
  digitalWrite(MAIN_LIGHT_LED_PIN, LOW);
  digitalWrite(NIGHT_LIGHT_LED_PIN, LOW);
  digitalWrite(ALARM_LIGHT_LED_PIN, LOW);
  
  attachInterrupt(BUTTON_MAIN_LIGHT_1_PIN, MainLightInterrupt, RISING);
  attachInterrupt(BUTTON_MAIN_LIGHT_2_PIN, MainLightInterrupt, RISING);  

  lcd.init();
  lcd.backlight();

  ShowTime();
}

void loop() {
  CheckLCDButtons();
  CheckMainLightButtons();
}