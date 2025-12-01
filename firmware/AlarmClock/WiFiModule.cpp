#include "WiFiModule.h"

WiFiModule::WiFiModule(const char* ssid, const char* password) 
    : ssid(ssid), password(password), connected(false), lastCheckTime(0) {
}

bool WiFiModule::connect() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    Serial.println("WiFi: 1");
    WiFi.mode(WIFI_STA);
    Serial.println("WiFi: 2");
    WiFi.begin(ssid, password);
    
    Serial.println("WiFi: 3");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    Serial.println("WiFi: 4");
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    Serial.println("WiFi: 5");
    if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        return true;
    } else {
        connected = false;
        Serial.println("\nWiFi connection failed!");
        return false;
    }
}

void WiFiModule::checkConnection() {
    unsigned long now = millis();
    
    if (now - lastCheckTime < CHECK_INTERVAL) {
        return;
    }
    
    lastCheckTime = now;
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost, attempting to reconnect...");
        connected = false;
        reconnect();
    } else {
        connected = true;
    }
}

void WiFiModule::reconnect() {
    WiFi.disconnect();
    delay(100);
    connect();
}

void WiFiModule::disconnect() {
    WiFi.disconnect();
    connected = false;
    Serial.println("WiFi disconnected");
}

bool WiFiModule::isConnected() {
    return connected && (WiFi.status() == WL_CONNECTED);
}

String WiFiModule::getLocalIP() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

int WiFiModule::getSignalStrength() {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}

String WiFiModule::getSSID() {
    return String(ssid);
}