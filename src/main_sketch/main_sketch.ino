#include <DHT.h>

#include "CallbackTimer.h"
#include "config.h"
#include "display.h"

uint8_t moisture = 0;
uint8_t light = 0;
float temp = 0.0;

Timer sensorTimer;

Ucglib_ST7735_18x128x160_SWSPI display(/*scl=*/ 8, /*data=*/ 9, /*cd=*/ 10, /*cs=*/12, /*reset=*/ 11);
SensorDisplay sensorDisplay(display, 5, 64, 128, 20);
PumpStatusDisplay pumpDisplay(display, 120, 100, 8, 8);
InternetStatusDisplay internetDisplay(display, 5, 100, 128, 20);
DHT dht11(config::temperatureSensor_readPin, DHT11);

struct Pump {
  bool isOnCooldown;
  bool on;
  Timer timer;

  void turnOn() {
    PORTD = (1 << config::pump_out) | PORTD;
    analogWrite(config::pump_pwm_pin, 255);
    instance().timer.setTimeout(config::pump_activeDuration, [] {
      PORTD = ~(1 << config::pump_out) & PORTD;
      analogWrite(config::pump_pwm_pin, 0);
      instance().on = true;
      Serial.println(F("Pump turned ON"));
    });
    isOnCooldown = true;
    instance().timer.setTimeout(config::pump_coolDown, [] {
      instance().isOnCooldown = false;
      instance().on = false;
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
  moisture = ((float) (1023.0 - analogRead(config::moistureSensor_readPin)) / 1023.0) * 100.0;
  // resistance increases with light
  light = ((analogRead(config::light_readPin)) / 1023.0) * 100.0;
  // use DHT to read temperature in Celsius degrees
  temp = dht11.readTemperature();

  Serial.print(F("Moisture: "));
  Serial.print(moisture);
  Serial.print(F("% | Light: "));
  Serial.print(light);
  Serial.print(F("% | Temperature: "));
  Serial.print(temp);
  Serial.println("°C");
}

void updateDisplay() {
  sensorDisplay.setSensorData(moisture, light, temp);
  pumpDisplay.setPumpStatus(pump.on);
  internetDisplay.setConnectionStatus(false, "127.0.0.1:5000");

  sensorDisplay.update();
  pumpDisplay.update();
  internetDisplay.update();
}

void setup() {
  Serial.begin(9600);

  DDRD = (1 << config::light_out) | (1 << config::pump_out) | (1 << config::pump_pwm_pin);
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

  updateDisplay();
}
