#ifndef AUDIO_MODULE_H
#define AUDIO_MODULE_H

#include <Arduino.h>
#include <Audio.h>
#include <string>

struct RadioStation {
    String name;
    String url;
};

class AudioModule {
private:
    Audio audio;
    RadioStation* stations;
    int stationCount;
    int currentStation;
    int currentVolume;
    int maxVolume;
    String currentStationName;
    bool isPlaying;

public:
    AudioModule(int bclkPin, int lrcPin, int doutPin, int maxVol = 21);
    
    void begin();
    void loop();
    
    void setStationList(RadioStation* stationList, int count);
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