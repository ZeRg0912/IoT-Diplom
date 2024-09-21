#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUTTON_UP_PIN     14
#define BUTTON_DOWN_PIN   12
#define BUTTON_ENTER_PIN  26
#define BUTTON_BACK_PIN   25
#define BUTTON_LEFT_PIN   13
#define BUTTON_RIGHT_PIN  27

#define DEBOUNCE_DELAY    50  // Задержка для антидребезга

LiquidCrystal_I2C lcd(0x27, 16, 2);

volatile bool upPressed = false;
volatile bool downPressed = false;
volatile bool enterPressed = false;
volatile bool backPressed = false;

unsigned long timer_lastUpPress = 0;
unsigned long timer_lastDownPress = 0;
unsigned long timer_lastEnterPress = 0;
unsigned long timer_lastBackPress = 0;
unsigned long timer_lastLeftPress = 0;  // Таймер для кнопки Left
unsigned long timer_lastRightPress = 0; // Таймер для кнопки Right

unsigned long timer_menuAFK = 0;

// Для отслеживания состояний кнопок
bool upState;
bool downState;
bool enterState;
bool backState;
bool leftState;   // Состояние кнопки Left
bool rightState;  // Состояние кнопки Right

// Для фиксации предыдущих состояний кнопок
bool lastUpState = LOW;
bool lastDownState = LOW;
bool lastEnterState = LOW;
bool lastBackState = LOW;
bool lastLeftState = LOW;   // Предыдущее состояние Left
bool lastRightState = LOW;  // Предыдущее состояние Right

enum MenuState { GREETING, MAIN_MENU, SUB_MENU };
MenuState currentMenu = GREETING;

int menuIndex = 0;          // Текущий индекс меню
int currentSubMenu = -1;    // Индекс текущего подменю
int charIndex = 0;          // Индекс текущего символа в строке

// Массив символов для выбора, включая пробел
char symbols[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
int numSymbols = sizeof(symbols) - 1;

String setting2 = "ABC"; // Настройка типа String, максимум 16 символов

// Переменные для других пунктов меню
int setting1 = 0;           // Настройка типа int
int setting3 = 10;          // Ещё одна настройка типа int

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  
  pinMode(BUTTON_UP_PIN, INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);
  pinMode(BUTTON_ENTER_PIN, INPUT);
  pinMode(BUTTON_BACK_PIN, INPUT);
  pinMode(BUTTON_LEFT_PIN, INPUT);  // Настройка пинов Left и Right
  pinMode(BUTTON_RIGHT_PIN, INPUT);

  showGreeting();
}

void loop() {
  handleButtonPress();
}

void CheckAFK() {
  if (currentMenu != GREETING) {
    if (millis() - timer_menuAFK >= 10000) {
      currentMenu = GREETING;
      showGreeting();
      timer_menuAFK = millis();
    }
  } else timer_menuAFK = millis();
}

void handleButtonPress() {
  unsigned long currentMillis = millis();
  CheckAFK();

  if (currentMillis - timer_menuAFK >= 10000 && currentMenu != GREETING) {
    currentMenu = GREETING;
    showGreeting();
    timer_menuAFK = millis();
  }

  // Чтение текущих состояний кнопок
  upState = digitalRead(BUTTON_UP_PIN);
  downState = digitalRead(BUTTON_DOWN_PIN);
  enterState = digitalRead(BUTTON_ENTER_PIN);
  backState = digitalRead(BUTTON_BACK_PIN);
  leftState = digitalRead(BUTTON_LEFT_PIN);   // Добавлено чтение для кнопки Left
  rightState = digitalRead(BUTTON_RIGHT_PIN); // Добавлено чтение для кнопки Right

  // Обработка кнопки "Up"
  if (upState == LOW && lastUpState == HIGH && (currentMillis - timer_lastUpPress > DEBOUNCE_DELAY)) {
    timer_lastUpPress = currentMillis;
    handleUpPress();
  }
  if (upState == HIGH && lastUpState == LOW) {
    timer_lastUpPress = currentMillis;
  }
  lastUpState = upState;

  // Обработка кнопки "Down"
  if (downState == LOW && lastDownState == HIGH && (currentMillis - timer_lastDownPress > DEBOUNCE_DELAY)) {
    timer_lastDownPress = currentMillis;
    handleDownPress();
  }
  if (downState == HIGH && lastDownState == LOW) {
    timer_lastDownPress = currentMillis;
  }
  lastDownState = downState;

  // Обработка кнопки "Enter"
  if (enterState == LOW && lastEnterState == HIGH && (currentMillis - timer_lastEnterPress > DEBOUNCE_DELAY)) {
    timer_lastEnterPress = currentMillis;
    handleEnterPress();
  }
  if (enterState == HIGH && lastEnterState == LOW) {
    timer_lastEnterPress = currentMillis;
  }
  lastEnterState = enterState;

  // Обработка кнопки "Back"
  if (backState == LOW && lastBackState == HIGH && (currentMillis - timer_lastBackPress > DEBOUNCE_DELAY)) {
    timer_lastBackPress = currentMillis;
    handleBackPress();
  }
  if (backState == HIGH && lastBackState == LOW) {
    timer_lastBackPress = currentMillis;
  }
  lastBackState = backState;

  // Обработка кнопки "Left"
  if (leftState == LOW && lastLeftState == HIGH && (currentMillis - timer_lastLeftPress > DEBOUNCE_DELAY)) {
    timer_lastLeftPress = currentMillis;
    handleLeftPress();
  }
  if (leftState == HIGH && lastLeftState == LOW) {
    timer_lastLeftPress = currentMillis;
  }
  lastLeftState = leftState;

  // Обработка кнопки "Right"
  if (rightState == LOW && lastRightState == HIGH && (currentMillis - timer_lastRightPress > DEBOUNCE_DELAY)) {
    timer_lastRightPress = currentMillis;
    handleRightPress();
  }
  if (rightState == HIGH && lastRightState == LOW) {
    timer_lastRightPress = currentMillis;
  }
  lastRightState = rightState;
}

