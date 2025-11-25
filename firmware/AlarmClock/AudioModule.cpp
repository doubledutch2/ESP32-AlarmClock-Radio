#include "AudioModule.h"

AudioModule::AudioModule(int bclkPin, int lrcPin, int doutPin, int maxVol)
    : stations(nullptr), stationCount(0), currentStation(-1), 
      currentVolume(10), maxVolume(maxVol), currentStationName("Unknown"), isPlaying(false) {
    
    audio.setPinout(bclkPin, lrcPin, doutPin);
}

void AudioModule::begin() {
    audio.setVolume(currentVolume);
    Serial.println("AudioModule initialized");
}

void AudioModule::loop() {
    audio.loop();
}

void AudioModule::setStationList(RadioStation* stationList, int count) {
    stations = stationList;
    stationCount = count;
    Serial.printf("Loaded %d stations\n", stationCount);
}

void AudioModule::playStation(int index) {
    if (!stations || index < 0 || index >= stationCount) {
        Serial.println("Invalid station index");
        return;
    }
    
    currentStation = index;
    currentStationName = stations[index].name;
    
    Serial.printf("Playing: %s (%s)\n", 
        stations[index].name.c_str(), 
        stations[index].url.c_str());
    
    audio.stopSong();
    delay(100);
    
    if (audio.connecttohost(stations[index].url.c_str())) {
        isPlaying = true;
        Serial.println("Stream connected successfully");
    } else {
        isPlaying = false;
        Serial.println("Failed to connect to stream");
    }
}

void AudioModule::nextStation() {
    if (!stations || stationCount == 0) return;
    
    currentStation = (currentStation + 1) % stationCount;
    playStation(currentStation);
}

void AudioModule::previousStation() {
    if (!stations || stationCount == 0) return;
    
    currentStation = (currentStation - 1 + stationCount) % stationCount;
    playStation(currentStation);
}

void AudioModule::playCustom(const char* name, const char* url) {
    currentStationName = String(name);
    currentStation = -1; // Custom station
    
    Serial.printf("Playing custom: %s (%s)\n", name, url);
    
    audio.stopSong();
    delay(100);
    
    if (audio.connecttohost(url)) {
        isPlaying = true;
        Serial.println("Custom stream connected successfully");
    } else {
        isPlaying = false;
        Serial.println("Failed to connect to custom stream");
    }
}

void AudioModule::stop() {
    audio.stopSong();
    isPlaying = false;
    currentStationName = "Stopped";
    Serial.println("Audio stopped");
}

void AudioModule::setVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > maxVolume) volume = maxVolume;
    
    currentVolume = volume;
    audio.setVolume(volume);
}

int AudioModule::getCurrentVolume() {
    return currentVolume;
}

int AudioModule::getMaxVolume() {
    return maxVolume;
}

String AudioModule::getCurrentStationName() {
    return currentStationName;
}

int AudioModule::getCurrentStationIndex() {
    return currentStation;
}

int AudioModule::getStationCount() {
    return stationCount;
}

bool AudioModule::getIsPlaying() {
    return isPlaying;
}