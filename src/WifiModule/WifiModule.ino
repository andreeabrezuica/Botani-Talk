#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <SerialTransfer.h>
#include <LittleFS.h>

// select which pin will trigger the configuration portal when set to LOW
#define TRIGGER_PIN D2

#define SW_SERIAL 0
#define HW_SERIAL 1

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
    } else {
      sendHomePage(client);
    }
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
  // TEMPORARY
  const String response = "{\"moist\": 87, \"light\": 60, \"temp\": 25.4}";
  sendResponseHeader(client, "text/json; charset=utf-8", response.length());
  client.println(response);
}

void sendHomePage(WiFiClient& client) {
  const String fileContent = ReadFileToString("/index.html");
  if (!fileContent.isEmpty()) {
    sendResponseHeader(client, "text/html; charset=utf-8", fileContent.length());
    client.println(fileContent);
  } else {
    send404(client);
  }
}

void sendCSSFile(WiFiClient& client) {
  const String fileContent = ReadFileToString("/style.css");
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

String ReadFileToString(const char* filename) {
  if (!LittleFS.begin()) {
    return String("An Error has occurred while mounting LittleFS");
  }

  File file = LittleFS.open(filename, "r");
  if (!file) {
    return String("Failed to open file for reading");
  }
  String data = file.readString();
  file.close();
  return data;
}

void transmitSerial(const char* buff, bool softwareSerial) {
  if (softwareSerial) {
    transfer.txObj(buff, sizeof(buff));
    transfer.sendData(sizeof(buff));
  } else {
    Serial.write(buff);
  }
}

void receiveSerial(char* buff, bool softwareSerial) {
  if (!softwareSerial) {
    String s = Serial.readString();
    strcpy_P(buff, s.c_str());
    Serial.print(buff);
    return;
  }
  if (transfer.available()) {
    transfer.rxObj(buff, sizeof(buff));
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
