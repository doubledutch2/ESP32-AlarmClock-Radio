// ==================== WiFiModule.cpp ====================
#include "WiFiModule.h"

WiFiModule::WiFiModule(const char* ssid, const char* password) {
  this->ssid = String(ssid);
  this->password = String(password);
  this->lastCheckTime = 0;
}

bool WiFiModule::connect() {
  Serial.println("\n=== WiFi Connection ===");
  Serial.printf("SSID: %s\n", ssid.c_str());
  
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
    return true;
  } else {
    Serial.println("\nFailed to connect to WiFi!");
    return false;
  }
}

bool WiFiModule::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WiFiModule::checkConnection() {
  unsigned long now = millis();
  if (now - lastCheckTime < CHECK_INTERVAL) {
    return;
  }
  
  lastCheckTime = now;
  
  if (!isConnected()) {
    Serial.println("WiFi disconnected! Reconnecting...");
    connect();
  }
}

String WiFiModule::getLocalIP() {
  return WiFi.localIP().toString();
}

int WiFiModule::getRSSI() {
  return WiFi.RSSI();
}
