// ==================== DisplayInterface.h ====================
#ifndef DISPLAY_INTERFACE_H
#define DISPLAY_INTERFACE_H

#include <Arduino.h>

// Abstract base class for all display types
class DisplayInterface {
public:
  virtual ~DisplayInterface() {}
  
  virtual bool begin() = 0;
  virtual void clear() = 0;
  virtual void update() = 0;
  virtual void setBrightness(uint8_t level) = 0;
  virtual void drawClock(int hours, int minutes, int seconds) = 0;
  virtual void drawInfo(int volume, int brightness) = 0;
  virtual void showStartupMessage(const char* message) = 0;
};

#endif