void showGreeting() {
  Serial.println(setting2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome!");
  lcd.setCursor(0, 1);
  lcd.print("Press Enter...");
}

void showMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Menu:");
  lcd.setCursor(0, 1);
  switch (menuIndex) {
    case 0: lcd.print("Setting 1 (int)"); break;
    case 1: lcd.print("Setting 2 (string)"); break;
    case 2: lcd.print("Setting 3 (int)"); break;
  }
}

void showSubMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);

  // Показ названия параметра в зависимости от выбранного пункта
  switch (currentSubMenu) {
    case 0:
      lcd.print("Setting 1");
      break;
    case 1:
      lcd.print("Setting 2");
      break;
    case 2:
      lcd.print("Setting 3");
      break;
  }
  
  lcd.setCursor(0, 1);
  
  // Отображаем значение настройки в зависимости от выбранного пункта меню
  switch (currentSubMenu) {
    case 0:
      lcd.print(setting1);  // Отображаем int для setting1
      break;
    case 1:
      lcd.print(setting2);  // Отображаем String для setting2
      lcd.setCursor(charIndex, 1); // Устанавливаем курсор на текущий символ
      break;
    case 2:
      lcd.print(setting3);  // Отображаем int для setting3
      break;
  }
}

void handleUpPress() {
  timer_menuAFK = millis();
  if (currentMenu == MAIN_MENU) {
    menuIndex = (menuIndex - 1 + 3) % 3; // Меню из 3 пунктов
    showMainMenu();
  } else if (currentMenu == SUB_MENU) {
    if (currentSubMenu == 1) {  // Если изменяем строку setting2
      // Изменяем текущий символ на следующий в массиве symbols
      char currentChar = setting2[charIndex];
      int symbolIndex = findSymbolIndex(currentChar);
      symbolIndex = (symbolIndex + 1) % numSymbols;
      setting2[charIndex] = symbols[symbolIndex];
      showSubMenu();
    } else if (currentSubMenu == 0) {
      setting1++;  // Увеличиваем значение для setting1 (int)
      showSubMenu();
    } else if (currentSubMenu == 2) {
      setting3++;  // Увеличиваем значение для setting3 (int)
      showSubMenu();
    }
  }
}

void handleDownPress() {
  timer_menuAFK = millis();
  if (currentMenu == MAIN_MENU) {
    menuIndex = (menuIndex + 1) % 3; // Меню из 3 пунктов
    showMainMenu();
  } else if (currentMenu == SUB_MENU) {
    if (currentSubMenu == 1) {  // Если изменяем строку setting2
      // Изменяем текущий символ на предыдущий в массиве symbols
      char currentChar = setting2[charIndex];
      int symbolIndex = findSymbolIndex(currentChar);
      symbolIndex = (symbolIndex - 1 + numSymbols) % numSymbols;
      setting2[charIndex] = symbols[symbolIndex];
      showSubMenu();
    } else if (currentSubMenu == 0) {
      setting1--;  // Уменьшаем значение для setting1 (int)
      showSubMenu();
    } else if (currentSubMenu == 2) {
      setting3--;  // Уменьшаем значение для setting3 (int)
      showSubMenu();
    }
  }
}

void handleLeftPress() {
  if (currentSubMenu == 1 && charIndex > 0) {
    charIndex--;  // Перемещаемся влево по строке setting2
    showSubMenu();
  }
}

void handleRightPress() {
  if (currentSubMenu == 1) {
    // Если мы находимся на последнем символе строки и строка меньше 16 символов, добавляем новый символ (пробел)
    if (charIndex == setting2.length() - 1 && setting2.length() < 16) {
      setting2 += " ";  // Добавляем пробел в конец строки
    }
    if (charIndex < setting2.length() - 1) {
      charIndex++;  // Перемещаемся вправо по строке setting2
    }
    showSubMenu();
  }
}

void handleEnterPress() {
  timer_menuAFK = millis();
  if (currentMenu == GREETING) {
    currentMenu = MAIN_MENU;
    showMainMenu();
  } else if (currentMenu == MAIN_MENU) {
    currentMenu = SUB_MENU;
    currentSubMenu = menuIndex;  // Сохраняем, какое подменю выбрано
    showSubMenu();
  } else if (currentMenu == SUB_MENU) {
    // Применение значения
    removeTrailingSpaces();  // Удаляем пробелы в конце строки
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Value saved!");
    delay(1000);
    currentMenu = MAIN_MENU;
    showMainMenu();
  }
}

void handleBackPress() {
  timer_menuAFK = millis();
  if (currentMenu == SUB_MENU) {
    currentMenu = MAIN_MENU;
    showMainMenu();
  } else if (currentMenu == MAIN_MENU) {
    currentMenu = GREETING;
    showGreeting();
  }
}

// Функция для поиска индекса символа в массиве
int findSymbolIndex(char currentChar) {
  for (int i = 0; i < numSymbols; i++) {
    if (symbols[i] == currentChar) {
      return i;
    }
  }
  return 0;  // Если символ не найден
}

// Функция для удаления пробелов в конце строки
void removeTrailingSpaces() {
  int lastNonSpace = setting2.length() - 1;
  while (lastNonSpace >= 0 && setting2[lastNonSpace] == ' ') {
    lastNonSpace--;
  }
  setting2 = setting2.substring(0, lastNonSpace + 1);
}
