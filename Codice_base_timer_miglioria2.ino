/*Modifiche principali:
Variabile batteryOk: Dichiarata per tenere traccia dello stato della batteria.
Controllo della batteria nel setup: Verifica se l'anno è inferiore al 2023 per determinare se la batteria è scarica.
Funzione displayBatteryStatus: Visualizza lo stato della batteria sull'LCD nella quarta riga.
Con queste modifiche, il display LCD mostrerà lo stato della batteria sulla quarta riga, permettendoti di sapere se la batteria del modulo RTC è scarica.*/

#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>

// Dichiarazione delle variabili globali
RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4); // Modificato per un display 20x4

const char daysOfTheWeek[7][10] = {"Domenica", "Lunedi", "Martedi", "Mercoledi", "Giovedi", "Venerdi", "Sabato"};

const byte BUTTON_UP = 2;
const byte BUTTON_DOWN = 3;
const byte BUTTON_SELECT = 4;
const byte ALARM_PIN = 12;

byte x = 0, minutos = 0, horas = 0, minutos_d = 0, horas_d = 0;
bool batteryOk = true;

void setup() {
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(ALARM_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();

  Serial.begin(57600);

  if (!rtc.begin()) {
    Serial.println("Impossibile trovare RTC");
    Serial.flush();
    abort();
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC NON sta funzionando, impostiamo l'ora!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Controlla la data e l'ora per verificare la batteria
  DateTime now = rtc.now();
  if (now.year() < 2023) { // Supponiamo che qualsiasi anno precedente al 2023 indichi un problema con la batteria
    batteryOk = false;
  }
}

void loop() {
  DateTime now = rtc.now();

  displayDateTime(now);
  displayMenu();
  displayBatteryStatus();
  handleButtonPress();
  manageAlarm(now);

  delay(100); // Aggiunta di un piccolo delay per il debounce dei pulsanti
}

void displayDateTime(DateTime now) {
  char dateStr[17];
  sprintf(dateStr, "%02d/%02d/%04d", now.day(), now.month(), now.year());

  lcd.setCursor(0, 0);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(' ');
  lcd.setCursor(10, 0);
  lcd.print(dateStr);

  lcd.setCursor(0, 1);
  printTime(now.hour(), 0, 1);
  lcd.print(':');
  printTime(now.minute(), 3, 1);
  lcd.print(':');
  printTime(now.second(), 6, 1);
  lcd.setCursor(8, 1);
  lcd.print(' ');
  lcd.setCursor(9, 1);
  lcd.print("Men:");
}

void printTime(int timeUnit, int col, int row) {
  lcd.setCursor(col, row);
  if (timeUnit < 10) {
    lcd.print('0');
  }
  lcd.print(timeUnit, DEC);
}

void displayMenu() {
  lcd.setCursor(13, 1);
  switch (x) {
    case 0:
      lcd.print("Set    ");
      break;
    case 1:
      lcd.print("Min/On ");
      break;
    case 2:
      lcd.print("Ore/On ");
      break;
    case 3:
      lcd.print("Min/Off");
      break;
    case 4:
      lcd.print("Ore/Off");
      break;
    case 5:
      lcd.print("Attivo ");
      break;
    default:
      lcd.print("       ");
      break;
  }

  lcd.setCursor(0, 2);
  lcd.print("On=");
  printTime(horas, 3, 2);
  lcd.print(':');
  printTime(minutos, 6, 2);

  lcd.setCursor(10, 2); // Posizione corretta per "Off="
  lcd.print("Off=");
  printTime(horas_d, 14, 2); // Stampa dell'ora di spegnimento corretta
  lcd.print(':');
  printTime(minutos_d, 17, 2); // Stampa dei minuti di spegnimento corretta
}

void displayBatteryStatus() {
  lcd.setCursor(0, 3);
  if (batteryOk) {
    lcd.print("Batteria OK  ");
  } else {
    lcd.print("Batteria SCARICA");
  }
}

void handleButtonPress() {
  if (digitalRead(BUTTON_UP) == LOW) {
    x = (x + 1) % 6;
    delay(50);
  }

  switch (x) {
    case 1:
      adjustTime(&minutos, 60);
      break;
    case 2:
      adjustTime(&horas, 24);
      break;
    case 3:
      adjustTime(&minutos_d, 60);
      break;
    case 4:
      adjustTime(&horas_d, 24);
      break;
  }
}

void adjustTime(byte *timeUnit, byte maxVal) {
  if (digitalRead(BUTTON_SELECT) == LOW) {
    *timeUnit = (*timeUnit + 1) % maxVal;
    delay(50);
  }
  if (digitalRead(BUTTON_DOWN) == LOW) {
    *timeUnit = (*timeUnit == 0) ? maxVal - 1 : *timeUnit - 1;
    delay(50);
  }
}

void manageAlarm(DateTime now) {
  if (x == 5) {
    if (now.hour() == horas && now.minute() == minutos) {
      digitalWrite(ALARM_PIN, HIGH);
    }
    if (now.hour() == horas_d && now.minute() == minutos_d) {
      digitalWrite(ALARM_PIN, LOW);
    }
  }
}
