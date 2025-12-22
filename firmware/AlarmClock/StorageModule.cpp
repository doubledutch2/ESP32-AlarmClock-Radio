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

// ===== INTERNET RADIO STATION STORAGE =====
bool StorageModule::saveInternetStation(int index, const char* name, const char* url) {
    if (!isInitialized || index < 0 || index >= MAX_INTERNET_STATIONS) return false;
    
    char nameKey[16], urlKey[16];
    sprintf(nameKey, "inet_n_%d", index);
    sprintf(urlKey, "inet_u_%d", index);
    
    prefs.putString(nameKey, name);
    prefs.putString(urlKey, url);
    
    Serial.printf("Saved internet station %d: %s\n", index, name);
    return true;
}

bool StorageModule::loadInternetStation(int index, String &name, String &url) {
    if (!isInitialized || index < 0 || index >= MAX_INTERNET_STATIONS) return false;
    
    char nameKey[16], urlKey[16];
    sprintf(nameKey, "inet_n_%d", index);
    sprintf(urlKey, "inet_u_%d", index);
    
    name = prefs.getString(nameKey, "");
    url = prefs.getString(urlKey, "");
    
    return (name.length() > 0 && url.length() > 0);
}

bool StorageModule::deleteInternetStation(int index) {
    if (!isInitialized || index < 0 || index >= MAX_INTERNET_STATIONS) return false;
    
    char nameKey[16], urlKey[16];
    sprintf(nameKey, "inet_n_%d", index);
    sprintf(urlKey, "inet_u_%d", index);
    
    prefs.remove(nameKey);
    prefs.remove(urlKey);
    
    Serial.printf("Deleted internet station %d\n", index);
    return true;
}

int StorageModule::getInternetStationCount() {
    if (!isInitialized) return 0;
    return prefs.getInt("inet_count", 0);
}

void StorageModule::setInternetStationCount(int count) {
    if (!isInitialized) return;
    if (count >= 0 && count <= MAX_INTERNET_STATIONS) {
        prefs.putInt("inet_count", count);
    }
}

void StorageModule::clearInternetStations() {
    if (!isInitialized) return;
    
    for (int i = 0; i < MAX_INTERNET_STATIONS; i++) {
        deleteInternetStation(i);
    }
    setInternetStationCount(0);
    Serial.println("All internet stations cleared");
}

// ===== TIMEZONE SETTINGS =====
bool StorageModule::saveTimezone(long gmtOffset, long dstOffset) {
    if (!isInitialized) return false;
    
    prefs.putLong("gmtOffset", gmtOffset);
    prefs.putInt("dstOffset", dstOffset);
    
    Serial.printf("Timezone saved: GMT %ld, DST %d\n", gmtOffset, dstOffset);
    return true;
}

bool StorageModule::loadTimezone(long &gmtOffset, long &dstOffset) {
    if (!isInitialized) return false;
    
    gmtOffset = prefs.getLong("gmtOffset", 0);
    dstOffset = prefs.getInt("dstOffset", 0);
    
    Serial.printf("Timezone loaded: GMT %ld, DST %d\n", gmtOffset, dstOffset);
    return true;
}

void StorageModule::factoryReset() {
    Serial.println("Performing factory reset...");
    
    // Clear NVS preferences
    prefs.clear();
    
    // Clear FM stations
    clearFMStations();
    
    // Clear internet stations
    clearInternetStations();
    
    // Delete stations file
    if (LittleFS.exists(stationsFile)) {
        LittleFS.remove(stationsFile);
    }
    
    Serial.println("Factory reset complete");
}

bool StorageModule::saveAlarm(int index, const AlarmConfig& alarm) {
    if (!isInitialized || index < 0 || index >= MAX_ALARMS) return false;
    
    char prefix[16];
    sprintf(prefix, "alm_%d_", index);
    
    prefs.putBool((String(prefix) + "en").c_str(), alarm.enabled);
    prefs.putUChar((String(prefix) + "h").c_str(), alarm.hour);
    prefs.putUChar((String(prefix) + "m").c_str(), alarm.minute);
    prefs.putUChar((String(prefix) + "rep").c_str(), (uint8_t)alarm.repeatMode);
    prefs.putUChar((String(prefix) + "snd").c_str(), (uint8_t)alarm.soundType);
    prefs.putInt((String(prefix) + "idx").c_str(), alarm.stationIndex);
    prefs.putFloat((String(prefix) + "fm").c_str(), alarm.fmFrequency);
    prefs.putString((String(prefix) + "mp3").c_str(), alarm.mp3File);
    prefs.putUShort((String(prefix) + "yr").c_str(), alarm.lastYear);
    prefs.putUChar((String(prefix) + "mon").c_str(), alarm.lastMonth);
    prefs.putUChar((String(prefix) + "day").c_str(), alarm.lastDay);
    
    Serial.printf("Saved Alarm %d: %02d:%02d %s\n", 
                  index, alarm.hour, alarm.minute, alarm.enabled ? "ON" : "OFF");
    return true;
}

