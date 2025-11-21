// ==================== WiFiModule.h ====================
#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <WiFi.h>

class WiFiModule {
private:
  String ssid;
  String password;
  unsigned long lastCheckTime;
  static const unsigned long CHECK_INTERVAL = 30000;  // Check every 30 seconds
  
public:
  WiFiModule(const char* ssid, const char* password);
  
  bool connect();
  bool isConnected();
  void checkConnection();
  String getLocalIP();
  int getRSSI();
};

#endif