#include <DHT.h>
#include <SerialTransfer.h>

#include "CallbackTimer.h"
#include "config.h"
#include "display.h"

struct PS {
  float temp;
  int moist;
  int light;
  bool lights_on;
  bool pump_on;
  bool pump_cooldown;
} plantStatus;

Timer sensorTimer;
SerialTransfer transfer;

Ucglib_ST7735_18x128x160_SWSPI display(/*scl=*/8, /*data=*/9, /*cd=*/10, /*cs=*/12, /*reset=*/11);
SensorDisplay sensorDisplay(display, 5, 64, 128, 20);
PumpStatusDisplay pumpDisplay(display, 120, 100, 8, 8);
InternetStatusDisplay internetDisplay(display, 5, 100, 128, 20);
DHT dht11(config::temperatureSensor_readPin, DHT11);

struct Pump {
  inline static bool isOnCooldown;
  inline static bool on;
  inline static Timer timer;

  void turnOn() {
    digitalWrite(config::pump_out, HIGH);
    analogWrite(config::pump_pwm_pin, 255);
    Serial.println(F("Pump turned ON"));
    isOnCooldown = true;
    timer.setTimeout(config::pump_activeDuration, [] {
      digitalWrite(config::pump_out, LOW);
      analogWrite(config::pump_pwm_pin, 0);
      Serial.println(F("Pump turned OFF"));
      Pump::on = false;
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
}

void updateDisplay() {
  sensorDisplay.setSensorData(plantStatus.moist, plantStatus.light, plantStatus.temp);
  pumpDisplay.setPumpStatus(pump.on);
  internetDisplay.setConnectionStatus(false, "127.0.0.1:5000");

  sensorDisplay.update();
  pumpDisplay.update();
  internetDisplay.update();
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  // transfer.begin(Serial1);

  dht11.begin();

  DDRD = (1 << config::light_out) | (1 << config::pump_out) | (1 << config::pump_pwm_pin);
  // PORTD = 1 << config::temperatureSensor_readPin;

  pinMode(config::temperatureSensor_readPin, INPUT);
  pinMode(config::moistureSensor_readPin, INPUT);
  pinMode(config::light_readPin, INPUT);

  sensorTimer.setInterval(config::sensor_pollRate, pollSensors);
}

void loop() {
  // call tick on all timers so they update at each loop iteration
  sensorTimer.tick();
  pump.timer.tick();
  if (pump.isOnCooldown && !pump.timer.hasStarted()) {
    pump.timer.setTimeout(config::pump_coolDown, [] {
      pump.isOnCooldown = false;
      Serial.println("Cooldown ended");
    });
  }

  if (!pump.isOnCooldown && plantStatus.moist < config::moisture_threshold) {
    pump.turnOn();
  }

  // turn on the led strip if dark, otherwise turn it off
  if (plantStatus.light != -1 && plantStatus.light < config::light_threshold) {
    PORTD = (1 << config::light_out) | PORTD;
  } else {
    PORTD = ~(1 << config::light_out) & PORTD;
  }

  updateDisplay();
  char ch;
  while (Serial1.available() > 0) {
    ch = Serial1.read();
    if (ch == 'I') {
      String response = "<";
      response += plantStatus.moist;
      response += ",";
      response += plantStatus.light;
      response += ",";
      if (isnanf(plantStatus.temp))
        response += ",";
      else
        response += plantStatus.temp;
      response += ",";
      response += plantStatus.lights_on;
      response += ",";
      response += plantStatus.pump_on;
      response += ",";
      response += plantStatus.pump_cooldown;
      response += ">";
      Serial1.println(response);
    }
  }
}
