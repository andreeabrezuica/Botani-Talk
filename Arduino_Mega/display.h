#ifndef DISPLAY_H
#define DISPLAY_H

#include <Ucglib.h>

// Base class for display sections
class DisplaySection {
protected:
  Ucglib4WireSWSPI& display;
  int xPos;
  int yPos;
  int width;
  int height;

public:
  DisplaySection(Ucglib4WireSWSPI& display, int xPos, int yPos, int width, int height)
    : display(display), xPos(xPos), yPos(yPos), width(width), height(height) {}

  virtual void update() = 0;
  virtual ~DisplaySection(){};
};

// Class for dusplaying sensor values
class SensorDisplay : public DisplaySection {
private:
  uint8_t moisture;
  uint8_t light;
  float temperature;

public:
  SensorDisplay(Ucglib4WireSWSPI& display, int xPos, int yPos, int width, int height)
    : DisplaySection(display, xPos, yPos, width, height) {}

  void setSensorData(float moisture, uint8_t light, float temp) {
    this->moisture = moisture;
    this->light = light;
    this->temperature = temp;
  }

  void update() override {
    // Display sensor values in this section
    // Position content within the specified region
    display.setColor(255, 255, 255);
    display.drawBox(xPos, yPos, width, height);
    display.setColor(0, 0, 0);
    display.setPrintPos(xPos + 5, yPos + 10);
    display.print("Moisture: ");
    display.println(moisture);
    display.setPrintPos(xPos + 5, yPos + 20);
    display.print("Light: ");
    display.println(light);
    display.setPrintPos(xPos + 5, yPos + 30);
    display.print("Temperature: ");
    display.print(light);
    display.println("C");
  }
};

// Class for displaying pump status
class PumpStatusDisplay : public DisplaySection {
private:
  bool isPumpOn;

public:
  PumpStatusDisplay(Ucglib4WireSWSPI& display, int xPos, int yPos, int width, int height)
    : DisplaySection(display, xPos, yPos, width, height) {}

  void setPumpStatus(bool isPumpOn) {
    this->isPumpOn = isPumpOn;
  }

  void update() override {
    // Display pump status in this section
    // Position content within the specified region
    display.setColor(255, 255, 255);
    display.drawBox(xPos, yPos, width, height);
    display.setColor(0, 0, 0);
    display.setPrintPos(xPos + 5, yPos + 10);
    display.print("Pump: ");
    display.println(isPumpOn ? "ON" : "OFF");
  }
};

// Class for displaying internet connection status
class InternetStatusDisplay : public DisplaySection {
private:
  bool isConnected;
  String ipAddress;

public:
  InternetStatusDisplay(Ucglib4WireSWSPI& display, int xPos, int yPos, int width, int height)
    : DisplaySection(display, xPos, yPos, width, height) {}

  void setConnectionStatus(bool isConnected, const String& ipAddress) {
    this->isConnected = isConnected;
    this->ipAddress = ipAddress;
  }

  void update() override {
    // Display internet connection status in this section
    // Position content within the specified region
    display.setColor(255, 255, 255);
    display.drawBox(xPos, yPos, width, height);
    display.setColor(0, 0, 0);
    display.setPrintPos(xPos + 5, yPos + 10);
    if (isConnected) {
      display.print("Connection: Connected");
      display.setPrintPos(xPos + 5, yPos + 20);
      display.print("IP Address: ");
      display.println(ipAddress);
    } else {
      display.print("Connection: Disconnected");
    }
  }
};

#endif  // end display.h
