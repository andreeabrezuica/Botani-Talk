#ifndef CONFIG_H
#define CONFIG_H

const uint8_t pump_out PROGMEM = 7; // gate signal to pump transistor 
const uint8_t light_out PROGMEM = 6; // power the DIY LED strip

const uint8_t light_readPin PROGMEM = A3; // light sensor (photoresistor) input pin 
const uint8_t moistureSensor_readPin PROGMEM = A4; // moisture sensor (DIY) input pin
const uint8_t temperatureSensor_readPin PROGMEM = 5; // digital temperature sensor input pin

const short sensor_pollRate = 2000; // interval (in ms) at which the sensor values are updated

const uint8_t moisture_threshold PROGMEM = 60; // under this limit (in percents [%]), the the pump is turned on
const uint8_t light_threshold PROGMEM = 15; // under this limit (in percents [%]), the led strip is turned on
const int minEmailInterval = 30000; // time (in ms) after which we try to send an e-mail if there is no water

unsigned long timeSinceLastMail = 0;
unsigned long sensor_lastPoll = 0; 
unsigned long pump_lastStart = 0;

const short pump_coolDown = 20000; // cooldown (ms) until the pump can start again (to give the plant time to absorb the water)
const short pump_activeDuration = 1000; // amount of time (in ms) for which the pump is on before stopping

uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC address from which to send the mail HTTP request

const char HOST_NAME[] = "maker.ifttt.com";
const char PATH_NAME[] = "/trigger/send-email/with/key/heQXcCZGAUOnHDjYqOk7d2zmYVCYitI7pZkzSlRJC-B";

#endif // end config.h