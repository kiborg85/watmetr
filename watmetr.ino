#define PIN 12				// кнопка подключена сюда (PIN --- КНОПКА --- GND)

#include "GyverButton.h"
GButton butt1(PIN);

#include <GyverOLED.h>
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

int value = 0;

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
}

void loop() {
  butt1.tick();  
  if (butt1.isClick()) {
    value++;         
    float pi = 3.14*value;
  oled.print("PI = ");
  oled.print(pi);
  }

}
