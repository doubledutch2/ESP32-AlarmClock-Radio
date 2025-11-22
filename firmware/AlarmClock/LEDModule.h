// ==================== LEDModule.h ====================
#ifndef LED_MODULE_H
#define LED_MODULE_H

#include <Adafruit_NeoPixel.h>

struct RGB {
  uint8_t r, g, b;
};

class LEDModule {
private:
  Adafruit_NeoPixel* led;
  uint8_t brightness;
  
public:
  static const RGB COLOR_OFF;
  static const RGB COLOR_RED;
  static const RGB COLOR_GREEN;
  static const RGB COLOR_BLUE;
  static const RGB COLOR_YELLOW;
  static const RGB COLOR_MAGENTA;
  
  LEDModule(uint8_t pin, uint8_t numLeds = 1);
  ~LEDModule();
  
  void begin();
  void setColor(const RGB& color, uint8_t bright = 250);
  void off();
};

#endif

