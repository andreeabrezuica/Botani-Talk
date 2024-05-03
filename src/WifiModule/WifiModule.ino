#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <SerialTransfer.h>
#include <FS.h>
#include <LittleFS.h>

// select which pin will trigger the configuration portal when set to LOW
#define TRIGGER_PIN D2

int timeout = 120;  // timeout for configuration portal
SerialTransfer transfer;
WiFiServer server(80);

void setup() {
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  Serial1.begin(9600);
  transfer.begin(Serial1);

  WiFiManager wm;
  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name
  bool res = wm.autoConnect("BotaniTalk-AP");

  if (res) {
    startServer();

  } else {
    Serial.println("Failed to connect");
  }
}

void loop() {
  if (digitalRead(TRIGGER_PIN) == LOW) {
    startPortal();
  }

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("Hello world");
  client.println(ReadFileToString("/index.html"));
}

String ReadFileToString(const char* filename) {
  if(!LittleFS.begin()){
    return String("An Error has occurred while mounting LittleFS");
  }

  File file = LittleFS.open(filename, "r");
  if(!file){
    return String("Failed to open file for reading");
  }
  String data = file.readString();
  file.close();

  Serial.println("File:");
  Serial.println(data);
  return data;
}

void startPortal() {
  WiFiManager wm;
  // set configportal timeout
  wm.setConfigPortalTimeout(timeout);

  if (!wm.startConfigPortal("BotaniTalk-AP")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("CONNECTED");
  startServer();
}

bool startServer() {
  if (WiFi.status() != WL_CONNECTED)
    return false;

  server.begin();
  Serial.println("Server started");
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  return true;
}