#include <SPI.h>
#include <Ethernet.h>

#include "config.h"

float moisture = 0;
int light = -1;
bool isWarm = false;

EthernetClient client;

bool isSent = false;

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

//ucglib_ST7735_18x128x160_SWSPI //ucg(/*scl=*/ 12, /*data=*/ 11, /*cd=*/ 9, /*cs=*/ 10, /*reset=*/ 8);

void setup() {
  Serial.begin(9600);
  DDRD = B11000000;
  PORTD = B00100000;
  // pinMode(moistureSensor_powerPin, OUTPUT);
  pinMode(moistureSensor_readPin, INPUT);

  pinMode(pump_pin, OUTPUT);

  //ucg.begin(//ucg_FONT_MODE_TRANSPARENT);
  //ucg.clearScreen();
  // //ucg.setRotate180();

  //ucg.setColor(0,0,0);
  //ucg.drawBox(0,0, //ucg.getWidth(), //ucg.getHeight());

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to obtaining an IP address using DHCP");
    while (true);
  }

}

void loop() {
  const unsigned long currentTime = millis();

  //ucg.setFont(//ucg_font_ncenR12_tr);
  //ucg.setColor(255, 255, 255);

  int y = 0;
  int h = 14;

  //y += h;
  //ucg.setFontMode(//ucg_FONT_MODE_TRANSPARENT);
  //ucg.setPrintPos(4,y);
  //ucg.setFont(//ucg_font_helvB08_tr);
  //ucg.print("Moisture:");
  //ucg.setFontMode(//ucg_FONT_MODE_SOLID);
  //ucg.setFont(//ucg_font_7x13_mr);

  //if (moisture >= moisture_threshold)
    //ucg.setColor(0, 0, 0, 255);
  //else //ucg.setColor(0, 0, 255, 0);
  //ucg.setColor(1, 0, 0, 0);  // use black as background for SOLID mode
  //ucg.setPrintPos(60,y);
  //ucg.print(moisture);
    
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