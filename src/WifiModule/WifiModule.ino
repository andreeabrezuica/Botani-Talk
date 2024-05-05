#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <SerialTransfer.h>
#include <LittleFS.h>
#include "time.h"

#include <CSV_Parser.h>

#include "CallbackTimer.h"

// select which pin will trigger the configuration portal when set to LOW
#define TRIGGER_PIN D2
#define FILE_READ_FAILED "Failed to open file for reading"

int timeout = 120;  // timeout for configuration portal
SerialTransfer transfer;
WiFiServer server(80);
Timer serialTimer;
bool connected;
bool gotTimeFromNTP;
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

void updateCSV() {
  Serial.println("Saving CSV file");
  getInfoFromMega();
  
  String values = String("\n") + plantStatus.moist + "," + plantStatus.light + "," + plantStatus.temp + "," + getUnixTime();
  if (readFile("/sensors.csv").isEmpty()) {
    appendFile("/sensors.csv", "moisture,light,temperature,time\n");
  }
  appendFile("/sensors.csv", values.c_str());
}

unsigned long getUnixTime() {
  if (connected && !gotTimeFromNTP) {
    time_t now;
    struct tm timeInfo;
    configTime(10800, 0, "pool.ntp.org");
    if (!getLocalTime(&timeInfo)) {
      Serial.println("Failed to obtain time");
      return false;
    }
    gotTimeFromNTP = true;
    time(&now);
    unixTime = now;
    return now;
  }
  if (gotTimeFromNTP) {
    unixTime += (millis() / 1000);
    return unixTime;
  }
  return 0;
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
  if (request.indexOf("GET") == 0) {
    if (request.indexOf("/status") != -1) {
      sendInfo(client);
    } else if (request.indexOf("/style.css") != -1) {
      sendCSSFile(client);
    } else if (request.indexOf("/sensors") != -1) {
      sendHistory(client);
    } else {
      sendHomePage(client);
    }
  }
}

void getInfoFromMega() {
  char arr[] = "-info";
  transfer.sendDatum(arr);

  while (receiveSerial(plantStatus)) {
    Serial.println("Receiving data...");
  }
}

void sendResponseHeader(WiFiClient& client, const String& contentType, size_t contentLength) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: " + contentType);
  client.print("Content-Length: ");
  client.println(contentLength);
  client.println("Connection: close");
  client.println();
}

void sendInfo(WiFiClient& client) {
  // request information from Arduino Mega
  getInfoFromMega();
  const String response = String("{\"moist\":") + plantStatus.moist + String(",\"light\":") + plantStatus.light + String(",\"temp\":") + plantStatus.temp + String(",\"lights_on\":") + plantStatus.lights_on + String(",\"pump_on\":") + plantStatus.pump_on + String("}");
  sendResponseHeader(client, "text/json; charset=utf-8", response.length());
  client.println(response);
}

void sendHistory(WiFiClient& client) {
  const String response = readFile("/sensors.csv");
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/csv");
  client.println("Content-Length: " + response.length());
  client.println("Access-Control-Allow-Origin: http:://127.0.0.1:80");
  client.println("Connection: close");
  client.println();

  client.println(response);
}

void sendHomePage(WiFiClient& client) {
  const String fileContent = readFile("/index.html");
  if (!fileContent.isEmpty()) {
    sendResponseHeader(client, "text/html; charset=utf-8", fileContent.length());
    client.println(fileContent);
  } else {
    send404(client);
  }
}

void sendCSSFile(WiFiClient& client) {
  const String fileContent = readFile("/style.css");
  if (!fileContent.isEmpty()) {
    sendResponseHeader(client, "text/css; charset=utf-8", fileContent.length());
    client.println(fileContent);
  } else {
    send404(client);
  }
}

void send404(WiFiClient& client) {
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println("404 Not Found");
}

String readFile(const char* filename) {
  if (!LittleFS.begin()) {
    return String("An Error has occurred while mounting LittleFS");
  }

  File file = LittleFS.open(filename, "r");
  if (!file) {
    return String(FILE_READ_FAILED);
  }
  String data = file.readString();
  file.close();
  return data;
}

bool appendFile(const char* filename, const char* message) {
  if (!LittleFS.begin()) {
    return false;
  }
  File file = LittleFS.open(filename, "a");
  if (!file) {
    return false;
  }
  bool success = file.print(message);
  file.close();
  return success;
}

template<typename T>
bool receiveSerial(T& buff) {
  if (transfer.available()) {
    transfer.rxObj(buff);
    return true;
  } else if (transfer.status < 0) {
    Serial.print("ERROR: ");
    switch (transfer.status) {
      case -1:
        Serial.println(F("CRC_ERROR"));
        break;
      case -2:
        Serial.println(F("PAYLOAD_ERROR"));
        break;
      case -3:
        Serial.println(F("STOP_BYTE_ERROR"));
        break;
    }
  }
  return false;
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
  connected = true;
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
