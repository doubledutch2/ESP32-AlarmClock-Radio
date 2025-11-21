// ==================== TimeModule.cpp ====================
#include "TimeModule.h"

TimeModule::TimeModule(int gmt, int dst) {
  gmtOffset = gmt;
  dstOffset = dst;
  hours = 0;
  minutes = 0;
  seconds = 0;
  lastUpdate = 0;
}

void TimeModule::begin() {
  configTime(gmtOffset * 3600, dstOffset * 3600, "pool.ntp.org");
  syncNTP();
}

void TimeModule::loop() {
  unsigned long now = millis();
  if (now - lastUpdate < 1000) {
    return;
  }
  
  lastUpdate = now;
  seconds++;
  
  if (seconds >= 60) {
    seconds = 0;
    minutes++;
    
    if (minutes >= 60) {
      minutes = 0;
      hours++;
      
      if (hours >= 24) {
        hours = 0;
      }
      
      // Sync with NTP once per hour
      syncNTP();
    }
  }
}

void TimeModule::syncNTP() {
  struct tm timeinfo;
  
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time from NTP");
    return;
  }
  
  hours = timeinfo.tm_hour;
  minutes = timeinfo.tm_min;
  seconds = timeinfo.tm_sec;
  
  Serial.printf("NTP Time synced: %02d:%02d:%02d\n", hours, minutes, seconds);
}

