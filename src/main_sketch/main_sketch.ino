#include <SPI.h>
#include "Ucglib.h"

const uint8_t pump_pin = A2;

const uint8_t moistureSensor_powerPin = 13;
const uint8_t moistureSensor_readPin = A0;

const int moistureSensor_pollRate = 5000; /* check the sensor every 5s */
unsigned long moistureSensor_lastPoll = 0;
const short moisture_threshold = 720; /* 0 - 1023 */
unsigned int pump_lastStart = 0;
const int pump_downTime = 20000; /* wait 20 seconds even if still dry */
const int pump_activeDuration = 3000;

int moisture = 0;

Ucglib_ST7735_18x128x160_SWSPI ucg(/*scl=*/ 12, /*data=*/ 11, /*cd=*/ 9, /*cs=*/ 10, /*reset=*/ 8);

void setup() {
  Serial.begin(9600);

  pinMode(moistureSensor_powerPin, OUTPUT);
  pinMode(moistureSensor_readPin, INPUT);

  pinMode(pump_pin, OUTPUT);

  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.clearScreen();
  // ucg.setRotate180();

  ucg.setColor(0,0,0);
  ucg.drawBox(0,0, ucg.getWidth(), ucg.getHeight());
}

void loop() {
  unsigned long currentTime = millis();

  ucg.setFont(ucg_font_ncenR12_tr);
  ucg.setColor(255, 255, 255);

  int y = 0;
  int h = 14;

  y += h;
  ucg.setFontMode(UCG_FONT_MODE_TRANSPARENT);
  ucg.setPrintPos(4,y);
  ucg.setFont(ucg_font_helvB08_tr);
  ucg.print("Moisture:");
  ucg.setFontMode(UCG_FONT_MODE_SOLID);
  ucg.setFont(ucg_font_7x13_mr);

  if (moisture >= moisture_threshold)
    ucg.setColor(0, 0, 0, 255);
  else ucg.setColor(0, 0, 255, 0);
  ucg.setColor(1, 0, 0, 0);  // use black as background for SOLID mode
  ucg.setPrintPos(60,y);
  ucg.print(moisture);
    
  if (currentTime - moistureSensor_lastPoll >= moistureSensor_pollRate) {
    digitalWrite(moistureSensor_powerPin, HIGH);
    delay(10);
    moisture = analogRead(moistureSensor_readPin);
    digitalWrite(moistureSensor_powerPin, LOW);
    
    Serial.print("Moisture: ");
    Serial.println(moisture);

    
    if (moisture >= moisture_threshold && currentTime - pump_lastStart >= pump_downTime) {
      digitalWrite(pump_pin, HIGH);
      pump_lastStart = currentTime;
    }

    moistureSensor_lastPoll = currentTime;
  } 
  if (currentTime - pump_lastStart >= pump_activeDuration) {
    digitalWrite(pump_pin, LOW);
  }
}