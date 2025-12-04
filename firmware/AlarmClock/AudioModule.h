#ifndef AUDIO_MODULE_H
#define AUDIO_MODULE_H

#include <Arduino.h>
#include <Audio.h>
#include "CommonTypes.h"

class AudioModule {
private:
    Audio audio;
    InternetRadioStation* stations;
    int stationCount;
    int currentStation;
    int currentVolume;
    int maxVolume;
    String currentStationName;
    bool isPlaying;

public:
    AudioModule(int bclkPin, int lrcPin, int doutPin, int maxVol = 21, int defaultVolume=0);
    
    void begin();
    void loop();
    
    void setStationList(InternetRadioStation* stationList, int count);
    void playStation(int index);
    void nextStation();
    void previousStation();
    void playCustom(const char* name, const char* url);
    void stop();
    
    void setVolume(int volume);
    int getCurrentVolume();
    int getMaxVolume();
    
    String getCurrentStationName();
    int getCurrentStationIndex();
    int getStationCount();
    
    bool getIsPlaying();
};

#endif