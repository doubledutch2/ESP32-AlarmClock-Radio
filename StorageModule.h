// ==================== StorageModule.h ====================
/*
#ifndef STORAGE_MODULE_H
#define STORAGE_MODULE_H

#include <Arduino.h>
#include <Preferences.h>
#include <LittleFS.h>

#define MAX_STATIONS 50
#define MAX_ALARMS 10

struct StoredStation {
  char name[64];
  char url[256];
};

struct StoredAlarm {
  uint8_t hour;
  uint8_t minute;
  bool enabled;
  uint8_t days;  // Bit field: Sun=0x01, Mon=0x02, Tue=0x04, etc.
  char soundFile[64];  // Path to WAV file in LittleFS
};

struct WiFiCredentials {
  char ssid[32];
  char password[64];
};

class StorageModule {
private:
  Preferences prefs;
  bool littleFSMounted;
  
  // Helper functions
  String getStationKey(int index);
  String getAlarmKey(int index);
  
public:
  StorageModule();
  ~StorageModule();
  
  // Initialize storage
  bool begin();
  void format();  // Erase all data (use carefully!)
  
  // WiFi Credentials
  bool saveWiFiCredentials(const char* ssid, const char* password);
  bool loadWiFiCredentials(WiFiCredentials& creds);
  bool hasWiFiCredentials();
  
  // Radio Stations
  bool saveStation(int index, const char* name, const char* url);
  bool loadStation(int index, StoredStation& station);
  int getStationCount();
  void setStationCount(int count);
  bool deleteStation(int index);
  
  // Alarms
  bool saveAlarm(int index, const StoredAlarm& alarm);
  bool loadAlarm(int index, StoredAlarm& alarm);
  int getAlarmCount();
  void setAlarmCount(int count);
  bool deleteAlarm(int index);
  
  // Settings
  bool saveVolume(int volume);
  int loadVolume(int defaultVol = 10);
  bool saveBrightness(int brightness);
  int loadBrightness(int defaultBright = 3);
  bool saveTimezone(int gmtOffset, int dstOffset);
  bool loadTimezone(int& gmtOffset, int& dstOffset);
  
  // File System (for WAV files, etc.)
  bool writeFile(const char* path, const uint8_t* data, size_t len);
  bool readFile(const char* path, uint8_t* buffer, size_t* len);
  bool deleteFile(const char* path);
  bool fileExists(const char* path);
  size_t getFileSize(const char* path);
  void listFiles(const char* dirname = "/");
  size_t getTotalSpace();
  size_t getUsedSpace();
  size_t getFreeSpace();
};

#endif
*/
