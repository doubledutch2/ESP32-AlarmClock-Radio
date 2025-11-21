// ==================== AudioModule.cpp ====================
#include "AudioModule.h"

AudioModule::AudioModule(uint8_t bclk, uint8_t lrc, uint8_t dout, int maxVol) {
  audio = new Audio();
  audio->setPinout(bclk, lrc, dout);
  maxVolume = maxVol;
  volume = maxVol / 2;
  currentIndex = 0;
  stationList = nullptr;
  stationCount = 0;
}

AudioModule::~AudioModule() {
  delete audio;
}

void AudioModule::begin() {
  audio->setVolume(volume);
  Serial.println("Audio module initialized");
}

void AudioModule::loop() {
  audio->loop();
}

void AudioModule::setVolume(int vol) {
  if (vol < 0) vol = 0;
  if (vol > maxVolume) vol = maxVolume;
  volume = vol;
  audio->setVolume(volume);
}

void AudioModule::playStation(int index) {
  if (!stationList || index < 0 || index >= stationCount) {
    Serial.println("Invalid station index");
    return;
  }
  
  currentIndex = index;
  RadioStation& station = stationList[index];
  
  Serial.println("\n=== Playing Station ===");
  Serial.printf("Index: %d\n", index);
  Serial.printf("Name: %s\n", station.name.c_str());
  Serial.printf("URL: %s\n", station.url.c_str());
  
  audio->stopSong();
  delay(100);
  
  if (audio->connecttohost(station.url.c_str())) {
    Serial.println("Stream connected!");
  } else {
    Serial.println("Stream failed!");
  }
}

void AudioModule::playCustom(const char* name, const char* url) {
  Serial.println("\n=== Playing Custom Stream ===");
  Serial.printf("Name: %s\n", name);
  Serial.printf("URL: %s\n", url);
  
  audio->stopSong();
  delay(100);
  
  if (audio->connecttohost(url)) {
    Serial.println("Custom stream connected!");
  } else {
    Serial.println("Custom stream failed!");
  }
}

void AudioModule::nextStation() {
  if (!stationList || stationCount == 0) return;
  
  currentIndex = (currentIndex + 1) % stationCount;
  playStation(currentIndex);
}

void AudioModule::previousStation() {
  if (!stationList || stationCount == 0) return;
  
  currentIndex--;
  if (currentIndex < 0) currentIndex = stationCount - 1;
  playStation(currentIndex);
}

String AudioModule::getCurrentStationName() {
  if (stationList && currentIndex >= 0 && currentIndex < stationCount) {
    return stationList[currentIndex].name;
  }
  return "Unknown";
}

void AudioModule::setStationList(RadioStation* stations, uint8_t count) {
  stationList = stations;
  stationCount = count;
}