bool StorageModule::loadAlarm(int index, AlarmConfig& alarm) {
    if (!isInitialized || index < 0 || index >= MAX_ALARMS) return false;
    
    char prefix[16];
    sprintf(prefix, "alm_%d_", index);
    
    alarm.enabled = prefs.getBool((String(prefix) + "en").c_str(), false);
    alarm.hour = prefs.getUChar((String(prefix) + "h").c_str(), 7);
    alarm.minute = prefs.getUChar((String(prefix) + "m").c_str(), 0);
    alarm.repeatMode = (AlarmRepeat)prefs.getUChar((String(prefix) + "rep").c_str(), ALARM_DAILY);
    alarm.soundType = (AlarmSoundType)prefs.getUChar((String(prefix) + "snd").c_str(), SOUND_INTERNET_RADIO);
    alarm.stationIndex = prefs.getInt((String(prefix) + "idx").c_str(), 0);
    alarm.fmFrequency = prefs.getFloat((String(prefix) + "fm").c_str(), 98.0);
    alarm.mp3File = prefs.getString((String(prefix) + "mp3").c_str(), "");
    alarm.lastYear = prefs.getUShort((String(prefix) + "yr").c_str(), 0);
    alarm.lastMonth = prefs.getUChar((String(prefix) + "mon").c_str(), 0);
    alarm.lastDay = prefs.getUChar((String(prefix) + "day").c_str(), 0);
    
    return true;
} // Here

// Add these methods to your existing StorageModule.cpp file
// Place them after your existing methods, before the final closing brace

// ===== FEATURE FLAGS =====
bool StorageModule::saveFeatureFlags(const FeatureFlags& flags) {
    if (!isInitialized) return false;
    
    prefs.putBool("feat_touch", flags.enableTouchScreen);
    prefs.putBool("feat_btn", flags.enableButtons);
    prefs.putBool("feat_draw", flags.enableDraw);
    prefs.putBool("feat_audio", flags.enableAudio);
    prefs.putBool("feat_stereo", flags.enableStereo);
    prefs.putBool("feat_led", flags.enableLED);
    prefs.putBool("feat_alarms", flags.enableAlarms);
    prefs.putBool("feat_web", flags.enableWeb);
    prefs.putBool("feat_fm", flags.enableFMRadio);
    prefs.putBool("feat_pram", flags.enablePRAM);
    prefs.putBool("feat_i2c", flags.enableI2CScan);
    
    Serial.println("Feature flags saved to NVS");
    /*
    Serial.print("Save EnableLED:");
    if (flags.enableLED) {
        Serial.println("true");
    }
    else {
        Serial.println("false");
    }
    */
    return true;
}

bool StorageModule::loadFeatureFlags(FeatureFlags& flags) {
    if (!isInitialized) return false;
    
    // Load with defaults from constructor
    flags.enableTouchScreen = prefs.getBool("feat_touch", true);
    flags.enableButtons = prefs.getBool("feat_btn", true);
    flags.enableDraw = prefs.getBool("feat_draw", true);
    flags.enableAudio = prefs.getBool("feat_audio", true);
    flags.enableStereo = prefs.getBool("feat_stereo", true);
    flags.enableLED = prefs.getBool("feat_led", true);
    flags.enableAlarms = prefs.getBool("feat_alarms", true);
    flags.enableWeb = prefs.getBool("feat_web", true);
    flags.enableFMRadio = prefs.getBool("feat_fm", false);
    flags.enablePRAM = prefs.getBool("feat_pram", true);
    flags.enableI2CScan = prefs.getBool("feat_i2c", true);
    /*
    Serial.print("Load EnableLED:");
    if (flags.enableLED) {
        Serial.println("true");
    }
    else {
        Serial.println("false");
    }
    */
    Serial.println("Feature flags loaded from NVS");
    return true;
}

// ===== VOLUME AND BRIGHTNESS =====
bool StorageModule::saveVolume(uint8_t volume) {
    if (!isInitialized) return false;
    prefs.putUChar("volume", volume);
    Serial.printf("Volume saved: %d\n", volume);
    return true;
}

uint8_t StorageModule::loadVolume(uint8_t defaultValue) {
    if (!isInitialized) return defaultValue;
    uint8_t vol = prefs.getUChar("volume", defaultValue);
    Serial.printf("Volume loaded: %d\n", vol);
    return vol;
}

bool StorageModule::saveBrightness(uint8_t brightness) {
    if (!isInitialized) return false;
    prefs.putUChar("brightness", brightness);
    Serial.printf("Brightness saved: %d\n", brightness);
    return true;
}

uint8_t StorageModule::loadBrightness(uint8_t defaultValue) {
    if (!isInitialized) return defaultValue;
    uint8_t bright = prefs.getUChar("brightness", defaultValue);
    Serial.printf("Brightness loaded: %d\n", bright);
    return bright;
}

