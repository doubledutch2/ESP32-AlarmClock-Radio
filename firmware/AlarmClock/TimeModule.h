#ifndef TIME_MODULE_H
#define TIME_MODULE_H

#include <WiFi.h>
#include <ezTime.h>

class TimeModule {
private:
    bool isInitialized;
    bool wifiConnected;
    Timezone myTZ;  // ezTime timezone object
    String timezoneName;
    
    void updateEvents();  // Check for DST changes

public:
    TimeModule(const char* tzName = "");  // Pass timezone like "Europe/London" or "America/New_York"
    
    bool begin(const char* ssid, const char* password);
    bool isReady();
    bool isWiFiConnected();
    
    // Time getters
    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getSecond();
    uint8_t getDay();
    uint8_t getMonth();
    uint16_t getYear();
    uint8_t getDayOfWeek();  // 0=Sunday, 6=Saturday
    String getDayName();     // "Monday", "Tuesday", etc.
    String getMonthName();   // "January", "February", etc.
    
    // Time setters (manual adjustment - overrides NTP temporarily)
    void setTime(uint8_t hour, uint8_t minute, uint8_t second);
    
    // Timezone management
    bool setTimezone(const char* tzName);  // e.g., "Europe/London", "America/New_York"
    String getTimezoneName();
    bool isDST();  // Is Daylight Saving Time active?
    
    // WiFi management
    void reconnectWiFi();
    String getIPAddress();
    int getWiFiSignal();
    
    // NTP sync
    bool syncTime();
    void loop();  // Call this regularly to handle DST changes
    
    // Formatted strings
    String getTimeString();      // "14:35:22"
    String getDateString();      // "Mon, 21 Dec 2024"
    String getFullDateString();  // "Monday, 21 December 2024"
};

#endif