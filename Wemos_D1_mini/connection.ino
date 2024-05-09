char tempChars[128];
char receivedChars[128];
boolean newData;

void requestInfoFromMega() {
  s.print("I");
  delay(100);
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char rc;

  while (s.available() > 0 && newData == false) {
    rc = s.read();

    if (recvInProgress == true) {
      if (rc != '>') {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= 128) {
          ndx = 127;
        }
      } else {
        receivedChars[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == '<') {
      recvInProgress = true;
    }
  }


  if (newData) {
    strcpy(tempChars, receivedChars);
    plantStatus.moist = atoi(strtok(tempChars, ","));
    plantStatus.light = atoi(strtok(NULL, ","));
    plantStatus.temp = atof(strtok(NULL, ","));
    plantStatus.lights_on = atoi(strtok(NULL, ","));
    plantStatus.pump_on = atoi(strtok(NULL, ","));
    plantStatus.pump_cooldown = atoi(strtok(NULL, ","));
    newData = false;
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
