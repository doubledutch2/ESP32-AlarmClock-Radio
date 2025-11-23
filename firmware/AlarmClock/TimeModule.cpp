#include "TimeModule.h"

TimeModule::TimeModule(const char* ntp, long gmtOffset, int dstOffset) 
    : isInitialized(false), wifiConnected(false), ntpServer(ntp), 
      gmtOffset_sec(gmtOffset), daylightOffset_sec(dstOffset) {
    memset(&timeinfo, 0, sizeof(timeinfo));
}

bool TimeModule::begin(const char* ssid, const char* password) {
    Serial.println("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nWiFi connection failed!");
        return false;
    }
    
    wifiConnected = true;
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Configure time with NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // Wait for time to be set
    Serial.println("Waiting for NTP time sync...");
    attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 10) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }
    
    if (attempts >= 10) {
        Serial.println("\nFailed to obtain time from NTP");
        return false;
    }
    
    Serial.println("\nTime synchronized!");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    
    isInitialized = true;
    return true;
}

bool TimeModule::isReady() {
    return isInitialized;
}

bool TimeModule::isWiFiConnected() {
    wifiConnected = (WiFi.status() == WL_CONNECTED);
    return wifiConnected;
}

void TimeModule::updateLocalTime() {
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
    }
}

uint8_t TimeModule::getHour() {
    updateLocalTime();
    return timeinfo.tm_hour;
}

uint8_t TimeModule::getMinute() {
    updateLocalTime();
    return timeinfo.tm_min;
}

uint8_t TimeModule::getSecond() {
    updateLocalTime();
    return timeinfo.tm_sec;
}

uint8_t TimeModule::getDay() {
    updateLocalTime();
    return timeinfo.tm_mday;
}

uint8_t TimeModule::getMonth() {
    updateLocalTime();
    return timeinfo.tm_mon + 1;  // tm_mon is 0-11
}

uint16_t TimeModule::getYear() {
    updateLocalTime();
    return timeinfo.tm_year + 1900;  // tm_year is years since 1900
}

uint8_t TimeModule::getDayOfWeek() {
    updateLocalTime();
    return timeinfo.tm_wday;  // 0=Sunday, 6=Saturday
}

void TimeModule::setTime(uint8_t hour, uint8_t minute, uint8_t second) {
    struct timeval tv;
    struct tm tm;
    
    updateLocalTime();
    tm = timeinfo;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    
    tv.tv_sec = mktime(&tm);
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
    
    Serial.printf("Time manually set to %02d:%02d:%02d\n", hour, minute, second);
}

void TimeModule::reconnectWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reconnecting to WiFi...");
        WiFi.reconnect();
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nReconnected!");
            wifiConnected = true;
        } else {
            Serial.println("\nReconnection failed");
            wifiConnected = false;
        }
    }
}

String TimeModule::getIPAddress() {
    if (wifiConnected) {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

int TimeModule::getWiFiSignal() {
    if (wifiConnected) {
        return WiFi.RSSI();
    }
    return 0;
}

bool TimeModule::syncTime() {
    if (!wifiConnected) {
        Serial.println("Cannot sync time - WiFi not connected");
        return false;
    }
    
    Serial.println("Syncing time with NTP...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 5) {
        delay(1000);
        attempts++;
    }
    
    if (attempts >= 5) {
        Serial.println("Time sync failed");
        return false;
    }
    
    Serial.println("Time synced successfully");
    return true;
}

String TimeModule::getTimeString() {
    updateLocalTime();
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return String(buffer);
}

String TimeModule::getDateString() {
    updateLocalTime();
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
    return String(buffer);
}