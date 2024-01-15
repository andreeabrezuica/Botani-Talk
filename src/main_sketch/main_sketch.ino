#include <SPI.h>
#include <Ethernet.h>
#include "Ucglib.h"
#include "config.h"

float moisture = 0;
int light = -1;
bool isWarm = false;

EthernetClient client;

Ucglib_ST7735_18x128x160_SWSPI ucg(/*scl=*/ 8, /*data=*/ 9, /*cd=*/ 10, /*cs=*/12, /*reset=*/ 11);
// Ucglib_ILI9163_18x128x128_SWSPI ucg(/*sclk=*/ 3, /*data=*/ 4, /*cd=*/ 5, /*cs=*/ 7, /*reset=*/ 6);

void displaySensorValue(uint8_t pos[2], uint8_t color[3], char* title, float value, char* unit = 0, bool toInt = false) {
  ucg.setColor(255, 255, 255);
  ucg.setFontMode(0);
  ucg.setPrintPos(pos[0], pos[1]);
  ucg.setFont(ucg_font_helvB08_tr);
  ucg.print(title);
  ucg.setFontMode(0);
  ucg.setFont(ucg_font_7x13_mr);

  ucg.setColor(0, color[0], color[1], color[2]);
  ucg.setColor(1, 0, 0, 0);  // use black as background for SOLID mode
  ucg.setPrintPos(ucg.getWidth() - 52, pos[1]);
  if (toInt) {
    ucg.print((int) value);
  }
  else {
    ucg.print(value);
  }
  if (unit) {
    ucg.print(unit);
  }
}

void displaySensorValue(uint8_t pos[2], uint8_t color[3], char* title, char* status) {
  ucg.setColor(255, 255, 255);
  ucg.setFontMode(0);
  ucg.setPrintPos(pos[0], pos[1]);
  ucg.setFont(ucg_font_helvB08_tr);
  ucg.print(title);
  ucg.setFontMode(0);
  ucg.setFont(ucg_font_7x13_mr);
  
  ucg.setColor(0, color[0], color[1], color[2]);
  ucg.setColor(1, 0, 0, 0);  // use black as background for SOLID mode
  ucg.setPrintPos(ucg.getWidth() - 52, pos[1]);
  ucg.print(status);
}

// void displaySensorValue(uint8_t pos[2], uint8_t color[3], char* title, int value, bool isPercent = false) {
//   displaySensorValue(pos, color, title, (int) value, isPercent);
// }

void sendEmail() {
  // connect to IFTTT server on port 80:
  if (client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    String queryString1 = "?value1=" + String(moisture);
    String queryString2 = "&value2=" + String(light);
    // send HTTP header
    client.println("GET " + PATH_NAME + queryString1 + queryString2 + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); // end HTTP header

    while (client.connected()) {
      if (client.available()) {
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }

    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  } 
  else {// if not connected:
    Serial.println("connection failed");
  }
}

void setup() {
  Serial.begin(9600);
  DDRD = B11000000;
  PORTD = B00100000;

  // pinMode(moistureSensor_powerPin, OUTPUT);
   pinMode(moistureSensor_readPin, INPUT);
   pinMode(light_readPin, INPUT);
  // pinMode(pump_pin, OUTPUT);
  // pinMode(light_out, OUTPUT);
  // pinMode(8, OUTPUT);

  ucg.begin(0);
  ucg.clearScreen();
  ucg.setRotate180();

  ucg.setColor(0,0,0);
  ucg.drawBox(0,0, ucg.getWidth(), ucg.getHeight());

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to obtaining an IP address using DHCP");
  }
}

void loop() {

  const unsigned long currentTime = millis();

  int text_height = 14;
  uint8_t pos[2] = {6, 32 + text_height};
  uint8_t color[3] = {0, 0, 255};  

  color[0] = 0;
  color[1] = moisture <= moisture_threshold ? 0 : 255;
  color[2] = moisture > moisture_threshold ? 0 : 255;
  displaySensorValue(pos, color, "Moisture", moisture, "%");
  
  pos[1] += text_height;
  color[0] = 0;
  color[1] = light < light_threshold ? 0 : 255;
  color[2] = light >= light_threshold ? 0 : 255;
  displaySensorValue(pos, color, "Light level", light, "%", true);

  pos[1] += text_height;
  color[0] = !isWarm ? 255 : 0;
  color[1] = 0;
  color[2] = isWarm ? 255 : 0;
  displaySensorValue(pos, color, "Temp", isWarm ? "Warm" : "Cold");

  if (currentTime - moistureSensor_lastPoll >= moistureSensor_pollRate) {
    // digitalWrite(moistureSensor_powerPin, HIGH);
    // delay(100);
    // int moistureR = (PIND &(1<<moistureSensor_readPin)) >> moistureSensor_readPin;
    moisture = ((1023.0 - analogRead(moistureSensor_readPin)) / 1023.0) * 100.0;
    // int lightR = (PIND & (1<<light_readPin))>>light_readPin;
    light = ((1023.0 - analogRead(light_readPin)) / 1023.0) * 100.0;
    // light = analogRead(light_readPin);
    // digitalWrite(moistureSensor_powerPin, LOW);

    isWarm = (PIND & (1 << 5)) >> 5;
    
    Serial.print("Moisture: ");
    Serial.print(moisture);
    Serial.print("%");

    // digitalWrite(light_powerPin, HIGH);
    // delay(1000);
    // digitalWrite(light_powerPin, LOW);

    Serial.print(" | Light: ");
    Serial.println(light);
    Serial.print("%");

    Serial.print(" | Temp: ");
    Serial.println(isWarm ? "Warm" : "Cold");

    moistureSensor_lastPoll = currentTime;
  }

  if (moisture <= moisture_threshold && currentTime - pump_lastStart >= pump_downTime) {
    // digitalWrite(pump_pin, HIGH);
    PORTD = (1<<pump_pin)|PORTD;
    pump_lastStart = currentTime;
    
    Serial.print("Pump turned off at: ");
    Serial.println(currentTime);
  }

  if (light != -1 && light < light_threshold) {
    // digitalWrite(light_out, HIGH); 
    PORTD = (1<<light_out) | PORTD;
  } else {
    // digitalWrite(light_out, LOW);
    PORTD = ~(1<<light_out) & PORTD;
  }
  // delay(1000);

  int isPumpOn = (PIND & (1 << pump_pin)) >> pump_pin;
  if (isPumpOn && currentTime - pump_lastStart >= pump_activeDuration) {
    // digitalWrite(pump_pin, LOW);
    PORTD = ~(1<<pump_pin) & PORTD;
    Serial.print("Pump turned off at: ");
    Serial.println(currentTime);
  }
  if (moisture <= moisture_threshold && currentTime - timeSinceLastMail >= maxTimeWithoutWater) {
    Serial.print("Trying to send e-mail!");
    sendEmail();
    timeSinceLastMail = currentTime;
  }
}
