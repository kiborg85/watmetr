#define PIN 12				// кнопка подключена сюда (PIN --- КНОПКА --- GND)
#define PIN_tok A0
#define NUM_READS 20  //количество измерений вольтажа для вычисления среднего
const float typVbg = 1.095; // 1.0 -- 1.2

#include "GyverButton.h"
GButton butt1(PIN);

#include <TroykaCurrent.h>
ACS712 dataI(PIN_tok);

#include <GyverOLED.h>
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

int value = 0;
const int v_tok = A0;
const int v_volt = A7;
bool dc;
static uint32_t tmr_readVolt;
unsigned long prevMillis;
float val_tok; 
float val_volt;
float Vcc;
float cap = 0; //начальная ёмкость
float Wh = 0;

void setup() {
  Serial.begin(9600);
  butt1.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  butt1.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
  butt1.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)


  // HIGH_PULL - кнопка подключена к GND, пин подтянут к VCC (PIN --- КНОПКА --- GND)
  // LOW_PULL  - кнопка подключена к VCC, пин подтянут к GND
  butt1.setType(LOW_PULL);

  // NORM_OPEN - нормально-разомкнутая кнопка
  // NORM_CLOSE - нормально-замкнутая кнопка
  butt1.setDirection(NORM_OPEN);


  oled.init();        // инициализация
  oled.clear();       // очистка
  oled.setScale(3);   // масштаб текста (1..4)
  oled.home();        // курсор в 0,0
  oled.print("Привет!");
  delay(1000);
  oled.setScale(1);
  // курсор на начало 3 строки
  oled.setCursor(0, 3);
  oled.print("GyverOLED v1.3.2");
  // курсор на (20, 50)
  oled.setCursorXY(20, 50);
  float pi = 3.14;
  oled.print("PI = ");
  oled.print(pi);

  dc=1;
  oled.clear();       // очистка
    prevMillis = millis();  //время первого шага
 Serial.print("!!! STARTED !!!"); Serial.print("\tDC = "); Serial.println(dc);
}

void loop() {
  butt1.tick();  
  if (butt1.isClick()) { Serial.print("Click");    if ( dc == 1) { dc=0; oled.clear(); } else {dc = 1;}; Serial.print("\tDC = "); Serial.println(dc);}

  if (millis() - tmr_readVolt > 1000) {  //считываем значение вольтажа аккумулятора
      tmr_readVolt = millis();
      if (dc == 1 ) {val_tok = (dataI.readCurrentDC()); } else {val_tok = (dataI.readCurrentAC());}
      oled.setScale(1);   // масштаб текста (1..4)
      oled.home();        // курсор в 0,0
      oled.print("Ток = "); oled.print(val_tok); 
      oled.setCursor(0, 1);
      if (dc == 1) {oled.print(" DC");} else {oled.print(" AC");}
      Serial.print ("val_tok = "); Serial.print (val_tok); //Serial.print (" Vcc = "); Serial.println (Vcc); //Serial.print (" val_sred = "); Serial.print (val_sred);
    if (dc == 1) {
      Vcc = readVcc(); //хитрое считывание опорного напряжения (функция readVcc() находится ниже)
      val_volt = (readAnalog(A7) * Vcc)/1023/0.3075;
      cap += val_tok*(millis()-prevMillis)/3600000*1000; //расчет емкости АКБ в мАч
      Wh += val_tok*val_volt *(millis() - prevMillis)/3600000; //расчет емкости АКБ в ВтЧ
      prevMillis = millis();
      Serial.print ("\tval_volt = "); Serial.print (val_volt);Serial.print (" \tVcc = "); Serial.println (Vcc); //Serial.print (" val_sred = "); Serial.print (val_sred);
      oled.setCursor(0, 3); oled.print("Напряжение = "); oled.print(val_volt);
      oled.setCursor(0, 4); oled.print("Опорное = "); oled.print(cap); 
      oled.setCursor(0, 5); oled.print("мАч = "); oled.print(Vcc); oled.print("ВтЧ = "); oled.print(Wh);
      oled.setCursor(0, 6); oled.print("Мощьность = "); oled.print(val_tok*val_volt);
    } else {Serial.println();}
  }
}

//----------фильтр данных (для уменьшения шумов и разброса данных)-------
float readVcc() {
  // read multiple values and sort them to take the mode
  float sortedValues[NUM_READS];
  for (int i = 0; i < NUM_READS; i++) {
      butt1.tick(); if (butt1.isClick()) {  Serial.print("Click");  if ( dc == 1) { dc=0; oled.clear();} else {dc = 1;}; Serial.print("\tDC = "); Serial.println(dc); }
    float tmp = 0.0;
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    ADCSRA |= _BV(ADSC); // Start conversion
    delay(25);
    while (bit_is_set(ADCSRA, ADSC)); // measuring
    uint8_t low = ADCL; // must read ADCL first - it then locks ADCH
    uint8_t high = ADCH; // unlocks both
    tmp = (high << 8) | low;
    float value = (typVbg * 1023.0) / tmp;
    int j;
    if (value < sortedValues[0] || i == 0) {
      j = 0; //insert at first position
    }
    else {
      for (j = 1; j < i; j++) {
        if (sortedValues[j - 1] <= value && sortedValues[j] >= value) {
          // j is insert position
          break;
        }
      }
    }
    for (int k = i; k > j; k--) {
      // move all values higher than current reading up one position
      sortedValues[k] = sortedValues[k - 1];
    }
    sortedValues[j] = value; //insert current reading
  }
  //return scaled mode of 10 values
  float returnval = 0;
  for (int i = NUM_READS / 2 - 5; i < (NUM_READS / 2 + 5); i++) {
    returnval += sortedValues[i];
  }
  return returnval / 10;
}
//----------фильтр данных (для уменьшения шумов и разброса данных) КОНЕЦ-------
//----------Функция точного определения опорного напряжения для измерения напряжения на акуме-------
float readAnalog(int pin) {  
  // read multiple values and sort them to take the mode
  int sortedValues[NUM_READS];
  for (int i = 0; i < NUM_READS; i++) {
      butt1.tick();   if (butt1.isClick()) {  Serial.print("Click");  if ( dc == 1) { dc=0; oled.clear(); } else {dc = 1;}; Serial.print("\tDC = "); Serial.println(dc);} 
    delay(25);    
    int value = analogRead(pin);
    int j;
    if (value < sortedValues[0] || i == 0) {
      j = 0; //insert at first position
    }
    else {
      for (j = 1; j < i; j++) {
        if (sortedValues[j - 1] <= value && sortedValues[j] >= value) {
          // j is insert position
          break;
        }
      }
    }
    for (int k = i; k > j; k--) {
      // move all values higher than current reading up one position
      sortedValues[k] = sortedValues[k - 1];
    }
    sortedValues[j] = value; //insert current reading
  }
  //return scaled mode of 10 values
  float returnval = 0;
  for (int i = NUM_READS / 2 - 5; i < (NUM_READS / 2 + 5); i++) {
    returnval += sortedValues[i];
  }
  return returnval / 10;
}
//----------Функция точного определения опорного напряжения для измерения напряжения на акуме КОНЕЦ-------
