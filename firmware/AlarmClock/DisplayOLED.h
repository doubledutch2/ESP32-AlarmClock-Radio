// ==================== DisplayOLED.h ====================
#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include "DisplayInterface.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class DisplayOLED : public DisplayInterface {
private:
  Adafruit_SSD1306* display;
  uint8_t brightnessLevel;
  
  static const int SCREEN_WIDTH = 128;
  static const int SCREEN_HEIGHT = 64;
  static const int CLOCK_CENTER_X = 30;
  static const int CLOCK_CENTER_Y = 36;
  static const int CLOCK_RADIUS = 26;
  
public:
  DisplayOLED(uint8_t sdaPin, uint8_t sclPin);
  ~DisplayOLED();
  
  bool begin() override;
  void clear() override;
  void update() override;
  void setBrightness(uint8_t level) override;
  void drawClock(int hours, int minutes, int seconds) override;
  void drawInfo(int volume, int brightness) override;
  void showStartupMessage(const char* message) override;
};

#endif

