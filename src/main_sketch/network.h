#ifndef NET_H
#define NET_H

#include "config.h"

EthernetClient client;

bool sendEmail(float moisture, int light, bool isWarm) {
  // connect to IFTTT server on port 80:
  if (client.connect(HOST_NAME, 80)) {
    // if connected:
    Serial.println(F("Connected to server"));
    // make a HTTP request:
    client.print("GET ");
    client.print(PATH_NAME);

    client.print(F("?value1="));
    client.print(moisture);
    client.print(F("&value2="));
    client.print(light);
    client.print(F("&value3="));
    client.print(isWarm ? "Warm" : "Cold");

    client.println(" HTTP/1.1");

    client.print("Host: ");
    client.println(HOST_NAME);

    client.println("Connection: close");
    client.println();

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
    Serial.println(F("disconnected"));
    return true;
  } 
  else {// if not connected:
    Serial.println(F("connection failed"));
    return false;
  }
}

#endif // end network.h