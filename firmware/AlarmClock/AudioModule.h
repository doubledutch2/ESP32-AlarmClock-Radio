// ==================== AudioModule.h ====================
#ifndef AUDIO_MODULE_H
#define AUDIO_MODULE_H

// Include SD libraries BEFORE Audio.h to use ESP32 SD library
#include <SD.h>
#include <FS.h>
#include <SPI.h>
#include "Audio.h"

struct RadioStation {
  String name;
  String url;
};

class AudioModule {
private:
  Audio* audio;
  RadioStation* stationList;
  uint8_t stationCount;
  int currentIndex;
  int volume;
  int maxVolume;
  
public:
  AudioModule(uint8_t bclk, uint8_t lrc, uint8_t dout, int maxVol = 25);
  ~AudioModule();
  
  void begin();
  void loop();
  void setVolume(int vol);
  void playStation(int index);
  void playCustom(const char* name, const char* url);
  void nextStation();
  void previousStation();
  int getCurrentVolume() { return volume; }
  String getCurrentStationName();
  
  void setStationList(RadioStation* stations, uint8_t count);
  RadioStation* getStationList() { return stationList; }
  uint8_t getStationCount() { return stationCount; }
};

#endif

