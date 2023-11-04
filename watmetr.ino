#define PIN 12				// кнопка подключена сюда (PIN --- КНОПКА --- GND)
#include "GyverButton.h"
GButton butt1(PIN);

#include <GyverINA.h>
INA219 ina;                       // Стандартный набор параметров для Arduino модуля (0.1, 3.2, 0x40)

#include <GyverOLED.h>
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

//int value = 0;
unsigned long prevMillis;
float val_tok; 
float val_volt;
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

Serial.print(F("INA219..."));



  oled.init();        // инициализация
  oled.clear();       // очистка
  oled.setScale(3);   // масштаб текста (1..4)
  oled.home();        // курсор в 0,0
  oled.print("Привет!");
  oled.setScale(1); oled.setCursor(0, 2);

    // Проверяем наличие и инициализируем INA219
  if (ina.begin()) {	// ina.begin(4, 5) // Для ESP32/ESP8266 можно указать пины I2C
    Serial.println(F("connected!"));
    oled.print("INA219 connected");
  } else {
    Serial.println(F("not found!"));
    oled.print("INA219 not found!");

    while (1);
  }
    Serial.print(F("Calibration value: ")); Serial.println(ina.getCalibration());
  ina.setResolution(INA219_VBUS, INA219_RES_12BIT_X4);      // Напряжение в 12ти битном режиме + 4х кратное усреднение
  ina.setResolution(INA219_VSHUNT, INA219_RES_12BIT_X128);  // Ток в 12ти битном режиме + 128х кратное усреднение

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

  oled.clear();       // очистка
    prevMillis = millis();  //время первого шага
 Serial.print("!!! STARTED !!!"); 
}

void loop() {
  butt1.tick();  
  if (butt1.isClick()) { Serial.print("Click");   }
  // Читаем напряжение
  Serial.print(F("Voltage: "));
  Serial.print(ina.getVoltage(), 3);
  Serial.println(F(" V"));

  // Читаем ток
  Serial.print(F("Current: "));
  Serial.print(ina.getCurrent(), 3);
  Serial.println(F(" A"));

  // Читаем мощность
  Serial.print(F("Power: "));
  Serial.print(ina.getPower(), 3);
  Serial.println(F(" W"));

  // Читаем напряжение на шунте
  Serial.print(F("Shunt voltage: "));
  Serial.print(ina.getShuntVoltage(), 6);
  Serial.println(F(" V"));

//oled.clear(); 
  oled.home(); oled.print("Voltage (V): "); oled.print(ina.getVoltage(), 3);
  oled.setCursor(0, 1); oled.print("Current (I): "); oled.print(ina.getCurrent(), 3);
  oled.setCursor(0, 2); oled.print("Power (W): "); oled.print(ina.getPower(), 3);
  oled.setCursor(0, 3); oled.print("Shunt (V): "); oled.print(ina.getShuntVoltage(), 5);
  Serial.println("");
  delay(1000);
  
   
 
}

