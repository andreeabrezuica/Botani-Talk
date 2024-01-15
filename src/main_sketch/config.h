#ifndef CONFIG_H
#define CONFIG_H

const uint8_t pump_pin = 7;

//const uint8_t moistureSensor_powerPin = 13;
const uint8_t moistureSensor_readPin = A4;

const int moistureSensor_pollRate = 2000; /* check the sensor every 5s */
unsigned long moistureSensor_lastPoll = 0;
const short moisture_threshold = 60; // percent
unsigned long pump_lastStart = 0;
const int pump_downTime = 20000; /* wait 20 seconds even if still dry */
const int pump_activeDuration = 1000;
const int maxTimeWithoutWater = 30000; // 30s
unsigned long timeSinceLastMail = 0;

const uint8_t light_readPin = A5;
const uint8_t light_out = 6;
const int light_threshold = 50;
//const uint8_t light_powerPin = 2;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

int    HTTP_PORT = 80;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME = "/trigger/send-email/with/key/heQXcCZGAUOnHDjYqOk7d2zmYVCYitI7pZkzSlRJC-B";

#endif // end config.h