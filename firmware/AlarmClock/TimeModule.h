#ifndef TIME_MODULE_H
#define TIME_MODULE_H

#include <WiFi.h>
#include <time.h>

class TimeModule {
private:
    bool isInitialized;
    bool wifiConnected;
    struct tm timeinfo;
    const char* ntpServer;
    long gmtOffset_sec;
    int daylightOffset_sec;
    
    void updateLocalTime();

public:
    TimeModule(const char* ntp = "pool.ntp.org", long gmtOffset = 0, int dstOffset = 0);
    
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
    uint8_t getDayOfWeek();
    
    // Time setters (manual adjustment)
    void setTime(uint8_t hour, uint8_t minute, uint8_t second);
    
    // WiFi management
    void reconnectWiFi();
    String getIPAddress();
    int getWiFiSignal();
    
    // NTP sync
    bool syncTime();
    String getTimeString();
    String getDateString();
};

#endif