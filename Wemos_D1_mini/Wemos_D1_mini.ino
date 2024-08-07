#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <SoftwareSerial.h>
#include <LittleFS.h>
#include <time.h>

#include <CSV_Parser.h>

#include "CallbackTimer.h"

// select which pin will trigger the configuration portal when set to LOW
#define TRIGGER_PIN D2
#define FILE_READ_FAILED "Failed to open file for reading"

int timeout = 120;  // timeout for configuration portal
SoftwareSerial s(D6, D5, false);
WiFiServer server(80);

Timer serialTimer;
Timer saveTimer;

unsigned long unixTime;

struct PS {
  float temp;
  int moist;
  int light;
  bool lights_on;
  bool pump_on;
  bool pump_cooldown;
} plantStatus;

struct IS {
  bool isConnected;
  bool isPortalOpen = false;
  bool gotTimeFromNTP;
  IPAddress ip;
} wifiStatus;

void setup() {
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  s.begin(9600);

  WiFiManager wm;
  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name
  if (wm.autoConnect("BotaniTalk-AP")) {
    wifiStatus.isConnected = true;
    startServer();
    getUnixTime();
  } else {
    Serial.println("Failed to connect");
  }

  serialTimer.setInterval(1000, [] {
    requestInfoFromMega();
    sendWifiInfo();
  });
  saveTimer.setInterval(60000, updateCSV);
}

void loop() {
  serialTimer.tick();
  saveTimer.tick();
  if (digitalRead(TRIGGER_PIN) == LOW) {
    startPortal();
  }

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  String request = client.readStringUntil('\r');
  Serial.println("------------- REQUEST -------------");
  Serial.println(request);
  Serial.println("___________________________________");
  Serial.println();
  client.flush();
  handleRequest(request, client);
}
