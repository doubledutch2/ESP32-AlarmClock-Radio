// ==================== TimeModule.h ====================
#ifndef TIME_MODULE_H
#define TIME_MODULE_H

#include <Arduino.h>
#include <time.h>

class TimeModule {
private:
  int hours;
  int minutes;
  int seconds;
  unsigned long lastUpdate;
  int gmtOffset;
  int dstOffset;
  
public:
  TimeModule(int gmt = 0, int dst = 0);
  
  void begin();
  void loop();
  void syncNTP();
  
  int getHours() { return hours; }
  int getMinutes() { return minutes; }
  int getSeconds() { return seconds; }
};

#endif
