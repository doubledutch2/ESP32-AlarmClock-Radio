// ==================== StorageModule.cpp ====================
/*
#include "StorageModule.h"

StorageModule::StorageModule() {
  littleFSMounted = false;
}

StorageModule::~StorageModule() {
  prefs.end();
  if (littleFSMounted) {
    LittleFS.end();
  }
}

bool StorageModule::begin() {
  Serial.println("\n=== Storage Module Init ===");
  
  // Initialize NVS (Preferences)
  if (!prefs.begin("alarmclock", false)) {
    Serial.println("Failed to initialize NVS");
    return false;
  }
  Serial.println("NVS initialized");
  
  // Initialize LittleFS
  if (!LittleFS.begin(true)) {  // true = format on fail
    Serial.println("LittleFS mount failed");
    return false;
  }
  littleFSMounted = true;
  
  Serial.printf("LittleFS mounted - Total: %d KB, Used: %d KB, Free: %d KB\n",
    getTotalSpace() / 1024, getUsedSpace() / 1024, getFreeSpace() / 1024);
  
  return true;
}

void StorageModule::format() {
  Serial.println("WARNING: Formatting all storage!");
  prefs.clear();
  if (littleFSMounted) {
    LittleFS.format();
  }
  Serial.println("Storage formatted");
}

// ==================== WiFi Credentials ====================

bool StorageModule::saveWiFiCredentials(const char* ssid, const char* password) {
  prefs.putString("wifi_ssid", ssid);
  prefs.putString("wifi_pass", password);
  Serial.printf("WiFi credentials saved: %s\n", ssid);
  return true;
}

bool StorageModule::loadWiFiCredentials(WiFiCredentials& creds) {
  if (!hasWiFiCredentials()) {
    return false;
  }
  
  String ssid = prefs.getString("wifi_ssid", "");
  String pass = prefs.getString("wifi_pass", "");
  
  strncpy(creds.ssid, ssid.c_str(), 31);
  strncpy(creds.password, pass.c_str(), 63);
  creds.ssid[31] = '\0';
  creds.password[63] = '\0';
  
  return true;
}

bool StorageModule::hasWiFiCredentials() {
  return prefs.isKey("wifi_ssid") && prefs.isKey("wifi_pass");
}

// ==================== Radio Stations ====================

String StorageModule::getStationKey(int index) {
  return "sta_" + String(index);
}

bool StorageModule::saveStation(int index, const char* name, const char* url) {
  if (index < 0 || index >= MAX_STATIONS) {
    return false;
  }
  
  String key = getStationKey(index);
  String value = String(name) + "|" + String(url);
  
  prefs.putString(key.c_str(), value);
  Serial.printf("Station %d saved: %s\n", index, name);
  return true;
}

bool StorageModule::loadStation(int index, StoredStation& station) {
  if (index < 0 || index >= MAX_STATIONS) {
    return false;
  }
  
  String key = getStationKey(index);
  String value = prefs.getString(key.c_str(), "");
  
  if (value.length() == 0) {
    return false;
  }
  
  // Parse "name|url"
  int sepIndex = value.indexOf('|');
  if (sepIndex == -1) {
    return false;
  }
  
  String name = value.substring(0, sepIndex);
  String url = value.substring(sepIndex + 1);
  
  strncpy(station.name, name.c_str(), 63);
  strncpy(station.url, url.c_str(), 255);
  station.name[63] = '\0';
  station.url[255] = '\0';
  
  return true;
}

int StorageModule::getStationCount() {
  return prefs.getInt("sta_count", 0);
}

void StorageModule::setStationCount(int count) {
  prefs.putInt("sta_count", count);
}

bool StorageModule::deleteStation(int index) {
  if (index < 0 || index >= MAX_STATIONS) {
    return false;
  }
  
  String key = getStationKey(index);
  prefs.remove(key.c_str());
  return true;
}

// ==================== Alarms ====================

String StorageModule::getAlarmKey(int index) {
  return "alm_" + String(index);
}

bool StorageModule::saveAlarm(int index, const StoredAlarm& alarm) {
  if (index < 0 || index >= MAX_ALARMS) {
    return false;
  }
  
  String key = getAlarmKey(index);
  
  // Pack alarm data into a string: "HH:MM:E:DD:soundfile"
  String value = String(alarm.hour) + ":" + 
                 String(alarm.minute) + ":" +
                 String(alarm.enabled ? 1 : 0) + ":" +
                 String(alarm.days) + ":" +
                 String(alarm.soundFile);
  
  prefs.putString(key.c_str(), value);
  Serial.printf("Alarm %d saved: %02d:%02d\n", index, alarm.hour, alarm.minute);
  return true;
}

bool StorageModule::loadAlarm(int index, StoredAlarm& alarm) {
  if (index < 0 || index >= MAX_ALARMS) {
    return false;
  }
  
  String key = getAlarmKey(index);
  String value = prefs.getString(key.c_str(), "");
  
  if (value.length() == 0) {
    return false;
  }
  
  // Parse "HH:MM:E:DD:soundfile"
  int idx[4];
  idx[0] = value.indexOf(':');
  idx[1] = value.indexOf(':', idx[0] + 1);
  idx[2] = value.indexOf(':', idx[1] + 1);
  idx[3] = value.indexOf(':', idx[2] + 1);
  
  if (idx[3] == -1) {
    return false;
  }
  
  alarm.hour = value.substring(0, idx[0]).toInt();
  alarm.minute = value.substring(idx[0] + 1, idx[1]).toInt();
  alarm.enabled = value.substring(idx[1] + 1, idx[2]).toInt() == 1;
  alarm.days = value.substring(idx[2] + 1, idx[3]).toInt();
  
  String soundFile = value.substring(idx[3] + 1);
  strncpy(alarm.soundFile, soundFile.c_str(), 63);
  alarm.soundFile[63] = '\0';
  
  return true;
}

int StorageModule::getAlarmCount() {
  return prefs.getInt("alm_count", 0);
}

void StorageModule::setAlarmCount(int count) {
  prefs.putInt("alm_count", count);
}

bool StorageModule::deleteAlarm(int index) {
  if (index < 0 || index >= MAX_ALARMS) {
    return false;
  }
  
  String key = getAlarmKey(index);
  prefs.remove(key.c_str());
  return true;
}

// ==================== Settings ====================

bool StorageModule::saveVolume(int volume) {
  prefs.putInt("volume", volume);
  return true;
}

int StorageModule::loadVolume(int defaultVol) {
  return prefs.getInt("volume", defaultVol);
}

bool StorageModule::saveBrightness(int brightness) {
  prefs.putInt("brightness", brightness);
  return true;
}

int StorageModule::loadBrightness(int defaultBright) {
  return prefs.getInt("brightness", defaultBright);
}

bool StorageModule::saveTimezone(int gmtOffset, int dstOffset) {
  prefs.putInt("gmt_offset", gmtOffset);
  prefs.putInt("dst_offset", dstOffset);
  return true;
}

bool StorageModule::loadTimezone(int& gmtOffset, int& dstOffset) {
  gmtOffset = prefs.getInt("gmt_offset", 0);
  dstOffset = prefs.getInt("dst_offset", 0);
  return true;
}

// ==================== File System (LittleFS) ====================

bool StorageModule::writeFile(const char* path, const uint8_t* data, size_t len) {
  if (!littleFSMounted) {
    return false;
  }
  
  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.printf("Failed to open file for writing: %s\n", path);
    return false;
  }
  
  size_t written = file.write(data, len);
  file.close();
  
  Serial.printf("File written: %s (%d bytes)\n", path, written);
  return written == len;
}

bool StorageModule::readFile(const char* path, uint8_t* buffer, size_t* len) {
  if (!littleFSMounted) {
    return false;
  }
  
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.printf("Failed to open file for reading: %s\n", path);
    return false;
  }
  
  size_t fileSize = file.size();
  if (fileSize > *len) {
    Serial.println("Buffer too small");
    file.close();
    return false;
  }
  
  *len = file.read(buffer, fileSize);
  file.close();
  
  Serial.printf("File read: %s (%d bytes)\n", path, *len);
  return true;
}

bool StorageModule::deleteFile(const char* path) {
  if (!littleFSMounted) {
    return false;
  }
  
  if (LittleFS.remove(path)) {
    Serial.printf("File deleted: %s\n", path);
    return true;
  }
  return false;
}

bool StorageModule::fileExists(const char* path) {
  if (!littleFSMounted) {
    return false;
  }
  
  return LittleFS.exists(path);
}

size_t StorageModule::getFileSize(const char* path) {
  if (!littleFSMounted) {
    return 0;
  }
  
  File file = LittleFS.open(path, "r");
  if (!file) {
    return 0;
  }
  
  size_t size = file.size();
  file.close();
  return size;
}

void StorageModule::listFiles(const char* dirname) {
  if (!littleFSMounted) {
    return;
  }
  
  Serial.printf("\nListing directory: %s\n", dirname);
  
  File root = LittleFS.open(dirname);
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open directory");
    return;
  }
  
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.printf("  DIR: %s\n", file.name());
    } else {
      Serial.printf("  FILE: %s (%d bytes)\n", file.name(), file.size());
    }
    file = root.openNextFile();
  }
}

size_t StorageModule::getTotalSpace() {
  if (!littleFSMounted) {
    return 0;
  }
  return LittleFS.totalBytes();
}

size_t StorageModule::getUsedSpace() {
  if (!littleFSMounted) {
    return 0;
  }
  return LittleFS.usedBytes();
}

size_t StorageModule::getFreeSpace() {
  return getTotalSpace() - getUsedSpace();
}
*/

