#include <DHT.h>
#include "Ucglib.h"

#include "CallbackTimer.h"
#include "config.h"
#include "display.h"

struct PS {
  float temp;
  int moist;
  int light;
  bool lights_on = -1;
  bool pump_on;
  bool pump_cooldown;
} plantStatus;

struct IS {
  bool isConnected;
  bool isPortalOpen;
  String ip = "";
} wifiStatus;

Timer sensorTimer;

DHT dht11(config::temperatureSensor_readPin, DHT11);

struct Pump {
  inline static bool isOnCooldown;
  inline static bool on;
  inline static Timer timer;

  void turnOn() {
    digitalWrite(config::pump_out, HIGH);
    analogWrite(config::pump_pwm_pin, 255);
    Serial.println(F("Pump turned ON"));
    on = true;
    timer.setTimeout(config::pump_activeDuration, [] {
      digitalWrite(config::pump_out, LOW);
      analogWrite(config::pump_pwm_pin, 0);
      Serial.println(F("Pump turned OFF"));
      plantStatus.pump_on = false;
      Pump::on = false;
      Pump::isOnCooldown = true;
      Serial.println("Cooldown started");
    });

    plantStatus.pump_on = on;
    plantStatus.pump_cooldown = isOnCooldown;
  }
} pump;

void pollSensors() {
  // resistance decreases when wet, so subtract it from max (1023)
  plantStatus.moist = ((float)(1023.0 - analogRead(config::moistureSensor_readPin)) / 1023.0) * 100.0;
  // resistance increases with light
  plantStatus.light = ((analogRead(config::light_readPin)) / 1023.0) * 100.0;
  // use DHT to read temperature in Celsius degrees
  plantStatus.temp = dht11.readTemperature();

  Serial.print(F("Moisture: "));
  Serial.print(plantStatus.moist);
  Serial.print(F("% | Light: "));
  Serial.print(plantStatus.light);
  Serial.print(F("% | Temperature: "));
  Serial.print(plantStatus.temp);
  Serial.println("°C");
  Serial.print(F("Connected: "));
  Serial.print(wifiStatus.isConnected);
  Serial.print(F(" | Portal open: "));
  Serial.print(wifiStatus.isPortalOpen);
  Serial.print(F(" | IP: "));
  Serial.println(wifiStatus.ip);
}

void initDisplay() {
  // initialize display
  ucg.begin(UCG_FONT_MODE_SOLID);
  ucg.clearScreen();
  ucg.setRotate180();
  ucg.setFont(ucg_font_ncenR14_hr);

  // make screen black
  ucg.setColor(0, 0, 0);
  ucg.drawBox(0, 0, ucg.getWidth() + 4, ucg.getHeight());
}

void updateDisplay() {
  const uint8_t text_height = 14;  // how many pixels to go down when printing anouther text row
  uint8_t pos[2] = { 6, text_height + 32 };
  uint8_t color[3];

  color[0] = 0;
  color[1] = plantStatus.moist <= config::moisture_threshold ? 0 : 255;  // green if moist
  color[2] = plantStatus.moist > config::moisture_threshold ? 0 : 255;   // red if dry
  displaySensorValue(pos, color, "Moisture", plantStatus.moist, "%");

  pos[1] += text_height;
  color[0] = 0;
  color[1] = plantStatus.light < config::light_threshold ? 0 : 255;   // green if well lit
  color[2] = plantStatus.light >= config::light_threshold ? 0 : 255;  // red if dark
  displaySensorValue(pos, color, "Light level", plantStatus.light, "%", true);

  pos[1] += text_height;
  color[0] = 178;
  color[1] = 255;
  color[2] = 108;
  displaySensorValue(pos, color, "Temperature", plantStatus.temp, "^C", false);

  displayFace(3, plantStatus.moist > config::moisture_threshold);

  displayNoInternet(2, !wifiStatus.isConnected);

  pos[0] = 6;
  pos[1] = ucg.getHeight() - 20;
  color[0] = 255;
  color[1] = 255;
  color[2] = 255;
  ucg.setFont(ucg_font_helvB08_tr);

  if (wifiStatus.isConnected || wifiStatus.isPortalOpen) {
    ucg.setColor(255, 255, 255);
    ucg.setPrintPos(pos[0], pos[1]);
    ucg.setFont(ucg_font_helvB08_tr);
    ucg.print(wifiStatus.isConnected ? "IP Address:" : "BotaniTalk-AP:");
    ucg.setFont(ucg_font_7x13_mr);

    ucg.setColor(0, color[0], color[1], color[2]);
    ucg.setPrintPos(6, pos[1] + text_height);
    ucg.print(wifiStatus.isConnected ? wifiStatus.ip.c_str() : "192.168.1.4");
  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  dht11.begin();
  initDisplay();

  DDRD = (1 << config::light_out) | (1 << config::pump_out) | (1 << config::pump_pwm_pin);

  pinMode(config::temperatureSensor_readPin, INPUT);
  pinMode(config::moistureSensor_readPin, INPUT);
  pinMode(config::light_readPin, INPUT);

  sensorTimer.setInterval(config::sensor_pollRate, pollSensors);
}

char tempChars[128];
char receivedChars[128];
bool newData;
void syncData() {
  char ch;
  plantStatus.pump_on = pump.on;
  plantStatus.pump_cooldown = pump.isOnCooldown;
  while (Serial1.available() > 0) {
    ch = Serial1.read();
    if (ch == 'I') {
      String response = "<";
      response += plantStatus.moist;
      response += ",";
      response += plantStatus.light;
      response += ",";
      response += plantStatus.pump_cooldown;
      response += ",";
      if (isnanf(plantStatus.temp))
        response += "0";
      else
        response += plantStatus.temp;
      response += ",";
      response += plantStatus.lights_on;
      response += ",";
      response += plantStatus.pump_on;
      response += ">";
      Serial1.println(response);
    } else if (ch = 'W') {
      static bool recvInProgress = false;
      static uint8_t ndx = 0;
      char rc;

      Serial1.print('W');
      while (Serial1.available() > 0 && newData == false) {
        rc = Serial1.read();

        if (recvInProgress) {
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
        } else if (rc == '<') {
          recvInProgress = true;
        }
      }

      if (newData) {
        strcpy(tempChars, receivedChars);
        wifiStatus.isConnected = atoi(strtok(tempChars, ","));
        wifiStatus.isPortalOpen = atoi(strtok(NULL, ","));
        wifiStatus.ip = strtok(NULL, ",");
        newData = false;
      }
    }
  }
}

void loop() {
  // call tick on all timers so they update at each loop iteration
  sensorTimer.tick();
  pump.timer.tick();

  if (pump.isOnCooldown && !pump.timer.hasStarted()) {
    pump.timer.setTimeout(config::pump_coolDown, [] {
      pump.isOnCooldown = false;
      pump.on = false;
      Serial.println("Cooldown ended");
      plantStatus.pump_cooldown = false;
      plantStatus.pump_on = false;
    });
  }

  if (!pump.on && !pump.isOnCooldown && plantStatus.moist < config::moisture_threshold) {
    pump.turnOn();
  }

  // turn on the led strip if dark, otherwise turn it off
  if (plantStatus.light != -1 && plantStatus.light < config::light_threshold) {
    digitalWrite(config::light_out, HIGH);
    plantStatus.lights_on = true;
  } else {
    digitalWrite(config::light_out, LOW);
    plantStatus.lights_on = false;
  }

  updateDisplay();
  syncData();
}
