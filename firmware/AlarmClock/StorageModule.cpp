#include "StorageModule.h"

StorageModule::StorageModule(int csPin) : sdCS(csPin), isInitialized(false), stationCount(0) {
    for (int i = 0; i < MAX_STATIONS; i++) {
        stations[i].frequency = 0.0;
        stations[i].name[0] = '\0';
    }
}

bool StorageModule::begin() {
    if (!SD.begin(sdCS)) {
        Serial.println("SD Card initialization failed!");
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }
    
    Serial.println("SD Card initialized successfully");
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

bool StorageModule::saveConfig(uint8_t alarmHour, uint8_t alarmMin, bool alarmEnabled, float fmFreq) {
    if (!isInitialized) return false;
    
    File file = SD.open(configFile, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return false;
    }
    
    file.printf("%d,%d,%d,%.1f\n", alarmHour, alarmMin, alarmEnabled ? 1 : 0, fmFreq);
    file.close();
    
    Serial.println("Config saved");
    return true;
}

bool StorageModule::loadConfig(uint8_t &alarmHour, uint8_t &alarmMin, bool &alarmEnabled, float &fmFreq) {
    if (!isInitialized) return false;
    
    File file = SD.open(configFile, FILE_READ);
    if (!file) {
        Serial.println("Config file not found, using defaults");
        alarmHour = 7;
        alarmMin = 0;
        alarmEnabled = false;
        fmFreq = 98.0;
        return false;
    }
    
    String line = file.readStringUntil('\n');
    file.close();
    
    int h, m, e;
    float f;
    if (sscanf(line.c_str(), "%d,%d,%d,%f", &h, &m, &e, &f) == 4) {
        alarmHour = h;
        alarmMin = m;
        alarmEnabled = (e == 1);
        fmFreq = f;
        Serial.println("Config loaded");
        return true;
    }
    
    Serial.println("Failed to parse config");
    return false;
}

bool StorageModule::saveStations() {
    if (!isInitialized) return false;
    
    File file = SD.open(stationsFile, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open stations file for writing");
        return false;
    }
    
    for (int i = 0; i < stationCount; i++) {
        file.printf("%.1f,%s\n", stations[i].frequency, stations[i].name);
    }
    file.close();
    
    Serial.println("Stations saved");
    return true;
}

bool StorageModule::loadStations() {
    if (!isInitialized) return false;
    
    File file = SD.open(stationsFile, FILE_READ);
    if (!file) {
        Serial.println("Stations file not found");
        return false;
    }
    
    stationCount = 0;
    while (file.available() && stationCount < MAX_STATIONS) {
        String line = file.readStringUntil('\n');
        int commaPos = line.indexOf(',');
        if (commaPos > 0) {
            stations[stationCount].frequency = line.substring(0, commaPos).toFloat();
            strncpy(stations[stationCount].name, line.substring(commaPos + 1).c_str(), 31);
            stations[stationCount].name[31] = '\0';
            stationCount++;
        }
    }
    file.close();
    
    Serial.printf("Loaded %d stations\n", stationCount);
    return true;
}

bool StorageModule::addStation(float frequency, const char* name) {
    if (stationCount >= MAX_STATIONS) return false;
    
    stations[stationCount].frequency = frequency;
    strncpy(stations[stationCount].name, name, 31);
    stations[stationCount].name[31] = '\0';
    stationCount++;
    
    return saveStations();
}

bool StorageModule::removeStation(int index) {
    if (index < 0 || index >= stationCount) return false;
    
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
}