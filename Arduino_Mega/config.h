#ifndef CONFIG_H
#define CONFIG_H

namespace config {
  const uint8_t pump_out PROGMEM = 7; // gate signal to pump transistor 
  const uint8_t pump_pwm_pin PROGMEM = 8; // PWM pin for speed control (needed by the driver, but might just set it to max anyways)
  const uint8_t light_out PROGMEM = 11; // power the DIY LED strip

  const uint8_t light_readPin PROGMEM = A3; // light sensor (photoresistor) input pin 
  const uint8_t moistureSensor_readPin PROGMEM = A4; // moisture sensor (DIY) input pin
  const uint8_t temperatureSensor_readPin PROGMEM = 5; // digital temperature sensor input pin

  const short sensor_pollRate = 2000; // interval (in ms) at which the sensor values are updated

  const uint8_t moisture_threshold PROGMEM = 60; // under this limit (in percents [%]), the pump is turned on
  const uint8_t light_threshold PROGMEM = 15; // under this limit (in percents [%]), the led strip is turned on

  const short pump_coolDown = 20000; // cooldown (ms) until the pump can start again (to give the plant time to absorb the water)
  const short pump_activeDuration = 5000; // amount of time (in ms) for which the pump is on before stopping
};

#endif // end config.h
