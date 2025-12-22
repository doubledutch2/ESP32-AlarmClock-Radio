#include "TimeModule.h"

TimeModule::TimeModule(const char* tzName) 
    : isInitialized(false), wifiConnected(false), timezoneName(tzName) {
}

bool TimeModule::begin(const char* ssid, const char* password) {
    // WiFi should already be connected by WiFiModule, but check anyway
    wifiConnected = (WiFi.status() == WL_CONNECTED);
    
    if (!wifiConnected) {
        Serial.println("WiFi not connected for time sync");
        return false;
    }
    
    Serial.println("Waiting for ezTime NTP sync...");
    
    // Wait for ezTime to sync with NTP
    waitForSync();
    
    Serial.println("UTC time synced");
    Serial.println("UTC: " + UTC.dateTime());
    
    // Set timezone
    if (timezoneName.length() > 0) {
        if (!setTimezone(timezoneName.c_str())) {
            Serial.println("Failed to set timezone, using UTC");
            timezoneName = "UTC";
        }
    } else {
        // Try to auto-detect timezone based on IP geolocation
        Serial.println("Auto-detecting timezone...");
        if (!myTZ.setLocation()) {
            Serial.println("Auto-detect failed, using UTC");
            timezoneName = "UTC";
            myTZ.setLocation("UTC");
        } else {
            timezoneName = myTZ.getTimezoneName();
            Serial.println("Auto-detected timezone: " + timezoneName);
        }
    }
    
    Serial.println("Local time: " + myTZ.dateTime());
    Serial.println("Timezone: " + timezoneName);
    Serial.println("DST active: " + String(isDST() ? "Yes" : "No"));
    
    isInitialized = true;
    return true;
}

bool TimeModule::setTimezone(const char* tzName) {
    if (!isInitialized) {
        timezoneName = tzName;
        return true;  // Will be applied in begin()
    }
    
    if (myTZ.setLocation(tzName)) {
        timezoneName = tzName;
        Serial.println("Timezone set to: " + timezoneName);
        Serial.println("Local time: " + myTZ.dateTime());
        Serial.println("DST active: " + String(isDST() ? "Yes" : "No"));
        return true;
    }
    
    Serial.println("Failed to set timezone: " + String(tzName));
    return false;
}

String TimeModule::getTimezoneName() {
    return timezoneName;
}

bool TimeModule::isDST() {
    if (!isInitialized) return false;
    return myTZ.isDST();
}

void TimeModule::loop() {
    // ezTime handles events automatically
    events();  // This checks for DST changes and handles them
}

bool TimeModule::isReady() {
    return isInitialized;
}

bool TimeModule::isWiFiConnected() {
    wifiConnected = (WiFi.status() == WL_CONNECTED);
    return wifiConnected;
}

uint8_t TimeModule::getHour() {
    if (!isInitialized) return 0;
    return myTZ.hour();
}

uint8_t TimeModule::getMinute() {
    if (!isInitialized) return 0;
    return myTZ.minute();
}

uint8_t TimeModule::getSecond() {
    if (!isInitialized) return 0;
    return myTZ.second();
}

uint8_t TimeModule::getDay() {
    if (!isInitialized) return 1;
    return myTZ.day();
}

uint8_t TimeModule::getMonth() {
    if (!isInitialized) return 1;
    return myTZ.month();
}

uint16_t TimeModule::getYear() {
    if (!isInitialized) return 2024;
    return myTZ.year();
}

uint8_t TimeModule::getDayOfWeek() {
    if (!isInitialized) return 0;
    return myTZ.weekday();  // 0=Sunday, 6=Saturday (same as tm_wday)
}

String TimeModule::getDayName() {
    if (!isInitialized) return "Unknown";
    return myTZ.dateTime("l");  // "Monday", "Tuesday", etc.
}

String TimeModule::getMonthName() {
    if (!isInitialized) return "Unknown";
    return myTZ.dateTime("F");  // "January", "February", etc.
}

void TimeModule::setTime(uint8_t hour, uint8_t minute, uint8_t second) {
    // Note: This is not recommended with ezTime as it will be overwritten by NTP
    // But keeping for compatibility
    Serial.printf("Manual time set to %02d:%02d:%02d (will be overwritten by NTP sync)\n", 
                  hour, minute, second);
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
            updateNTP();  // Sync time after reconnection
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
    updateNTP();
    
    // Wait a moment for sync
    delay(1000);
    
    Serial.println("Time synced: " + myTZ.dateTime());
    return true;
}

String TimeModule::getTimeString() {
    if (!isInitialized) return "00:00:00";
    return myTZ.dateTime("H:i:s");  // "14:35:22"
}

String TimeModule::getDateString() {
    if (!isInitialized) return "Unknown";
    // Format: "Mon, 21 Dec 2024"
    return myTZ.dateTime("D, d M Y");
}

String TimeModule::getFullDateString() {
    if (!isInitialized) return "Unknown";
    // Format: "Monday, 21 December 2024"
    return myTZ.dateTime("l, d F Y");
}

void TimeModule::updateEvents() {
    // This is called by loop() automatically
    // ezTime handles DST transitions through the events() system
}