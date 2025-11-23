#include "StorageModule.h"

StorageModule::StorageModule() : isInitialized(false), stationCount(0) {
    for (int i = 0; i < MAX_STATIONS; i++) {
        stations[i].frequency = 0.0;
        stations[i].name[0] = '\0';
    }
}

bool StorageModule::begin() {
    // Initialize LittleFS
    if (!LittleFS.begin(true)) {  // true = format if mount fails
        Serial.println("LittleFS Mount Failed");
        return false;
    }
    
    Serial.println("LittleFS mounted successfully");
    
    // Initialize NVS Preferences
    prefs.begin("alarmclock", false);  // false = read/write mode
    
    isInitialized = true;
    
    // Load saved data
    uint8_t h, m;
    bool en;
    float freq;
    loadConfig(h, m, en, freq);
    loadStations();
    
    return true;
}

bool StorageModule::isReady() {
    return isInitialized;
}

// ===== NVS CONFIGURATION (fast, small data) =====
bool StorageModule::saveConfig(uint8_t alarmHour, uint8_t alarmMin, bool alarmEnabled, float fmFreq) {
    if (!isInitialized) return false;
    
    prefs.putUChar("alarmHour", alarmHour);
    prefs.putUChar("alarmMin", alarmMin);
    prefs.putBool("alarmEnabled", alarmEnabled);
    prefs.putFloat("fmFreq", fmFreq);
    
    Serial.println("Config saved to NVS");
    return true;
}

bool StorageModule::loadConfig(uint8_t &alarmHour, uint8_t &alarmMin, bool &alarmEnabled, float &fmFreq) {
    if (!isInitialized) return false;
    
    // Load with defaults if keys don't exist
    alarmHour = prefs.getUChar("alarmHour", 7);
    alarmMin = prefs.getUChar("alarmMin", 0);
    alarmEnabled = prefs.getBool("alarmEnabled", false);
    fmFreq = prefs.getFloat("fmFreq", 98.0);
    
    Serial.printf("Config loaded: Alarm %02d:%02d %s, FM %.1f\n", 
                  alarmHour, alarmMin, alarmEnabled ? "ON" : "OFF", fmFreq);
    
    return true;
}

// ===== LITTLEFS STATION STORAGE (larger data, file-based) =====
bool StorageModule::saveStations() {
    if (!isInitialized) return false;
    
    File file = LittleFS.open(stationsFile, "w");
    if (!file) {
        Serial.println("Failed to open stations file for writing");
        return false;
    }
    
    for (int i = 0; i < stationCount; i++) {
        file.printf("%.1f,%s\n", stations[i].frequency, stations[i].name);
    }
    file.close();
    
    Serial.printf("Saved %d stations to LittleFS\n", stationCount);
    return true;
}

bool StorageModule::loadStations() {
    if (!isInitialized) return false;
    
    if (!LittleFS.exists(stationsFile)) {
        Serial.println("Stations file not found");
        return false;
    }
    
    File file = LittleFS.open(stationsFile, "r");
    if (!file) {
        Serial.println("Failed to open stations file for reading");
        return false;
    }
    
    stationCount = 0;
    while (file.available() && stationCount < MAX_STATIONS) {
        String line = file.readStringUntil('\n');
        line.trim();
        
        int commaPos = line.indexOf(',');
        if (commaPos > 0) {
            stations[stationCount].frequency = line.substring(0, commaPos).toFloat();
            String name = line.substring(commaPos + 1);
            strncpy(stations[stationCount].name, name.c_str(), 31);
            stations[stationCount].name[31] = '\0';
            stationCount++;
        }
    }
    file.close();
    
    Serial.printf("Loaded %d stations from LittleFS\n", stationCount);
    return true;
}

bool StorageModule::addStation(float frequency, const char* name) {
    if (stationCount >= MAX_STATIONS) {
        Serial.println("Station list full");
        return false;
    }
    
    stations[stationCount].frequency = frequency;
    strncpy(stations[stationCount].name, name, 31);
    stations[stationCount].name[31] = '\0';
    stationCount++;
    
    return saveStations();
}

bool StorageModule::removeStation(int index) {
    if (index < 0 || index >= stationCount) return false;
    
    // Shift stations down
    for (int i = index; i < stationCount - 1; i++) {
        stations[i] = stations[i + 1];
    }
    stationCount--;
    
    return saveStations();
}

RadioStation* StorageModule::getStation(int index) {
    if (index < 0 || index >= stationCount) return nullptr;
    return &stations[index];
}

int StorageModule::getStationCount() {
    return stationCount;
}

void StorageModule::setStationCount(int count) {
    if (count >= 0 && count <= MAX_STATIONS) {
        stationCount = count;
    }
}

void StorageModule::clearStations() {
    stationCount = 0;
    for (int i = 0; i < MAX_STATIONS; i++) {
        stations[i].frequency = 0.0;
        stations[i].name[0] = '\0';
    }
    saveStations();
    Serial.println("All stations cleared");
}

void StorageModule::factoryReset() {
    Serial.println("Performing factory reset...");
    
    // Clear NVS preferences
    prefs.clear();
    
    // Clear stations
    clearStations();
    
    // Delete stations file
    if (LittleFS.exists(stationsFile)) {
        LittleFS.remove(stationsFile);
    }
    
    Serial.println("Factory reset complete");
}