#ifndef STORAGE_MODULE_H
#define STORAGE_MODULE_H

#include <SD.h>
#include <FS.h>

#define MAX_STATIONS 20

struct RadioStation {
    float frequency;
    char name[32];
};

class StorageModule {
private:
    int sdCS;
    bool isInitialized;
    RadioStation stations[MAX_STATIONS];
    int stationCount;
    const char* configFile = "/config.txt";
    const char* stationsFile = "/stations.txt";

public:
    StorageModule(int csPin);
    
    bool begin();
    bool isReady();
    
    // Config management
    bool saveConfig(uint8_t alarmHour, uint8_t alarmMin, bool alarmEnabled, float fmFreq);
    bool loadConfig(uint8_t &alarmHour, uint8_t &alarmMin, bool &alarmEnabled, float &fmFreq);
    
    // Station management
    bool saveStations();
    bool loadStations();
    bool addStation(float frequency, const char* name);
    bool removeStation(int index);
    RadioStation* getStation(int index);
    int getStationCount();
    void setStationCount(int count);
    void clearStations();
};

#endif