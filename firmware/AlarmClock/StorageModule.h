#ifndef STORAGE_MODULE_H
#define STORAGE_MODULE_H

#include <Preferences.h>
#include <LittleFS.h>

#define MAX_STATIONS 20

struct RadioStation {
    float frequency;
    char name[32];
};

class StorageModule {
private:
    Preferences prefs;
    bool isInitialized;
    RadioStation stations[MAX_STATIONS];
    int stationCount;
    const char* stationsFile = "/stations.txt";

public:
    StorageModule();
    
    bool begin();
    bool isReady();
    
    // Config management using NVS (Preferences)
    bool saveConfig(uint8_t alarmHour, uint8_t alarmMin, bool alarmEnabled, float fmFreq);
    bool loadConfig(uint8_t &alarmHour, uint8_t &alarmMin, bool &alarmEnabled, float &fmFreq);
    
    // Station management using LittleFS
    bool saveStations();
    bool loadStations();
    bool addStation(float frequency, const char* name);
    bool removeStation(int index);
    RadioStation* getStation(int index);
    int getStationCount();
    void setStationCount(int count);
    void clearStations();
    
    // Utility
    void factoryReset();
};

#endif