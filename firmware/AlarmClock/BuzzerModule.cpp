#include "BuzzerModule.h"

BuzzerModule::BuzzerModule(uint8_t pin) : buzzerPin(pin), isPlaying(false) {}

void BuzzerModule::begin() {
    // ESP32 Core 3.x uses ledcAttach instead of ledcSetup + ledcAttachPin
    ledcAttach(buzzerPin, 2000, 8); // pin, frequency, resolution
}

void BuzzerModule::playTone(uint16_t frequency, uint32_t duration) {
    if (frequency > 0) {
        ledcWriteTone(buzzerPin, frequency);
        isPlaying = true;
        
        if (duration > 0) {
            delay(duration);
            stopTone();
        }
    }
}

void BuzzerModule::stopTone() {
    ledcWriteTone(buzzerPin, 0);
    isPlaying = false;
}

void BuzzerModule::playAlarm() {
    // Simple alarm pattern
    for (int i = 0; i < 3; i++) {
        playTone(1000, 200);
        delay(100);
        playTone(1500, 200);
        delay(100);
    }
    stopTone();
}

void BuzzerModule::playBeep() {
    playTone(2000, 100);
}

bool BuzzerModule::isBuzzing() {
    return isPlaying;
}
