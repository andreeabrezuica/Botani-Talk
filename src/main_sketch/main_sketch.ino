#include <timer.h>
#include "config.h"
#include <Ucglib.h>

float moisture = 0;
uint8_t light = 0;

Timer sensorTimer;

struct Pump {
  bool isOnCooldown;
  Timer timer;

  void turnOn() {
    PORTD = (1 << config::pump_out) | PORTD;
    instance().timer.setTimeout(config::pump_activeDuration, [] {
      PORTD = ~(1 << config::pump_out) & PORTD;
      Serial.println(F("Pump turned ON"));
    });
    isOnCooldown = true;
    instance().timer.setTimeout(config::pump_coolDown, [] {
      instance().isOnCooldown = false;
      Serial.println(F("Pump turned OFF"));
    });
  }

private:
  // Helper function to get the instance of the class, as we cannot capture anything in timer's lambda
  static Pump& instance() {
    static Pump pumpInstance;
    return pumpInstance;
  }
} pump;

void pollSensors() {
  // resistance decreases when wet, so subtract it from max (1023)
  moisture = ((1023.0 - analogRead(config::moistureSensor_readPin)) / 1023.0) * 100.0;
  // resistance increases with light
  light = ((analogRead(config::light_readPin)) / 1023.0) * 100.0;

  Serial.print(F("Moisture: "));
  Serial.print(moisture);
  Serial.print(F("%"));

  Serial.print(F(" | Light: "));
  Serial.print(light);
  Serial.println(F("%"));
}

void setup() {
  Serial.begin(9600);

  // configure pins
  DDRD = (1 << config::light_out) | (1 << config::pump_out);
  PORTD = 1 << config::temperatureSensor_readPin;

  pinMode(config::moistureSensor_readPin, INPUT);
  pinMode(config::light_readPin, INPUT);

  sensorTimer.setInterval(config::sensor_pollRate, pollSensors);
}

void loop() {
  // call tick on all timers so they update at each loop iteration
  sensorTimer.tick();
  pump.timer.tick();

  if (!pump.isOnCooldown && moisture < config::moisture_threshold) {
    pump.turnOn();
  }

  // turn on the led strip if dark, otherwise turn it off
  if (light != -1 && light < config::light_threshold) {
    PORTD = (1 << config::light_out) | PORTD;
  } else {
    PORTD = ~(1 << config::light_out) & PORTD;
  }
}
