#ifndef STORAGE_MODULE_H
#define STORAGE_MODULE_H

#include <Preferences.h>
#include <LittleFS.h>
#include "CommonTypes.h"

#define MAX_STATIONS 20
#define MAX_INTERNET_STATIONS 10

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
    
    // Timezone settings using NVS
    bool saveTimezone(long gmtOffset, long dstOffset);
    bool loadTimezone(long &gmtOffset, long &dstOffset);
    
    // Utility
    void factoryReset();
};

#endif