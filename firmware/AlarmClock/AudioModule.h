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
    bool isPlayingMP3;
    bool shouldLoopMP3;
    String currentMP3File;

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
    
    // MP3 file playback
    bool playMP3File(const char* filename, bool loop = false);
    void stopMP3();
    bool isMP3Playing();
    String getCurrentMP3File();
    
    void setVolume(int volume);
    int getCurrentVolume();
    int getMaxVolume();
    
    String getCurrentStationName();
    int getCurrentStationIndex();
    int getStationCount();
    
    bool getIsPlaying();
};
#endif
