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

void getInfoFromMega() {
  char arr[] = "-info";
  transfer.sendDatum(arr);

  while (receiveSerial(plantStatus)) {
    Serial.println("Receiving data...");
  }
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
