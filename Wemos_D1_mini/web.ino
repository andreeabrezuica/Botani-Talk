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

void handleRequest(const String& request, WiFiClient& client) {
    if (request.indexOf("GET") == 0) {
    if (request.indexOf("/status") != -1) {
      sendInfo(client);
    } else if (request.indexOf("/style.css") != -1) {
      sendCSSFile(client);
    } else if (request.indexOf("/app.js") != -1) {
      sendJSFile(client);
    } else if (request.indexOf("/sensors") != -1) {
      sendHistory(client);
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

void sendJSFile(WiFiClient& client) {
  const String fileContent = readFile("/app.js");
  if (!fileContent.isEmpty()) {
    sendResponseHeader(client, "text/javascript; charset=utf-8", fileContent.length());
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
