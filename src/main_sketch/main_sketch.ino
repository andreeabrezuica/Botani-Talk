#include <SPI.h>
#include <Ethernet.h>
// #include "ucglib.h"

const uint8_t pump_pin = A2;

const uint8_t moistureSensor_powerPin = A1;
const uint8_t moistureSensor_readPin = A3;

const int moistureSensor_pollRate = 5000; /* check the sensor every 5s */
unsigned long moistureSensor_lastPoll = 0;
const short moisture_threshold = 200; // percent
unsigned int pump_lastStart = 0;
const int pump_downTime = 20000; /* wait 20 seconds even if still dry */
const int pump_activeDuration = 3000;
const int maxTimeWithoutWater = 60000; // 6 min
unsigned long timeSinceLastMail = 0;

int moisture = 0;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient client;

int    HTTP_PORT   = 80;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME   = "/trigger/send-email/with/key/heQXcCZGAUOnHDjYqOk7d2zmYVCYitI7pZkzSlRJC-B"; // change your Webhooks key

bool isSent = false;

void sendEmail(int moisture) {
  // connect to IFTTT server on port 80:
  if (client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    String queryString = "?value1=" + String(moisture);
    // send HTTP header
    client.println("GET " + PATH_NAME + queryString + " HTTP/1.1");
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

  pinMode(moistureSensor_powerPin, OUTPUT);
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
  unsigned long currentTime = millis();

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
    digitalWrite(moistureSensor_powerPin, HIGH);
    delay(100);
    moisture = analogRead(moistureSensor_readPin);
    // digitalWrite(moistureSensor_powerPin, LOW);
    
    Serial.print("Moisture: ");
    Serial.println(moisture);

    if (moisture >= moisture_threshold && currentTime - pump_lastStart >= pump_downTime + pump_activeDuration) {
      digitalWrite(pump_pin, HIGH);
      pump_lastStart = currentTime;

      if (moisture >= moisture_threshold && currentTime - timeSinceLastMail >= maxTimeWithoutWater) {
        if (isSent == false) { // to make sure that Arduino does not send duplicated emails
          sendEmail(moisture);
          isSent = true;
          timeSinceLastMail = millis();
        }
      }
    }
      moistureSensor_lastPoll = currentTime;
    }

    if (currentTime - pump_lastStart >= pump_activeDuration) {
      digitalWrite(pump_pin, LOW);
    }
}
