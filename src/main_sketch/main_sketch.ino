#include <SPI.h>
#include <Ethernet.h>

#include "Ucglib.h"

#include "config.h"
#include "display.h"
#include "network.h"

// sensor variables
float moisture = 0;
uint8_t light = -1;
bool isWarm = false;

bool internetAvailable;

void setup() {
  Serial.begin(9600);

  // configure pins
  DDRD = (1 << light_out) | (1 << pump_out);
  PORTD = 1 << temperatureSensor_readPin;

  pinMode(moistureSensor_readPin, INPUT);
  pinMode(light_readPin, INPUT);

  // initialize display
  ucg.begin(0);
  ucg.clearScreen();

  // make screen black
  ucg.setColor(0,0,0);
  ucg.drawBox(4,0, ucg.getWidth() + 4, ucg.getHeight());

  ucg.setColor(255, 255, 255);
  ucg.setPrintPos(4, 16);
  ucg.setFont(ucg_font_helvB08_tr);
  ucg.println(F("Connecting to internet..."));

  // try to connect to the Internet (timeout = 10s)
  internetAvailable = Ethernet.begin(mac, 10000);
  ucg.setColor(0,0,0);
  ucg.drawBox(0,0, ucg.getWidth() + 4, ucg.getHeight() - LAN_ICON_H - 4);

  // handle connection failure
  if (!internetAvailable) {
    Serial.println(F("Failed to obtain an IP address using DHCP"));
    displayNoInternet(2, true); // display an icon scaled x2
  }
}

void loop() {
  // get the time in ms since the program started
  const unsigned long currentTime = millis();

  const uint8_t text_height = 14; // how many pixels to go down when printing anouther text row
  uint8_t pos[2] = {6, text_height};
  uint8_t color[3];

  // check the sensor once at every <sensor_pollrate> ms
  if (currentTime - sensor_lastPoll >= sensor_pollRate) {
    // get percent values for moisture and light level
    moisture = ((1023.0 - analogRead(moistureSensor_readPin)) / 1023.0) * 100.0; // resistance decreases when wet, so subtract it from max (1023)
    light = ((analogRead(light_readPin)) / 1023.0) * 100.0; // resistance increases with light

    // read temperature sensor from PIND readPin 
    isWarm = !((PIND & (1 << temperatureSensor_readPin)) >> temperatureSensor_readPin); // HIGH = cold; LOW = warm
    
    Serial.print(F("Moisture: "));
    Serial.print(moisture);
    Serial.print(F("%"));

    Serial.print(F(" | Light: "));
    Serial.print(light);
    Serial.print(F("%"));

    Serial.print(F(" | Temp: "));
    Serial.println(isWarm ? "Warm" : "Cold");

    sensor_lastPoll = currentTime; // keep track of this sensor polling

    color[0] = 0;
    color[1] = moisture <= moisture_threshold ? 0 : 255; // green if moist
    color[2] = moisture > moisture_threshold ? 0 : 255; // red if dry
    displaySensorValue(pos, color, "Moisture", moisture, "%");
    
    pos[1] += text_height;
    color[0] = 0;
    color[1] = light < light_threshold ? 0 : 255; // green if well lit
    color[2] = light >= light_threshold ? 0 : 255; // red if dark
    displaySensorValue(pos, color, "Light level", light, "%", true);

    pos[1] += text_height;
    color[0] = !isWarm ? 255 : 0; // blue if cold
    color[1] = isWarm ? 255 : 0; // green if warm
    color[2] = 0;
    displaySensorValue(pos, color, "Temp", isWarm ? "Warm" : "Cold");

    // happy face if moist, sad face if dry
    displayFace(3, moisture > moisture_threshold);
  }

  // turn on the pump if the soil is dry and the pump is noton cooldown
  if (moisture <= moisture_threshold && currentTime - pump_lastStart >= pump_coolDown) {
    // digitalWrite(pump_pin, HIGH);
    PORTD = (1 << pump_out) | PORTD;
    pump_lastStart = currentTime;
    
    Serial.print(F("Pump turned ON at: "));
    Serial.println(currentTime);
  }

  // turn the pump off after pump_activeDuration has passed
  int isPumpOn = (PIND & (1 << pump_out)) >> pump_out;
  if (isPumpOn && currentTime - pump_lastStart >= pump_activeDuration) {
    // digitalWrite(pump_pin, LOW);
    PORTD = ~(1 << pump_out) & PORTD;
    Serial.print(F("Pump turned OFF at: "));
    Serial.println(currentTime);
  }

  // turn on the led strip if dark, otherwise turn it off 
  if (light != -1 && light < light_threshold) {
    // digitalWrite(light_out, HIGH); 
    PORTD = (1 << light_out) | PORTD;
  } else {
    // digitalWrite(light_out, LOW);
    PORTD = ~(1 << light_out) & PORTD;
  }

  // try to send an e-mail if the soil has been dry for a while
  if (moisture <= moisture_threshold && pump_lastStart - timeSinceLastMail >= minEmailInterval) {
    Serial.println(F("Trying to send e-mail!"));
    if (!internetAvailable)
      internetAvailable = Ethernet.begin(mac, 10000);
    // try to send an emai; and if it fails, show an icon telling there is no internet
    displayNoInternet(2, !sendEmail(moisture, light, isWarm));
    timeSinceLastMail = currentTime;
  }
}
