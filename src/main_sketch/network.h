#ifndef NET_H
#define NET_H

#include "config.h"

EthernetClient client;

bool sendEmail(float moisture, int light, bool isWarm) {
  // connect to IFTTT server on port 80:
  if (client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    String queryString1 = "?value1=" + String(moisture);
    String queryString2 = "&value2=" + String(light);
    String queryString3 = "&value3=" + isWarm ? "Warm" : "Cold";
    // send HTTP header
    client.println("GET " + PATH_NAME + queryString1 + queryString2 + queryString3 + " HTTP/1.1");
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

    // the server disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
    return true;
  } 
  else {// if not connected:
    Serial.println("connection failed");
    return false;
  }
}

#endif // end network.h