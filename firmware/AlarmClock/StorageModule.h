#ifndef STORAGE_MODULE_H
#define STORAGE_MODULE_H

#include <Preferences.h>
#include <LittleFS.h>
#include "CommonTypes.h"
#include "AlarmData.h"
#include "FeatureFlags.h"

#define MAX_STATIONS 20
#define MAX_INTERNET_STATIONS 10

// Feature flags structure

class StorageModule {

private:
    Preferences prefs;
    bool isInitialized;
    FMRadioPreset fmPresets[MAX_STATIONS];
    int stationCount;
    const char* stationsFile = "/fmstations.txt";

public:
    StorageModule();
    
    bool begin();
    bool isReady();
    
    // Config management using NVS (Preferences)
    bool saveConfig(uint8_t alarmHour, uint8_t alarmMin, bool alarmEnabled, float fmFreq);
    bool loadConfig(uint8_t &alarmHour, uint8_t &alarmMin, bool &alarmEnabled, float &fmFreq);
    
    // Alarm management using NVS
    bool saveAlarm(int index, const AlarmConfig& alarm);
    bool loadAlarm(int index, AlarmConfig& alarm);
    
    // FM Station management using LittleFS
    bool saveFMStations();
    bool loadFMStations();
    bool addFMStation(float frequency, const char* name);
    bool removeFMStation(int index);
    FMRadioPreset* getFMStation(int index);
    int getFMStationCount();
    void setFMStationCount(int count);
    void clearFMStations();
    
    // Internet Radio Station management using NVS
    bool saveInternetStation(int index, const char* name, const char* url);
    bool loadInternetStation(int index, String &name, String &url);
    bool deleteInternetStation(int index);
    int getInternetStationCount();
    void setInternetStationCount(int count);
    void clearInternetStations();
    // Audio mode settings (FM Radio vs Internet Radio)
    bool saveAudioMode(bool useFMRadio);
    bool loadAudioMode(bool defaultValue = false);  // false = Internet Radio, true = FM Radio
    // Timezone settings using NVS
    bool saveTimezone(long gmtOffset, long dstOffset);
    bool loadTimezone(long &gmtOffset, long &dstOffset);
    
    // Feature flags management
    bool saveFeatureFlags(const FeatureFlags& flags);
    bool loadFeatureFlags(FeatureFlags& flags);
    
    // Volume and brightness settings
    bool saveVolume(uint8_t volume);
    uint8_t loadVolume(uint8_t defaultValue = 3);
    bool saveBrightness(uint8_t brightness);
    uint8_t loadBrightness(uint8_t defaultValue = 200);
    
    // Utility
    void factoryReset();

};

#endif