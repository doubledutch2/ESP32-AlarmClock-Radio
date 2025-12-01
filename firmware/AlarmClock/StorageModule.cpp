#include "StorageModule.h"

StorageModule::StorageModule() : isInitialized(false), stationCount(0) {
    for (int i = 0; i < MAX_STATIONS; i++) {
        fmPresets[i].frequency = 0.0;
        fmPresets[i].name[0] = '\0';
    }
}

bool StorageModule::begin() {
    // Initialize LittleFS
    Serial.println("Before Init LittleFS");
    // delay(1000);
    if (!LittleFS.begin(true,"/spiffs")) {  // true = format if mount fails
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
    Serial.println("Before load Config");
    loadConfig(h, m, en, freq);
    Serial.println("Before Load FM Stations");
    loadFMStations();
    
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

// ===== LITTLEFS FM STATION STORAGE (larger data, file-based) =====
bool StorageModule::saveFMStations() {
    if (!isInitialized) return false;
    
    File file = LittleFS.open(stationsFile, "w");
    if (!file) {
        Serial.println("Failed to open FM stations file for writing");
        return false;
    }
    
    for (int i = 0; i < stationCount; i++) {
        file.printf("%.1f,%s\n", fmPresets[i].frequency, fmPresets[i].name);
    }
    file.close();
    
    Serial.printf("Saved %d FM stations to LittleFS\n", stationCount);
    return true;
}

bool StorageModule::loadFMStations() {
    if (!isInitialized) return false;
    
    if (!LittleFS.exists(stationsFile)) {
        Serial.println("FM stations file not found");
        return false;
    }
    
    File file = LittleFS.open(stationsFile, "r");
    if (!file) {
        Serial.println("Failed to open FM stations file for reading");
        return false;
    }
    
    stationCount = 0;
    while (file.available() && stationCount < MAX_STATIONS) {
        String line = file.readStringUntil('\n');
        line.trim();
        
        int commaPos = line.indexOf(',');
        if (commaPos > 0) {
            fmPresets[stationCount].frequency = line.substring(0, commaPos).toFloat();
            String name = line.substring(commaPos + 1);
            strncpy(fmPresets[stationCount].name, name.c_str(), 31);
            fmPresets[stationCount].name[31] = '\0';
            stationCount++;
        }
    }
    file.close();
    
    Serial.printf("Loaded %d FM stations from LittleFS\n", stationCount);
    return true;
}

bool StorageModule::addFMStation(float frequency, const char* name) {
    if (stationCount >= MAX_STATIONS) {
        Serial.println("FM station list full");
        return false;
    }
    
    fmPresets[stationCount].frequency = frequency;
    strncpy(fmPresets[stationCount].name, name, 31);
    fmPresets[stationCount].name[31] = '\0';
    stationCount++;
    
    return saveFMStations();
}

bool StorageModule::removeFMStation(int index) {
    if (index < 0 || index >= stationCount) return false;
    
    // Shift stations down
    for (int i = index; i < stationCount - 1; i++) {
        fmPresets[i] = fmPresets[i + 1];
    }
    stationCount--;
    
    return saveFMStations();
}

FMRadioPreset* StorageModule::getFMStation(int index) {
    if (index < 0 || index >= stationCount) return nullptr;
    return &fmPresets[index];
}

int StorageModule::getFMStationCount() {
    return stationCount;
}

void StorageModule::setFMStationCount(int count) {
    if (count >= 0 && count <= MAX_STATIONS) {
        stationCount = count;
    }
}

void StorageModule::clearFMStations() {
    stationCount = 0;
    for (int i = 0; i < MAX_STATIONS; i++) {
        fmPresets[i].frequency = 0.0;
        fmPresets[i].name[0] = '\0';
    }
    saveFMStations();
    Serial.println("All FM stations cleared");
}

void StorageModule::factoryReset() {
    Serial.println("Performing factory reset...");
    
    // Clear NVS preferences
    prefs.clear();
    
    // Clear FM stations
    clearFMStations();
    
    // Delete stations file
    if (LittleFS.exists(stationsFile)) {
        LittleFS.remove(stationsFile);
    }
    
    Serial.println("Factory reset complete");
}