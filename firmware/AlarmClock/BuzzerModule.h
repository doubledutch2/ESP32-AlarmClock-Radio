#ifndef BUZZER_MODULE_H
#define BUZZER_MODULE_H

#include <Arduino.h>

class BuzzerModule {
private:
    uint8_t buzzerPin;
    bool isPlaying;

public:
    BuzzerModule(uint8_t pin);
    
    void begin();
    void playTone(uint16_t frequency, uint32_t duration = 0);
    void stopTone();
    void playAlarm();
    void playBeep();
    bool isBuzzing();
};

#endif

