#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <WiFi.h>
#include <string>

class WiFiModule {
private:
    const char* ssid;
    const char* password;
    bool connected;
    unsigned long lastCheckTime;
    static const unsigned long CHECK_INTERVAL = 30000; // Check every 30 seconds

public:
    WiFiModule(const char* ssid, const char* password);
    
    bool connect();
    void checkConnection();
    void reconnect();
    void disconnect();
    
    bool isConnected();
    String getLocalIP();
    int getSignalStrength();
    String getSSID();
};

#endif