#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <SerialTransfer.h>
#include <LittleFS.h>
#include <time.h>

#include <CSV_Parser.h>

#include "CallbackTimer.h"

// select which pin will trigger the configuration portal when set to LOW
#define TRIGGER_PIN D2
#define FILE_READ_FAILED "Failed to open file for reading"

int timeout = 120;  // timeout for configuration portal
SerialTransfer transfer;
WiFiServer server(80);

bool connected;
bool gotTimeFromNTP;

Timer serialTimer;
Timer saveTimer;

unsigned long unixTime;

struct __attribute__((packed)) PS {
  float temp;
  int moist;
  int light;
  bool lights_on;
  bool pump_on;
} plantStatus;

void setup() {
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  Serial1.begin(9600);
  transfer.begin(Serial1);

  WiFiManager wm;
  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name
  if (wm.autoConnect("BotaniTalk-AP")) {
    connected = true;
    startServer();
    getUnixTime();
  } else {
    Serial.println("Failed to connect");
  }

  // TEST
  serialTimer.setInterval(1000, [] {
    plantStatus.light = (plantStatus.light + 13) % 100;
    plantStatus.moist = (plantStatus.moist + 5) % 100;
    plantStatus.temp = (plantStatus.temp + .5);
    if (plantStatus.temp > 30) {
      plantStatus.temp = 10;
    }
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
