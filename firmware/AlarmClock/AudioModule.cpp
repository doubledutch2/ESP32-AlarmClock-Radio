#include "AudioModule.h"

AudioModule::AudioModule(int bclkPin, int lrcPin, int doutPin, int maxVol, int lastVolume)
    : stations(nullptr), stationCount(0), currentStation(-1), 
      currentVolume(lastVolume), maxVolume(maxVol), currentStationName("Unknown"), 
      isPlaying(false), isPlayingMP3(false), shouldLoopMP3(false), currentMP3File("") {
    
    audio.setPinout(bclkPin, lrcPin, doutPin);
}

void AudioModule::begin() {
    audio.setVolume(currentVolume);
    Serial.println("AudioModule initialized");
}

void AudioModule::loop() {
    audio.loop();
    
    // Check if MP3 has finished and should loop
    if (isPlayingMP3 && shouldLoopMP3) {
        // The Audio library will automatically loop if we don't stop it
        // This is just a placeholder for any additional logic needed
    }
}

void AudioModule::setStationList(InternetRadioStation* stationList, int count) {
    stations = stationList;
    stationCount = count;
    Serial.printf("AudioModule: Loaded %d stations\n", stationCount);
}

void AudioModule::playStation(int index) {
    if (!stations || index < 0 || index >= stationCount) {
        Serial.println("AudioModule: Invalid station index");
        return;
    }
    
    currentStation = index;
    currentStationName = stations[index].name;
    isPlayingMP3 = false;
    shouldLoopMP3 = false;
    currentMP3File = "";
    
    Serial.printf("AudioModule: Playing: %s (%s)\n", 
        stations[index].name.c_str(), 
        stations[index].url.c_str());
    
    audio.stopSong();
    delay(100);
    
    if (audio.connecttohost(stations[index].url.c_str())) {
        isPlaying = true;
        Serial.println("AudioModule: Stream connected successfully");
    } else {
        isPlaying = false;
        Serial.println("AudioModule: Failed to connect to stream");
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
    isPlayingMP3 = false;
    shouldLoopMP3 = false;
    currentMP3File = "";
    
    Serial.printf("AudioModule: Playing custom: %s (%s)\n", name, url);
    
    audio.stopSong();
    delay(100);
    
    if (audio.connecttohost(url)) {
        isPlaying = true;
        Serial.println("AudioModule: Custom stream connected successfully");
    } else {
        isPlaying = false;
        Serial.println("AudioModule: Failed to connect to custom stream");
    }
}

bool AudioModule::playMP3File(const char* filename, bool loop) {
    if (!filename || strlen(filename) == 0) {
        Serial.println("AudioModule: Invalid MP3 filename");
        return false;
    }
    
    // Stop any current playback
    audio.stopSong();
    delay(100);
    
    // Construct full path (assuming files are in /mp3/ directory on SD or LittleFS)
    String fullPath = String("/mp3/") + filename;
    
    Serial.printf("AudioModule: Playing MP3: %s (loop: %s)\n", 
                  fullPath.c_str(), loop ? "yes" : "no");
    
    currentMP3File = String(filename);
    shouldLoopMP3 = loop;
    currentStationName = "MP3: " + String(filename);
    currentStation = -1;
    
    // Try to connect to the MP3 file
    if (audio.connecttoFS(fullPath.c_str())) {
        isPlaying = true;
        isPlayingMP3 = true;
        Serial.println("AudioModule: MP3 file started successfully");
        return true;
    } else {
        isPlaying = false;
        isPlayingMP3 = false;
        shouldLoopMP3 = false;
        currentMP3File = "";
        Serial.println("AudioModule: Failed to play MP3 file");
        return false;
    }
}

void AudioModule::stopMP3() {
    if (isPlayingMP3) {
        audio.stopSong();
        isPlayingMP3 = false;
        shouldLoopMP3 = false;
        currentMP3File = "";
        isPlaying = false;
        Serial.println("AudioModule: MP3 playback stopped");
    }
}

bool AudioModule::isMP3Playing() {
    return isPlayingMP3;
}

String AudioModule::getCurrentMP3File() {
    return currentMP3File;
}

void AudioModule::stop() {
    audio.stopSong();
    isPlaying = false;
    isPlayingMP3 = false;
    shouldLoopMP3 = false;
    currentMP3File = "";
    currentStationName = "Stopped";
    Serial.println("AudioModule: Audio stopped");
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