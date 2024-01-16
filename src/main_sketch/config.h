#ifndef CONFIG_H
#define CONFIG_H

const uint8_t pump_out = 7; // gate signal to pump transistor 
const uint8_t light_out = 6; // power the DIY LED strip

const uint8_t light_readPin = A3; // light sensor (photoresistor) input pin 
const uint8_t moistureSensor_readPin = A4; // moisture sensor (DIY) input pin
const uint8_t temperatureSensor_readPin = 5; // digital temperature sensor input pin

const int sensor_pollRate = 2000; // interval (in ms) at which the sensor values are updated

const short moisture_threshold = 60; // under this limit (in percents [%]), the the pump is turned on
const int light_threshold = 15; // under this limit (in percents [%]), the led strip is turned on
const int minEmailInterval = 30000; // time (in ms) after which we try to send an e-mail if there is no water

unsigned long timeSinceLastMail = 0;
unsigned long sensor_lastPoll = 0; 
unsigned long pump_lastStart = 0;

const int pump_coolDown = 20000; // cooldown (ms) until the pump can start again (to give the plant time to absorb the water)
const int pump_activeDuration = 1000; // amount of time (in ms) for which the pump is on before stopping

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC address from which to send the mail HTTPrequest

int HTTP_PORT = 80; // HTTP port
String HTTP_METHOD = "GET";
char HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME = "/trigger/send-email/with/key/cedGxRcG2gRP0xD0qs130HRFP3rxYoQ83BHdCqjzl14";

#endif // end config.h